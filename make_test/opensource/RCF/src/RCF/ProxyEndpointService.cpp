
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

#include <RCF/ProxyEndpointService.hpp>

#include <RCF/Enums.hpp>
#include <RCF/Uuid.hpp>
#include <RCF/ProxyEndpointTransport.hpp>

namespace RCF
{

    ProxyEndpointEntry::ProxyEndpointEntry()
    {
    }

    ProxyEndpointEntry::ProxyEndpointEntry(const std::string& endpointName) : mName(endpointName)
    {
    }

    void ProxyEndpointService::onServiceAdded(RCF::RcfServer &server)
    {
        mpRcfServer = &server;
        server.bind<I_ProxyEp>(*this);
    }

    void ProxyEndpointService::onServiceRemoved(RCF::RcfServer &server)
    {
        RCF_UNUSED_VARIABLE(server);

    }

    void ProxyEndpointService::onServerStart(RCF::RcfServer &server)
    {
        RCF_UNUSED_VARIABLE(server);

    }

    void ProxyEndpointService::onServerStop(RCF::RcfServer &server)
    {
        RCF_UNUSED_VARIABLE(server);

        {
            Lock lock(mEntriesMutex);
            mEntries.clear();
        }

        {
            Lock lock(mEndpointConnectionsMutex);
            mEndpointConnections.clear();
        }
    }

    void ProxyEndpointService::enumerateProxyEndpoints(std::vector<std::string>& endpoints)
    {
        if ( !mpRcfServer->getEnableProxyEndpoints() )
        {
            RCF_THROW(Exception(RcfError_ProxyEndpointsNotEnabled));
        }

        endpoints.clear();
        std::vector<std::string> duds;
        
        Lock lock(mEntriesMutex);
        
        for ( auto iter : mEntries )
        {
            const std::string& endpointName = iter.first;
            ProxyEndpointEntry & entry = iter.second;
            RcfSessionPtr sessionPtr = entry.mSessionWeakPtr.lock();
            if ( sessionPtr && sessionPtr->isConnected() )
            {
                endpoints.push_back(endpointName);
            }
            else
            {
                duds.push_back(endpointName);
            }
        }

        for (auto dud : duds)
        {
            mEntries.erase(mEntries.find(dud));
        }
    }

    ClientTransportUniquePtr ProxyEndpointService::makeProxyEndpointConnection(
        const std::string& proxyEndpointName)
    {

        if ( !mpRcfServer->getEnableProxyEndpoints() )
        {
            RCF_THROW(Exception(RcfError_ProxyEndpointsNotEnabled));
        }

        std::string requestId = generateUuid();

        bool endpointOnline = true;
        {
            Lock lock(mEntriesMutex);
            auto iter = mEntries.find(proxyEndpointName);
            if (iter != mEntries.end())
            {
                ProxyEndpointEntry & entry = iter->second;

                RcfSessionPtr sessionPtr = entry.mSessionWeakPtr.lock();
                if ( !sessionPtr || !sessionPtr->isConnected() )
                {
                    mEntries.erase(iter);
                    endpointOnline = false;
                }
                else
                {
                    entry.mPendingRequests.push_back(requestId);
                    if ( entry.mAmdPtr  )
                    {
                        std::vector<std::string> & amdRequests = entry.mAmdPtr->parameters().a1.get();
                        amdRequests.clear();
                        amdRequests.swap(entry.mPendingRequests);
                        entry.mAmdPtr->commit();
                        entry.mAmdPtr.reset();
                    }
                }
            }
            else
            {
                endpointOnline = false;
            }
        }

        if ( !endpointOnline )
        {
            Exception e(RcfError_ProxyEndpointDown, proxyEndpointName);
            RCF_THROW(e);
        }

        // Wait for a connection to show up.
        auto requestKey = std::make_pair(proxyEndpointName, requestId);
        Timer connectTimer;
        
        while ( !connectTimer.elapsed(10*1000) )
        {
            Lock lock(mEndpointConnectionsMutex);
            auto iter = mEndpointConnections.find(requestKey);
            if ( iter != mEndpointConnections.end() )
            {
                ClientTransportUniquePtr transportPtr = std::move(iter->second);
                mEndpointConnections.erase(iter);
                return transportPtr;
            }

            using namespace std::chrono_literals;
            mEndpointConnectionsCond.wait_for(lock, 1000ms);
        }
        
        // TODO: chuck. Connection timeout.
        // ...
        RCF_ASSERT(0);
        return ClientTransportUniquePtr();
    }

