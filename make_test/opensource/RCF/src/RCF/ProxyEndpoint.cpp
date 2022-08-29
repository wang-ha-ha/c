
//******************************************************************************
// RCF - Remote Call Framework
//
// Copyright (c) 2005 - 2018, Delta V Software. All rights reserved.
// http://www.deltavsoft.com
//
// RCF is distributed under dual licenses - closed source or GPL.
// Consult your particular license for conditions of use.
//
// If you have not purchased a commercial license, you are using RCF 
// under GPL terms.
//
// Version: 3.0
// Contact: support <at> deltavsoft.com 
//
//******************************************************************************

#include <RCF/ProxyEndpoint.hpp>

#include <RCF/ServerTransport.hpp>
#include <RCF/ProxyEndpointTransport.hpp>

namespace RCF
{
    ProxyEndpoint::ProxyEndpoint(const Endpoint& ep, const std::string& proxyEndpointName) :
        mpEndpoint(&ep),
        mpServer(),
        mProxyEndpointName(proxyEndpointName)
    {
    }

    ProxyEndpoint::ProxyEndpoint(RcfServer& server, const std::string& proxyEndpointName) :
        mpEndpoint(),
        mpServer(&server),
        mProxyEndpointName(proxyEndpointName)
    {
    }

    std::unique_ptr<ServerTransport>  ProxyEndpoint::createServerTransport() const
    {
        return std::unique_ptr<ServerTransport>(
            new ProxyEndpointTransport(*mpEndpoint, mProxyEndpointName));
    }

    std::unique_ptr<ClientTransport>  ProxyEndpoint::createClientTransport() const
    {
        ClientTransportUniquePtr transportPtr = mpServer->makeProxyEndpointConnection(mProxyEndpointName);
        return transportPtr;
    }

    EndpointPtr ProxyEndpoint::clone() const
    {
        if ( mpServer )
        {
            return EndpointPtr( new ProxyEndpoint(*mpServer, mProxyEndpointName));
        }
        else if ( mpEndpoint )
        {
            return EndpointPtr(new ProxyEndpoint(*mpEndpoint, mProxyEndpointName));
        }
        else
        {
            return EndpointPtr();
        }
    }

    std::string ProxyEndpoint::asString() const
    {
        // NB: this is used in ClientStub::disconnect() to identify proxy endpoints.
        return "ProxyEndpoint: " + mProxyEndpointName;
    }

}
