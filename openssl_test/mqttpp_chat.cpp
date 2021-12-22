// async_subscribe.cpp
//
// This is a Paho MQTT C++ client, sample application.
//
// This application is an MQTT subscriber using the C++ asynchronous client
// interface, employing callbacks to receive messages and status updates.
//
// The sample demonstrates:
//  - Connecting to an MQTT server/broker.
//  - Subscribing to a topic
//  - Receiving messages through the callback API
//  - Receiving network disconnect updates and attempting manual reconnects.
//  - Using a "clean session" and manually re-subscribing to topics on
//    reconnect.
//

/*******************************************************************************
 * Copyright (c) 2013-2020 Frank Pagliughi <fpagliughi@mindspring.com>
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Frank Pagliughi - initial implementation and documentation
 *******************************************************************************/

#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <cctype>
#include <thread>
#include <chrono>
#include <sys/time.h>
#include "mqtt/async_client.h"
#include "openssl_tool.h"
#include "cJSON.h"

const std::string CLIENT_ID("wanghaha");
const std::string sub_topic = "downlink/+/" + CLIENT_ID;
const std::string pub_online_topic = "uplink/online/" + CLIENT_ID;

const int	QOS = 1;
const int	N_RETRY_ATTEMPTS = 5;

std::string PUBLIC_KEY = "-----BEGIN RSA PUBLIC KEY-----\n\
MIGHAoGBAKk41UCmzqEfWMExxImp6NCv8UupKYItHqiJH1bso4IEYPw7n65+LvZd\n\
9XCh03YspufXUa0xVrNoDLbujLeGr4Y2crIlSV8Mr7aY82oqyI8zCZg8iL3Qypvp\n\
mnbO5BjlQ4lRiHG+Hb2JUr+B1Wt4/FGvs6fKmHVgAYMX7gub+VsPAgED\n\
-----END RSA PUBLIC KEY-----"\n\;

std::string  get_timestamp_ms()
{
    std::chrono::milliseconds ms = std::chrono::duration_cast< std::chrono::milliseconds >(
        std::chrono::system_clock::now().time_since_epoch()
    );
 
    std::cout << ms.count() << std::endl;
    return std::to_string(ms.count());
}

/////////////////////////////////////////////////////////////////////////////

// Callbacks for the success or failures of requested actions.
// This could be used to initiate further action, but here we just log the
// results to the console.

class action_listener : public virtual mqtt::iaction_listener
{
	std::string name_;

	void on_failure(const mqtt::token& tok) override {
		std::cout << name_ << " failure";
		if (tok.get_message_id() != 0)
			std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
		std::cout << std::endl;
	}

	void on_success(const mqtt::token& tok) override {
		std::cout << name_ << " success";
		if (tok.get_message_id() != 0)
			std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
		auto top = tok.get_topics();
		if (top && !top->empty())
			std::cout << "\ttoken topic: '" << (*top)[0] << "', ..." << std::endl;
		std::cout << std::endl;
	}

public:
	action_listener(const std::string& name) : name_(name) {}
};


/////////////////////////////////////////////////////////////////////////////

/**
 * Local callback & listener class for use with the client connection.
 * This is primarily intended to receive messages, but it will also monitor
 * the connection to the broker. If the connection is lost, it will attempt
 * to restore the connection and re-subscribe to the topic.
 */
class callback : public virtual mqtt::callback,
					public virtual mqtt::iaction_listener

