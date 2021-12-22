
APPNAME = test_mqtt
cflags += -std=c++11 -I/usr/local/include/cjson 
ldflags += -L/usr/local/lib -lstdc++  -lssl -lcrypto -lpaho-mqtt3as -lpaho-mqttpp3 -lcjson

$(APPNAME):mqttpp_chat.cpp base64.cpp openssl_tool.cpp
	gcc $^ -o $(APPNAME) $(cflags) $(ldflags)

open_ssl_demo:base64.cpp openssl_tool.cpp
	gcc $^ -o $@ -DOPENSSL_DEMO_TEST -lstdc++  -lssl -lcrypto

clean:
	rm -f $(APPNAME) open_ssl_demo