    class ProxyEndpointSession
    {
    public:
        ProxyEndpointSession()
        {
        }

        ~ProxyEndpointSession()
        {
        }

        ProxyEndpointService *      mpEpService = NULL;
        std::string                 mEndpointName;
    };

    void ProxyEndpointService::removeEndpoint(const std::string& endpointName)
    {
        Lock lock(mEntriesMutex);
        auto iter = mEntries.find(endpointName);
        if ( iter != mEntries.end() )
        {
            mEntries.erase(iter);
        }
    }

    void ProxyEndpointService::SetupProxyEndpoint(const std::string& endpointName, const std::string& password)
    {
        if ( !mpRcfServer->getEnableProxyEndpoints() )
        {
            RCF_THROW(Exception(RcfError_ProxyEndpointsNotEnabled));
        }

        // TODO: check password.
        RCF_UNUSED_VARIABLE(password);

        ProxyEndpointSession * pEpSession = &getCurrentRcfSession().createSessionObject<ProxyEndpointSession>();
        pEpSession->mEndpointName = endpointName;
        pEpSession->mpEpService = this;

        Lock lock(mEntriesMutex);
        mEntries[endpointName] = ProxyEndpointEntry(endpointName);
        mEntries[endpointName].mSessionWeakPtr = getCurrentRcfSession().shared_from_this();
    }

    void ProxyEndpointService::GetConnectionRequests(
        std::vector<std::string>&   requests)
    {
        ProxyEndpointSession * pEpSession = 
            getCurrentRcfSession().querySessionObject<ProxyEndpointSession>();

        if ( !pEpSession )
        {
            // TODO: literal
            Exception e("Invalid proxy endpoint connection.");
            RCF_THROW(e);
        }

        std::string endpointName = pEpSession->mEndpointName;

        requests.clear();

        Lock lock(mEntriesMutex);
        auto iter = mEntries.find(endpointName);
        if (iter != mEntries.end())
        {
            ProxyEndpointEntry & entry = iter->second;
            if ( entry.mSessionWeakPtr.lock() == getCurrentRcfSession().shared_from_this() )
            {
                if ( entry.mPendingRequests.size() > 0 )
                {
                    // Synchronous return.
                    requests.clear();
                    requests.swap(entry.mPendingRequests);
                }
                else
                {
                    // Asynchronous return, once a request is made.
                    entry.mAmdPtr.reset(new AmdGetRequests(getCurrentRcfSession()));
                    entry.mSessionWeakPtr = getCurrentRcfSession().shared_from_this();
                }
            }
        }
    }

    void ProxyEndpointService::onConnectionAvailable(
        const std::string& endpointName,
        const std::string& requestId,
        RCF::RcfSessionPtr sessionPtr,
        RCF::ClientTransportUniquePtr transportPtr)
    {
        Lock lock(mEntriesMutex);
        mEndpointConnections[std::make_pair(endpointName, requestId)].reset( transportPtr.release() );
        mEndpointConnectionsCond.notify_all();
    }

    void ProxyEndpointService::MakeConnectionAvailable(
        const std::string&          endpointName, 
        const std::string&          requestId)
    {
        RCF::convertRcfSessionToRcfClient(
            [=](RcfSessionPtr sessionPtr, ClientTransportUniquePtr transportPtr)
            {
                onConnectionAvailable(endpointName, requestId, sessionPtr, std::move(transportPtr));
            },
            RCF::Twoway);
    }

} // namespace RCF
