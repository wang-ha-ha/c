/*
    Copyright (c) 2006-2018, Alexis Royer, http://alexis.royer.free.fr/CLI

    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

        * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation
          and/or other materials provided with the distribution.
        * Neither the name of the CLI library project nor the names of its contributors may be used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
    CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
    LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
    NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "cli/pch.h"

#ifndef CLI_WIN_NETWORK
    #include <errno.h> // errno
    #define socket_errno errno
    #include <sys/types.h> // socket, bind, listen
    #include <sys/socket.h> // socket, bind, listen
    #include <netinet/in.h> // sockaddr_in
    #include <unistd.h> // getpid, close
#else
    #include <winsock2.h> // Windows sockets
    #define socket_errno WSAGetLastError()
    #define socklen_t int
    #define SHUT_RD SD_RECEIVE
    #define SHUT_WR SD_SEND
    #define SHUT_RDWR SD_BOTH
    #define close closesocket
    #define sleep(seconds) Sleep((seconds)*1000)
    // Disable conflicting macros.
    #undef DELETE
#endif
#include <string.h>
#include <time.h> // time

#include "cli/telnet.h"
#include "cli/exec_context.h"
#include "cli/traces.h"

#include "encoding.h"

CLI_NS_USE(cli)


//! @brief Telnet input buffer size.
#ifndef CLI_TELNET_INPUT_BUFFER_SIZE
    #define CLI_TELNET_INPUT_BUFFER_SIZE 1024
#endif

//! @brief Telnet server trace class singleton redirection.
#define CLI_TELNET_SERVER GetTelnetServerTraceClass()
static const TraceClass& GetTelnetServerTraceClass(void)
{
    static const TraceClass cli_TelnetServerTraceClass("CLI_TELNET_SERVER", Help()
        .AddHelp(Help::LANG_EN, "CLI telnet server traces")
        .AddHelp(Help::LANG_FR, "Traces de serveur telnet CLI"));
    return cli_TelnetServerTraceClass;
}
//! @brief Telnet connection trace class singleton redirection.
#define CLI_TELNET_IN GetTelnetInTraceClass()
//! @brief Telnet connection trace class singleton.
static const TraceClass& GetTelnetInTraceClass(void)
{
    static const TraceClass cli_TelnetInTraceClass("CLI_TELNET_IN", Help()
        .AddHelp(Help::LANG_EN, "CLI telnet connection input traces")
        .AddHelp(Help::LANG_FR, "Traces d'entrées d'une connexion telnet CLI"));
    return cli_TelnetInTraceClass;
}
//! @brief Telnet connection trace class singleton redirection.
#define CLI_TELNET_OUT GetTelnetOutTraceClass()
//! @brief Telnet connection trace class singleton.
static const TraceClass& GetTelnetOutTraceClass(void)
{
    static const TraceClass cli_TelnetOutTraceClass("CLI_TELNET_OUT", Help()
        .AddHelp(Help::LANG_EN, "CLI telnet connection output traces")
        .AddHelp(Help::LANG_FR, "Traces de sorties d'une connexion telnet CLI"));
    return cli_TelnetOutTraceClass;
}


TelnetServer::TelnetServer(const unsigned int UI_MaxConnections, const unsigned long UL_TcpPort, const ResourceString::LANG E_Lang)
  : m_iServerSocket(-1), m_ulTcpPort(UL_TcpPort), m_eLang(E_Lang),
    m_tkConnections(UI_MaxConnections), m_uiMaxConnections(UI_MaxConnections)
{
    GetTraces().Declare(CLI_TELNET_SERVER);
    GetTraces().Declare(CLI_TELNET_IN);
    GetTraces().Declare(CLI_TELNET_OUT);
}

TelnetServer::~TelnetServer(void)
{
}

void TelnetServer::StartServer(void)
{
    GetTraces().Trace(CLI_TELNET_SERVER) << "Starting server on port " << (uint32_t) m_ulTcpPort << endl;

    m_iServerSocket = socket(AF_INET, SOCK_STREAM, 0);
#ifdef CLI_WIN_NETWORK
    // Winsock needs to be initialized.
    if (m_iServerSocket < 0)
    {
        WORD wVersionRequested = MAKEWORD(2, 2);
        WSADATA wsaData;
        int err = WSAStartup(wVersionRequested, & wsaData);
        if (err == 0)
        {
            ((int&) m_iServerSocket) = socket(AF_INET, SOCK_STREAM, 0);
        }
    }
#endif
    if (m_iServerSocket < 0)
    {
        const ResourceString cli_Error = ResourceString()
            .SetString(ResourceString::LANG_EN, "socket() failed")
            .SetString(ResourceString::LANG_FR, "Echec de la fonction socket()");
        OutputDevice::GetStdErr() << cli_Error.GetString(m_eLang) << endl;
        GetTraces().Trace(CLI_TELNET_SERVER) << "errno = " << socket_errno << endl;
        return;
    }
    GetTraces().Trace(CLI_TELNET_SERVER) << "socket() successful (m_iServerSocket = " << m_iServerSocket << ")" << endl;

    // So that we can re-bind to it without TIME_WAIT problems
    int i_ReuseAddr = 1;
    if (setsockopt(m_iServerSocket, SOL_SOCKET, SO_REUSEADDR, (const char*) & i_ReuseAddr, sizeof(i_ReuseAddr)) != 0)
    {
        close(m_iServerSocket);
        const ResourceString cli_Error = ResourceString()
            .SetString(ResourceString::LANG_EN, "setsockopt() failed")
            .SetString(ResourceString::LANG_FR, "Echec de la fonction setsockopt()");
        OutputDevice::GetStdErr() << cli_Error.GetString(m_eLang) << endl;
        GetTraces().Trace(CLI_TELNET_SERVER) << "errno = " << socket_errno << endl;
        return;
    }

    // Bind the socket
    struct sockaddr_in s_SockAddr;
    s_SockAddr.sin_family = AF_INET;
    s_SockAddr.sin_port = htons((unsigned short) m_ulTcpPort);
    s_SockAddr.sin_addr.s_addr = INADDR_ANY;

    int i_BindTries = 0;
    while (bind(m_iServerSocket, (struct sockaddr*) & s_SockAddr, sizeof(s_SockAddr)) < 0)
    {
        i_BindTries ++;
#ifndef CLI_WIN_NETWORK
        if ((socket_errno == EADDRINUSE) && (i_BindTries < 2))
#else
        if ((socket_errno == WSAEADDRINUSE) && (i_BindTries < 2))
#endif
        {
            sleep(500);
        }
        else
        {
            close(m_iServerSocket);
            const ResourceString cli_Error = ResourceString()
                .SetString(ResourceString::LANG_EN, "bind() failed")
                .SetString(ResourceString::LANG_FR, "Echec de la fonction bind()");
            OutputDevice::GetStdErr() << cli_Error.GetString(m_eLang) << endl;
            GetTraces().Trace(CLI_TELNET_SERVER) << "errno = " << socket_errno << endl;
            return;
        }
    }
    GetTraces().Trace(CLI_TELNET_SERVER) << "bind(m_iServerSocket) successful" << endl;

    // Make the server listen up to m_uiMaxConnections clients.
    if (listen(m_iServerSocket, m_uiMaxConnections) < 0)
    {
        close(m_iServerSocket);
        const ResourceString cli_Error = ResourceString()
            .SetString(ResourceString::LANG_EN, "listen() failed")
            .SetString(ResourceString::LANG_FR, "Echec de la fonction listen()");
        OutputDevice::GetStdErr() << cli_Error.GetString(m_eLang) << endl;
        GetTraces().Trace(CLI_TELNET_SERVER) << "errno = " << socket_errno << endl;
        return;
    }
    GetTraces().Trace(CLI_TELNET_SERVER) << "listen(m_iServerSocket) successful" << endl;

    GetTraces().Trace(CLI_TELNET_SERVER) << "Waiting for clients..." << endl;
    while (m_iServerSocket >= 0)
    {
        if (! RunLoop(-1))
        {
            break;
        }
    }

    GetTraces().Trace(CLI_TELNET_SERVER) << "Server is turning off..." << endl;
    TerminateServer();

    GetTraces().Trace(CLI_TELNET_SERVER) << "Server is done." << endl;
}

void TelnetServer::StopServer(void)
{
    // First of all, unset the main socket reference to let the server loop know about the stop server procedure.
    const int i_ServerSocket = m_iServerSocket;
    m_iServerSocket = -1;

    // Release server resources
    if (i_ServerSocket >= 0)
    {
        if (close(i_ServerSocket) != 0)
        {
            const ResourceString cli_Error = ResourceString()
                .SetString(ResourceString::LANG_EN, "close(m_iServerSocket) failed")
                .SetString(ResourceString::LANG_FR, "Echec de la fonction close()");
            OutputDevice::GetStdErr() << cli_Error.GetString(m_eLang) << endl;
            GetTraces().Trace(CLI_TELNET_SERVER) << "errno = " << socket_errno << endl;
            return;
        }
        GetTraces().Trace(CLI_TELNET_SERVER) << "close(m_iServerSocket) successful" << endl;
    }
}

const bool TelnetServer::RunLoop(const int I_Milli)
{
    // Prepare a fd_set structure to call select().
    fd_set fd_Sockets; FD_ZERO(& fd_Sockets);
    // Peek the server socket to detect new connections.
    FD_SET(m_iServerSocket, & fd_Sockets);
    // Peek known connections for input data and remember each known socket (for consistency).
    tk::Queue<int> tk_Sockets(m_uiMaxConnections);
    for (   tk::Map<int, ConnectionInfo>::Iterator it = m_tkConnections.GetIterator();
            m_tkConnections.IsValid(it);
            m_tkConnections.MoveNext(it))
    {
        const ConnectionInfo cli_Info = m_tkConnections.GetAt(it);
        FD_SET(cli_Info.i_Socket, & fd_Sockets);
        tk_Sockets.AddTail(cli_Info.i_Socket);
    }

    // Set timeout as required.
    timeval t_Timeout, *pt_Timeout = NULL;
    if (I_Milli >= 0)
    {
        t_Timeout.tv_sec = I_Milli / 1000;
        t_Timeout.tv_usec = (I_Milli * 1000) % 1000000;
        pt_Timeout = & t_Timeout;
    }

    // Check the server is still running.
    if (m_iServerSocket < 0)
    {
        return false;
    }

    // Use select.
    GetTraces().Trace(CLI_TELNET_SERVER) << "calling select()..." << endl;
    const int i_Select = select(FD_SETSIZE, & fd_Sockets, NULL, NULL, pt_Timeout);
    GetTraces().Trace(CLI_TELNET_SERVER) << "select() returned " << i_Select << endl;
    if (i_Select < 0)
    {
        GetTraces().Trace(CLI_TELNET_SERVER) << "select() failed, errno = " << socket_errno << endl;
        return false;
    }

    // Check the server is still running.
    if (m_iServerSocket < 0)
    {
        return false;
    }

    // Are there new connections?
    if (FD_ISSET(m_iServerSocket, & fd_Sockets))
    {
        GetTraces().Trace(CLI_TELNET_SERVER) << "FD_ISSET(m_iServerSocket, & fd_Sockets) = " << FD_ISSET(m_iServerSocket, & fd_Sockets) << endl;
        // New client.
        if (! AcceptConnection())
        {
            // If the client cannot be accepted, let's continue whatever.
            //return false;
        }
    }

    // Is there data for known connections?
    while (! tk_Sockets.IsEmpty())
    {
        const int i_ConnectionSocket = tk_Sockets.RemoveHead();
        if (const ConnectionInfo* const pcli_Info = m_tkConnections.GetAt(i_ConnectionSocket))
        {
            if (FD_ISSET(pcli_Info->i_Socket, & fd_Sockets))
            {
                if (TelnetConnection* const pcli_Connection = const_cast<TelnetConnection*>(pcli_Info->pcli_Connection))
                {
                    GetTraces().Trace(CLI_TELNET_SERVER)
                        << "FD_ISSET(" << pcli_Info->i_Socket << " , & fd_Sockets) "
                        << "= " << FD_ISSET(pcli_Info->i_Socket, & fd_Sockets) << endl;

                    bool b_Res = false;
                    if (pcli_Connection->ReceiveChars())
                    {
                        if (pcli_Connection->ProcessKeys())
                        {
                            b_Res = true;
                        }
                    }
                    if (! b_Res)
                    {
                        // [contrib: Dinesh Balasubramaniam, 2014, based on CLI 2.8]
                        // Client error / termination. Release socket resources so that pending/subsequent clients can be handled.
                        GetTraces().SafeTrace(CLI_TELNET_SERVER, *pcli_Connection) << "Error while receiving/processing keys. Closing telnet connection for socket " << i_ConnectionSocket << endl;
                        CloseConnection(i_ConnectionSocket);
                    }
                }
            }
        }
    }

    return true;
}

const bool TelnetServer::AcceptConnection(void)
{
    // Accept the client
    const int i_ConnectionSocket = accept(m_iServerSocket, NULL, NULL);
    if (i_ConnectionSocket < 0)
    {
        const ResourceString cli_Error = ResourceString()
            .SetString(ResourceString::LANG_EN, "accept() failed")
            .SetString(ResourceString::LANG_FR, "Echec de la fonction accept()");
        OutputDevice::GetStdErr() << cli_Error.GetString(m_eLang) << endl;
        GetTraces().Trace(CLI_TELNET_SERVER) << "errno = " << socket_errno << endl;
        return false;
    }
    GetTraces().Trace(CLI_TELNET_SERVER) << "accept() successful (i_ConnectionSocket = " << i_ConnectionSocket << ")" << endl;
    const ResourceString cli_InfoClient = ResourceString()
        .SetString(ResourceString::LANG_EN, "Accepting telnet connection")
        .SetString(ResourceString::LANG_FR, "Connexion telnet acceptée");
    GetTraces().Trace(CLI_TELNET_SERVER) << cli_InfoClient.GetString(m_eLang) << endl;

    // TelnetConnection / ExecutionContext configurations
    if (TelnetConnection* const pcli_Connection = new TelnetConnection(this, i_ConnectionSocket, m_eLang, true))
    {
        // First check we do not have too many connections.
        GetTraces().Trace(CLI_TELNET_SERVER) << "Already " << m_tkConnections.GetCount() << " active connections" << endl;
        if (m_tkConnections.GetCount() >= m_uiMaxConnections)
        {
            const ResourceString cli_Error = ResourceString()
                .SetString(ResourceString::LANG_EN, "Too many connections!")
                .SetString(ResourceString::LANG_FR, "Nombre de connexions dépassé.");
            GetTraces().Trace(CLI_TELNET_SERVER) << cli_Error.GetString(m_eLang) << endl;
            if (pcli_Connection->OpenUp(__CALL_INFO__))
            {
                *pcli_Connection << cli_Error.GetString(m_eLang) << endl;
                const ResourceString cli_Sorry = ResourceString()
                    .SetString(ResourceString::LANG_EN, "Too many connections! Sorry.")
                    .SetString(ResourceString::LANG_FR, "Nombre de connexions dépassé. Désolé.");
                *pcli_Connection << cli_Sorry.GetString(m_eLang) << endl;
                pcli_Connection->CloseDown(__CALL_INFO__);
            }
        }
        else
        {
            // Require an execution context for the given connection.
            if (ExecutionContext* const pcli_Context = OnNewConnection(*pcli_Connection))
            {
                // Store in the connection registry.
                const ConnectionInfo s_ConnectionInfo = { i_ConnectionSocket, pcli_Connection, pcli_Context };
                if ((! m_tkConnections.IsSet(i_ConnectionSocket))
                    && m_tkConnections.SetAt(i_ConnectionSocket, s_ConnectionInfo))
                {
                    // Run the shell on that telnet connection.
                    pcli_Connection->UseInstance(__CALL_INFO__);
                    pcli_Context->Run(*pcli_Connection);

                    // ExecutionContext automatically configures the stream as a trace stream.
                    // However in case of telnet connections, this may not be convenient by default.
                    GetTraces().UnsetStream(*pcli_Connection);

                    // Since telnet connections are non-blocking devices, let's return successfully right now.
                    return true;
                }

                // Fallback release.
                OnCloseConnection(*pcli_Connection, pcli_Context);
            }
        }
        // Fallback release.
        delete pcli_Connection;
    }
    // Fallback release.
    close(i_ConnectionSocket);
    return false;
}

const bool TelnetServer::CloseConnection(const int I_ConnectionSocket)
{
    GetTraces().Trace(CLI_TELNET_SERVER) << "Ending connection " << I_ConnectionSocket << "..." << endl;

    // Release objects.
    if (const ConnectionInfo* const pcli_Info = m_tkConnections.GetAt(I_ConnectionSocket))
    {
        if (TelnetConnection* const pcli_Connection = pcli_Info->pcli_Connection)
        {
            if (ExecutionContext* const pcli_Context = pcli_Info->pcli_Context)
            {
                OnCloseConnection(*pcli_Connection, pcli_Context);
            }
            pcli_Connection->FreeInstance(__CALL_INFO__);
        }
        m_tkConnections.Unset(I_ConnectionSocket);
    }

    return true;
}

const bool TelnetServer::TerminateServer(void)
{
    bool b_Res = true;

    // Close all client connections.
    while (! m_tkConnections.IsEmpty())
    {
        tk::Map<int, ConnectionInfo>::Iterator it = m_tkConnections.GetIterator();
        if (m_tkConnections.IsValid(it))
        {
            const ConnectionInfo cli_Info = m_tkConnections.Remove(it);

            // Let the shell terminate.
            if (cli_Info.pcli_Context != NULL)
            {
                if (cli_Info.pcli_Context->IsRunning())
                {
                    GetTraces().Trace(CLI_TELNET_SERVER) << "Terminating shell of connection " << cli_Info.i_Socket << "..." << cli::endl;
                    cli_Info.pcli_Context->StopExecution();
                }
            }
        }
    }

    return b_Res;
}


TelnetConnection::TelnetConnection(
        TelnetServer* const PCLI_Server,
        const int I_ConnectionSocket, const ResourceString::LANG E_Lang,
        const bool B_AutoDelete)
  : NonBlockingIODevice("telnet", B_AutoDelete),
    m_pcliServer(PCLI_Server),
    m_iSocket(I_ConnectionSocket), m_eLang(E_Lang),
    m_qChars(CLI_TELNET_INPUT_BUFFER_SIZE), m_bWaitingForKeys(false)
{
}

TelnetConnection::~TelnetConnection(void)
{
}

const bool TelnetConnection::OpenDevice(void)
{
    // Configure LINGER option
    linger t_Linger;
    t_Linger.l_onoff = 1; // enable LINGER
    t_Linger.l_linger = 1; // seconds
    if (setsockopt(m_iSocket, SOL_SOCKET, SO_LINGER, (const char*) & t_Linger, sizeof(t_Linger)) != 0)
    {
        const ResourceString cli_Error = ResourceString()
            .SetString(ResourceString::LANG_EN, "setsockopt(SO_LINGER) failed")
            .SetString(ResourceString::LANG_FR, "Echec de la fonction setsockopt(SO_LINGER)");
        OutputDevice::GetStdErr() << cli_Error.GetString(m_eLang) << endl;
        GetTraces().SafeTrace(CLI_TELNET_SERVER, *this) << "errno = " << socket_errno << endl;
        CloseDevice();
        return false;
    }
    GetTraces().SafeTrace(CLI_TELNET_SERVER, *this) << "setsockopt(" << m_iSocket << ", SO_LINGER) successful" << endl;

    return true;
}

const bool TelnetConnection::CloseDevice(void)
{
    // Did not manage to correctly end up the connection on the server side,
    // neither on Windows nor Linux systems.
    // This a hack until I provide another implementation.
    {
        linger t_Linger; socklen_t i_Len = sizeof(t_Linger);
        if (getsockopt(m_iSocket, SOL_SOCKET, SO_LINGER, (char*) & t_Linger, & i_Len) != 0)
        {
            close(m_iSocket);
            const ResourceString cli_Error = ResourceString()
                .SetString(ResourceString::LANG_EN, "getsockopt(SO_LINGER) failed")
                .SetString(ResourceString::LANG_FR, "Echec de la fonction getsockopt(SO_LINGER)");
            OutputDevice::GetStdErr() << cli_Error.GetString(m_eLang) << endl;
            GetTraces().SafeTrace(CLI_TELNET_SERVER, *this) << "errno = " << socket_errno << endl;
            return false;
        }
        GetTraces().SafeTrace(CLI_TELNET_SERVER, *this) << "getsockopt(" << m_iSocket << ", SO_LINGER): onoff = " << t_Linger.l_onoff << endl;
        GetTraces().SafeTrace(CLI_TELNET_SERVER, *this) << "getsockopt(" << m_iSocket << ", SO_LINGER): linger = " << t_Linger.l_linger << endl;

        if (t_Linger.l_onoff)
        {
            sleep(t_Linger.l_linger);
        }
    }

    // Stop sending characters
    if (shutdown(m_iSocket, SHUT_WR) != 0)
    {
        close(m_iSocket);
        const ResourceString cli_Error = ResourceString()
            .SetString(ResourceString::LANG_EN, "shutdown() failed")
            .SetString(ResourceString::LANG_FR, "Echec de la fonction shutdown()");
        OutputDevice::GetStdErr() << cli_Error.GetString(m_eLang) << endl;
        GetTraces().SafeTrace(CLI_TELNET_SERVER, *this) << "errno = " << socket_errno << endl;
        return false;
    }
    GetTraces().SafeTrace(CLI_TELNET_SERVER, *this) << "shutdown(" << m_iSocket << ") successful" << endl;

    // Release resources
    if (close(m_iSocket) != 0)
    {
        const ResourceString cli_Error = ResourceString()
            .SetString(ResourceString::LANG_EN, "close() failed")
            .SetString(ResourceString::LANG_FR, "Echec de la fonction close()");
        OutputDevice::GetStdErr() << cli_Error.GetString(m_eLang) << endl;
        GetTraces().SafeTrace(CLI_TELNET_SERVER, *this) << "errno = " << socket_errno << endl;
        return false;
    }
    GetTraces().SafeTrace(CLI_TELNET_SERVER, *this) << "close(" << m_iSocket << ") successful" << endl;

    return true;
}

const bool TelnetConnection::ReceiveChars(void) const
{
    // Dequeue characters from the socket
    GetTraces().SafeTrace(CLI_TELNET_IN, *this) << "m_iSocket = " << m_iSocket << ", calling recv()..." << endl;
    char arc_Buffer[256];
    const int i_LenMax = ((CLI_TELNET_INPUT_BUFFER_SIZE - m_qChars.GetCount() < 256) ? (CLI_TELNET_INPUT_BUFFER_SIZE - m_qChars.GetCount()) : 256);
    const int i_Len = recv(m_iSocket, arc_Buffer, i_LenMax, 0);
    GetTraces().SafeTrace(CLI_TELNET_IN, *this) << "recv() returned " << i_Len << endl;
    if (i_Len <= 0)
    {
        m_cliLastError
            .SetString(ResourceString::LANG_EN, "Telnet reception error")
            .SetString(ResourceString::LANG_FR, "Echec de réception en telnet");
        GetTraces().SafeTrace(CLI_TELNET_IN, *this) << "recv() failed, errno = " << socket_errno << endl;
        return false; // [contrib: Dinesh Balasubramaniam, 2014, based on CLI 2.8]
    }

    // Queue them in the internal fifo
    for (int i=0; i<i_Len; i++)
    {
        if (arc_Buffer[i] != '\0')
        {
            if (! m_qChars.AddTail(arc_Buffer[i]))
            {
                // In as much as i_LenMax has been computed on the amount of available space in m_qChars, that should never happen.
                // Yet, let's handle the error.
                GetTraces().SafeTrace(INTERNAL_ERROR, *this)
                    << "TelnetConnection::ReceiveChars(): "
                    << "Could not append character '" << arc_Buffer[i] << "' "
                    << "to m_qChars" << endl;
                return false;
            }
        }
    }

    return true;
}

const bool TelnetConnection::ProcessKeys(void) const
{
    while (CheckUp() && (! m_qChars.IsEmpty()))
    {
        const KEY e_Key = GetKey();
        if (e_Key == NULL_KEY)
        {
            return false;
        }
        else
        {
            OnKey(e_Key);
        }
    }

    return true;
}

const bool TelnetConnection::CheckUp(void) const
{
    const ExecutionContext* const pcli_ExecutionContext = NonBlockingIODevice::GetExecutionContext();
    if ((pcli_ExecutionContext == NULL) || (! pcli_ExecutionContext->IsRunning()))
    {
        // The shell is done. Hang up!
        if (m_pcliServer != NULL)
        {
            GetTraces().SafeTrace(CLI_TELNET_SERVER, *this) << "Shell is done. Closing telnet connection for socket " << m_iSocket << endl;
            m_pcliServer->CloseConnection(m_iSocket);
        }
        else
        {
            const_cast<TelnetConnection*>(this)->CloseDevice();
        }
        return false;
    }

    return true;
}

const KEY TelnetConnection::GetKey(void) const
{
    // Wait for characters if needed
    if (m_qChars.GetCount() < 10)
    {
        GetTraces().SafeTrace(CLI_TELNET_IN, *this) << "m_qChars.GetCount() = " << m_qChars.GetCount() << endl;

        // Just peek the socket to read if there are characters remaining
        fd_set fd_Read; FD_ZERO(& fd_Read); FD_SET(m_iSocket, & fd_Read);
        timeval t_Timeout; t_Timeout.tv_sec = 0; t_Timeout.tv_usec = 10000;
        const int i_Select = select(FD_SETSIZE, & fd_Read, NULL, NULL, & t_Timeout);
        GetTraces().SafeTrace(CLI_TELNET_IN, *this) << "select() returned " << i_Select << endl;
        if (i_Select < 0)
        {
            m_cliLastError
                .SetString(ResourceString::LANG_EN, "Telnet reception error")
                .SetString(ResourceString::LANG_FR, "Echec de réception en telnet");
            GetTraces().SafeTrace(CLI_TELNET_SERVER, *this) << "select() failed, errno = " << socket_errno << endl;
            return NULL_KEY;
        }

        GetTraces().SafeTrace(CLI_TELNET_IN, *this) << "FD_ISSET(m_iSocket, & fd_Read) = " << FD_ISSET(m_iSocket, & fd_Read) << endl;
        if ((i_Select > 0) && FD_ISSET(m_iSocket, & fd_Read))
        {
            GetTraces().SafeTrace(CLI_TELNET_IN, *this) << "Calling ReceiveChars() from TelnetConnection::GetKey()" << endl;
            if (! ReceiveChars())
            {
                return NULL_KEY;
            }
        }
    }

    // Process characters
    if (! m_qChars.IsEmpty())
    {
        // Dequeue the received characters
        const int i_Front = m_qChars.RemoveHead();
        GetTraces().SafeTrace(CLI_TELNET_IN, *this) << "i_Char = " << (char) i_Front << " (" << i_Front << ")" << endl;

        // Special character management.
        switch (i_Front)
        {
        case 1:     return KEY_BEGIN;   // CTRL+A
        case 3:     return BREAK;       // CTRL+C
        case 4:     return LOGOUT;      // CTRL+D
        case 5:     return KEY_END;     // CTRL+E
        case 8:     return BACKSPACE;   // CTRL+H
        case 10:    return ENTER;
        case 13:
            if (! m_qChars.IsEmpty())
            {
                GetTraces().SafeTrace(CLI_TELNET_IN, *this) << "i_Char2 = " << (char) m_qChars.GetHead() << " (" << m_qChars.GetHead() << ")" << endl;
                if (m_qChars.GetHead() == 10)
                {
                    // Sequence is 13-10 => ENTER
                    m_qChars.RemoveHead();
                    return ENTER;
                }
            }
            break;
        case 12:    return CLS;         // CTRL+L
        case 14:    return NEXT;        // CTRL+N
        case 16:    return PREVIOUS;    // CTRL+P
        case 25:    return REDO;        // CTRL+Y
        case 26:    return UNDO;        // CTRL+Z
        case 27:
            if (! m_qChars.IsEmpty())
            {
                GetTraces().SafeTrace(CLI_TELNET_IN, *this) << "i_Char2 = " << (char) m_qChars.GetHead() << " (" << m_qChars.GetHead() << ")" << endl;
                if (m_qChars.GetHead() == 91)
                {
                    m_qChars.RemoveHead();

                    if (! m_qChars.IsEmpty())
                    {
                        const int i_Special = m_qChars.RemoveHead();
                        GetTraces().SafeTrace(CLI_TELNET_IN, *this) << "i_Char3 = " << (char) i_Special << " (" << i_Special << ")" << endl;

                        switch (i_Special)
                        {
                        // All characters that will be followed by a 126 character.
                        case 49: // INSERT
                        case 50: // KEY_BEGIN;
                        case 51: // PAGE_UP;
                        case 52: // DELETE;
                        case 53: // KEY_END;
                        case 54: // PAGE_DOWN;
                            if (! m_qChars.IsEmpty())
                            {
                                const int i_Finish = m_qChars.RemoveHead();
                                GetTraces().SafeTrace(CLI_TELNET_IN, *this) << "i_Char4 = " << (char) i_Finish << " (" << i_Finish << ")" << endl;

                                if (i_Finish == 126)
                                {
                                    switch (i_Special)
                                    {
                                    case 49: return INSERT;
                                    case 50: return KEY_BEGIN;
                                    case 51: return PAGE_UP;
                                    case 52: return DELETE;
                                    case 53: return KEY_END;
                                    case 54: return PAGE_DOWN;
                                    }
                                }
                                else if ((i_Special == 49) && (! m_qChars.IsEmpty()))
                                {
                                    // Function keys.
                                    const int i_Function = i_Finish;
                                    const int i_FunctionFinish = m_qChars.RemoveHead();
                                    GetTraces().SafeTrace(CLI_TELNET_IN, *this) << "i_Char5 = " << (char) i_FunctionFinish << " (" << i_FunctionFinish << ")" << endl;

                                    if (i_FunctionFinish == 126)
                                    {
                                        switch (i_Function)
                                        {
                                        case 49: return F1;
                                        case 50: return F2;
                                        case 51: return F3;
                                        case 52: return F4;
                                        case 53: return F5;
                                        case 55: return F6;
                                        case 56: return F7;
                                        case 57: return F8;
                                        }
                                    }

                                    // Unhandled.
                                    if (! m_qChars.AddHead(i_FunctionFinish))
                                    {
                                        GetTraces().SafeTrace(INTERNAL_ERROR, *this)
                                            << "TelnetConnection::GetKey(): "
                                            << "Unable to restore previously removed character from m_qChars"
                                            << endl;
                                    }
                                }
                                else if ((i_Special == 50) && (! m_qChars.IsEmpty()))
                                {
                                    // Function keys.
                                    const int i_Function = i_Finish;
                                    const int i_FunctionFinish = m_qChars.RemoveHead();
                                    GetTraces().SafeTrace(CLI_TELNET_IN, *this) << "i_Char5 = " << (char) i_FunctionFinish << " (" << i_FunctionFinish << ")" << endl;

                                    if (i_FunctionFinish == 126)
                                    {
                                        switch (i_Function)
                                        {
                                        case 48: return F9;
                                        case 49: return F10;
                                        case 51: return F11;
                                        case 52: return F12;
                                        }
                                    }

                                    // Unhandled.
                                    if (! m_qChars.AddHead(i_FunctionFinish))
                                    {
                                        GetTraces().SafeTrace(INTERNAL_ERROR, *this)
                                            << "TelnetConnection::GetKey(): "
                                            << "Unable to restore previously removed character from m_qChars"
                                            << endl;
                                    }
                                }

                                // Unhandled.
                                if (! m_qChars.AddHead(i_Finish))
                                {
                                    GetTraces().SafeTrace(INTERNAL_ERROR, *this)
                                        << "TelnetConnection::GetKey(): "
                                        << "Unable to restore previously removed character from m_qChars"
                                        << endl;
                                }
                            }
                        case 65: return KEY_UP;
                        case 66: return KEY_DOWN;
                        case 67: return KEY_RIGHT;
                        case 68: return KEY_LEFT;
                        }

                        // Unhandled.
                        if (! m_qChars.AddHead(i_Special))
                        {
                            GetTraces().SafeTrace(INTERNAL_ERROR, *this)
                                << "TelnetConnection::GetKey(): "
                                << "Unable to restore previously removed character from m_qChars"
                                << endl;
                        }
                    }

                    // Unhandled.
                    if (! m_qChars.AddHead(91))
                    {
                        GetTraces().SafeTrace(INTERNAL_ERROR, *this)
                            << "TelnetConnection::GetKey(): "
                            << "Unable to restore previously removed character from m_qChars"
                            << endl;
                    }
                }
            }
            else
            {
                return ESCAPE;
            }
            break;
        case 96:    return BACK_QUOTE;
        case 126:   return TILDE;
        case 127:   return DELETE;

        // Accented characters.
        case -31:   return KEY_aacute;
        case -32:   return KEY_agrave;
        case -28:   return KEY_auml;
        case -30:   return KEY_acirc;
        case -25:   return KEY_ccedil;
        case -23:   return KEY_eacute;
        case -24:   return KEY_egrave;
        case -21:   return KEY_euml;
        case -22:   return KEY_ecirc;
        case -19:   return KEY_iacute;
        case -20:   return KEY_igrave;
        case -17:   return KEY_iuml;
        case -18:   return KEY_icirc;
        case -13:   return KEY_oacute;
        case -14:   return KEY_ograve;
        case -10:   return KEY_ouml;
        case -12:   return KEY_ocirc;
        case -6:    return KEY_uacute;
        case -7:    return KEY_ugrave;
        case -4:    return KEY_uuml;
        case -5:    return KEY_ucirc;
        // Accented characters copied from the output itself.
        case -96:   return KEY_aacute;
        case -123:  return KEY_agrave;
        case -124:  return KEY_auml;
        case -125:  return KEY_acirc;
        case -121:  return KEY_ccedil;
        case -126:  return KEY_eacute;
        case -118:  return KEY_egrave;
        case -119:  return KEY_euml;
        case -120:  return KEY_ecirc;
        case -95:   return KEY_iacute;
        case -115:  return KEY_igrave;
        case -117:  return KEY_iuml;
        case -116:  return KEY_icirc;
        case -94:   return KEY_oacute;
        case -107:  return KEY_ograve;
        case -108:  return KEY_ouml;
        case -109:  return KEY_ocirc;
        //case -93:   return KEY_uacute;    // Conflicts with POUND!
        case -105:  return KEY_ugrave;
        case -127:  return KEY_uuml;
        case -106:  return KEY_ucirc;

        // Special characters.
        case -75:   return MICRO;
        case -26:   return MICRO;       // Copied from the output itself.
        case -78:   return SQUARE;
        case -3:    return SQUARE;      // Copied from the output itself.
        case -80:   return DEGREE;
        case -8:    return DEGREE;      // Copied from the output itself.
        case -87:   return COPYRIGHT;
        case -72:   return COPYRIGHT;   // Copied from the output itself.
        case -89:   return PARAGRAPH;
        case -11:   return PARAGRAPH;   // Copied from the output itself.
        case -93:   return POUND;
        case -100:  return POUND;       // Copied from the output itself.
        case -128:  return EURO;
        case -79:   return EURO;    // Copied from the output itself.
        }

        // Unhandled
        // Use default behaviour.
        const KEY e_Char = Char2Key(i_Front);
        if (e_Char == NULL_KEY)
        {
            // Unknown character.
            return GetKey();
        }
        else if (e_Char == FEED_MORE)
        {
            // More characters needed
            return GetKey();
        }
        else
        {
            return e_Char;
        }
    }

    return NULL_KEY;
}

void TelnetConnection::OnKey(const KEY E_Key) const
{
    if (m_bWaitingForKeys)
    {
        m_bWaitingForKeys = false;
    }
    NonBlockingIODevice::OnKey(E_Key);
}

void TelnetConnection::PutString(const char* const STR_Out) const
{
    if (STR_Out != NULL)
    {
        const int i_InLen = (int) strlen(STR_Out);
        int i_OutLen = i_InLen;
        for (int i=0; i<i_InLen; i++) {
            if (STR_Out[i] == '\n') {
                i_OutLen ++;
            }
        }
        if (i_OutLen > 0)
        if (char* const str_Out = new char[i_OutLen + 1])
        {
            memset(str_Out, '\0', i_OutLen + 1);
            int o = 0;

            // Conversions.
            StringDecoder cli_Out(STR_Out);
            for (KEY e_Key = cli_Out.GetKey(); e_Key != NULL_KEY; e_Key = cli_Out.GetKey())
            {
                switch (e_Key)
                {
                case KEY_aacute:    str_Out[o++] = (char) -96;  break;
                case KEY_agrave:    str_Out[o++] = (char) -123; break;
                case KEY_auml:      str_Out[o++] = (char) -124; break;
                case KEY_acirc:     str_Out[o++] = (char) -125; break;
                case KEY_ccedil:    str_Out[o++] = (char) -121; break;
                case KEY_eacute:    str_Out[o++] = (char) -126; break;
                case KEY_egrave:    str_Out[o++] = (char) -118; break;
                case KEY_euml:      str_Out[o++] = (char) -119; break;
                case KEY_ecirc:     str_Out[o++] = (char) -120; break;
                case KEY_iacute:    str_Out[o++] = (char) -95;  break;
                case KEY_igrave:    str_Out[o++] = (char) -115; break;
                case KEY_iuml:      str_Out[o++] = (char) -117; break;
                case KEY_icirc:     str_Out[o++] = (char) -116; break;
                case KEY_oacute:    str_Out[o++] = (char) -94;  break;
                case KEY_ograve:    str_Out[o++] = (char) -107; break;
                case KEY_ouml:      str_Out[o++] = (char) -108; break;
                case KEY_ocirc:     str_Out[o++] = (char) -109; break;
                case KEY_uacute:    str_Out[o++] = (char) -93;  break;
                case KEY_ugrave:    str_Out[o++] = (char) -105; break;
                case KEY_uuml:      str_Out[o++] = (char) -127; break;
                case KEY_ucirc:     str_Out[o++] = (char) -106; break;
                case SQUARE:        str_Out[o++] = (char) -3;   break;
                case EURO:          str_Out[o++] = (char) -79;  break;
                case POUND:         str_Out[o++] = (char) -100; break;
                case MICRO:         str_Out[o++] = (char) -26;  break;
                case PARAGRAPH:     str_Out[o++] = (char) -11;  break;
                case DEGREE:        str_Out[o++] = (char) -8;   break;
                case COPYRIGHT:     str_Out[o++] = (char) -72;  break;
                case ENTER:         str_Out[o++] = '\r'; str_Out[o++] = '\n'; break;
                default:            str_Out[o++] = (char) e_Key; break;
                }
            }
            // Due to conversions above, i_OutLen may be shorter than expected.
            i_OutLen = (int) strlen(str_Out);

            // Send the buffer.
            for (int r=0; r<i_OutLen; r++)
            {
                GetTraces().SafeTrace(CLI_TELNET_OUT, *this) << "sending " << (char) str_Out[r] << endl;
            }
            const int i_Len = send(m_iSocket, str_Out, i_OutLen, 0);
            if (i_Len != i_OutLen)
            {
                m_cliLastError
                    .SetString(ResourceString::LANG_EN, "Telnet emission error")
                    .SetString(ResourceString::LANG_FR, "Echec d'émission en telnet");

                if (i_Len < 0)
                {
                    GetTraces().SafeTrace(CLI_TELNET_OUT, *this) << "send failed" << endl;
                }
                else if (i_Len < i_OutLen)
                {
                    GetTraces().SafeTrace(CLI_TELNET_OUT, *this)
                        << "send incomplete"
                        << " (only " << i_Len << " characters sent"
                        << " over " << i_OutLen << ")" << endl;
                }
                else if (i_Len > i_OutLen)
                {
                    GetTraces().SafeTrace(CLI_TELNET_OUT, *this)
                        << "strange send return value"
                        << " (" << i_Len << " for " << i_OutLen << " characters sent)" << endl;
                }
            }

            // Free the local output buffer.
            delete [] str_Out;
        }
    }
}

void TelnetConnection::Beep(void) const
{
    NonBlockingIODevice::Beep();
}

void TelnetConnection::CleanScreen(void) const
{
    NonBlockingIODevice::CleanScreen();
}