{
	// Counter for the number of connection retries
	int nretry_;
	// The MQTT client
	mqtt::async_client& cli_;
	// Options to use if we need to reconnect
	mqtt::connect_options& connOpts_;
	// An action listener to display the result of actions.
	action_listener subListener_;

	// This deomonstrates manually reconnecting to the broker by calling
	// connect() again. This is a possibility for an application that keeps
	// a copy of it's original connect_options, or if the app wants to
	// reconnect with different options.
	// Another way this can be done manually, if using the same options, is
	// to just call the async_client::reconnect() method.
	void reconnect() {
		std::this_thread::sleep_for(std::chrono::milliseconds(2500));
		try {
			cli_.connect(connOpts_, nullptr, *this);
		}
		catch (const mqtt::exception& exc) {
			std::cerr << "Error: " << exc.what() << std::endl;
			exit(1);
		}
	}

	// Re-connection failure
	void on_failure(const mqtt::token& tok) override {
		std::cout << "Connection attempt failed" << std::endl;
		if (++nretry_ > N_RETRY_ATTEMPTS)
			exit(1);
		reconnect();
	}

	// (Re)connection success
	// Either this or connected() can be used for callbacks.
	void on_success(const mqtt::token& tok) override {}

	// (Re)connection success
	void connected(const std::string& cause) override {
		std::cout << "\nConnection success" << std::endl;
		std::cout << "\nSubscribing to topic '" << sub_topic << "'\n"
			<< "\tfor client " << CLIENT_ID
			<< " using QoS" << QOS << "\n"
			<< "\nPress Q<Enter> to quit\n" << std::endl;

		cli_.subscribe(sub_topic, QOS, nullptr, subListener_);
		mqtt_send_online_msg();
	}

	// Callback for when the connection is lost.
	// This will initiate the attempt to manually reconnect.
	void connection_lost(const std::string& cause) override {
		std::cout << "\nConnection lost" << std::endl;
		if (!cause.empty())
			std::cout << "\tcause: " << cause << std::endl;

		std::cout << "Reconnecting..." << std::endl;
		nretry_ = 0;
		reconnect();
	}

	// Callback for when a message arrives.
	void message_arrived(mqtt::const_message_ptr msg) override {
		std::cout << "Message arrived" << std::endl;
		std::cout << "\ttopic: '" << msg->get_topic() << "'" << std::endl;
		std::cout << "\tpayload: '" << msg->to_string() << "'\n" << std::endl;

		std::string recv_msg = rsa_pub_split128_decrypt(msg->to_string(),PUBLIC_KEY);
		std::cout << "\tmsg: " << recv_msg << std::endl;
		}
	}

	void delivery_complete(mqtt::delivery_token_ptr token) override {}

private:
	int mqtt_send_online_msg()
	{
		cJSON *root = cJSON_CreateObject();

		cJSON_AddNumberToObject(root,"id",1);
		cJSON_AddStringToObject(root, "ts", get_timestamp_ms().c_str());
		char *out = cJSON_Print(root);
		std::string strMsg = std::string(out);
		cJSON_Delete(root);

		if (out)
		{
			free(out);
		}

		strMsg = rsa_pub_encrypt(strMsg , PUBLIC_KEY);
		cli_.publish(pub_online_topic, strMsg.c_str(),strlen(strMsg.c_str()));
		return 0;
	}

public:
	callback(mqtt::async_client& cli, mqtt::connect_options& connOpts)
				: nretry_(0), cli_(cli), connOpts_(connOpts), subListener_("Subscription") {}
};

/////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
	// A subscriber often wants the server to remember its messages when its
	// disconnected. In that case, it needs a unique ClientID and a
	// non-clean session.
	std::string SERVER_ADDRESS;
	if (argc == 2) 
	{
		SERVER_ADDRESS = argv[1];
	}
	else
	{
		std::cout << "USAGE: test_mqtt <ip:addr>" << std::endl;
		exit(1);
	}

	mqtt::async_client cli(SERVER_ADDRESS, CLIENT_ID);

	mqtt::connect_options connOpts;
	connOpts.set_clean_session(false);
	connOpts.set_keep_alive_interval(100);

	std::string username = get_timestamp_ms();
	std::string password = rsa_pub_encrypt(CLIENT_ID+username,PUBLIC_KEY);
	connOpts.set_user_name(username);
	connOpts.set_password(password);
	// Install the callback(s) before connecting.
	callback cb(cli, connOpts);
	cli.set_callback(cb);

	// Start the connection.
	// When completed, the callback will subscribe to topic.

	try {
		std::cout << "Connecting to the MQTT server..." << std::flush;
		cli.connect(connOpts, nullptr, cb);
	}
	catch (const mqtt::exception& exc) {
		std::cerr << "\nERROR: Unable to connect to MQTT server: '"
			<< SERVER_ADDRESS << "'" << exc << std::endl;
		return 1;
	}

	// Just block till user tells us to quit.

	while (std::tolower(std::cin.get()) != 'q')
		;

	// // Disconnect
	// try {
	// 	std::cout << "\nDisconnecting from the MQTT server..." << std::flush;
	// 	cli.disconnect()->wait();
	// 	std::cout << "OK" << std::endl;
	// }
	// catch (const mqtt::exception& exc) {
	// 	std::cerr << exc << std::endl;
	// 	return 1;
	// }

 	return 0;
}

