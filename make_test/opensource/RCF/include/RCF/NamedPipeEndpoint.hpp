
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

#ifndef INCLUDE_RCF_NAMEDPIPEENDPOINT_HPP
#define INCLUDE_RCF_NAMEDPIPEENDPOINT_HPP

#include <RCF/Endpoint.hpp>
#include <RCF/Export.hpp>
#include <RCF/Tchar.hpp>

namespace RCF {

    class RCF_EXPORT NamedPipeEndpoint : public Endpoint
    {
    public:

        NamedPipeEndpoint();

        NamedPipeEndpoint(const tstring & pipeName);

        ServerTransportUniquePtr createServerTransport() const;
        ClientTransportUniquePtr createClientTransport() const;
        EndpointPtr clone() const;

        std::string asString() const;

    private:
        tstring mPipeName;
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_NAMEDPIPEENDPOINT_HPP
