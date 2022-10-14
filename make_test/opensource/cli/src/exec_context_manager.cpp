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

#include "cli/non_blocking_io_device.h"
#include "cli/traces.h"
#include "cli/assert.h"

#include "exec_context_manager.h"
#include "constraints.h"
#include "consistency.h"

CLI_NS_USE(cli)


//! @brief Execution context trace class singleton.
const TraceClass& ExecutionContextManager::GetTraceClass(void)
{
    static const TraceClass cli_ExecCtxTraceClass("CLI_EXEC_CTX", Help()
        .AddHelp(Help::LANG_EN, "Execution context traces")
        .AddHelp(Help::LANG_FR, "Traces de contexte d'exécution"));
    return cli_ExecCtxTraceClass;
}

ExecutionContextManager::ExecutionContextManager(void)
  : m_pcliInput(NULL), m_eLang(Help::LANG_EN), m_bBeep(true),
    m_tkUserInstances(MAX_EXECUTION_CONTEXTS), m_tkRunningContexts(MAX_EXECUTION_CONTEXTS)
{
    EnsureCommonDevices();
    EnsureTraces();

    // Members initialization.
    for (int i=0; i<STREAM_TYPES_COUNT; i++)
    {
        m_artStream[i].pcliStream = NULL;
        m_artStream[i].bEnable = true;
    }

    GetTraces().Trace(TRACE_EXEC_CTX) << "Execution context manager " << this << ": New execution context manager." << endl;
}

ExecutionContextManager::~ExecutionContextManager(void)
{
    CLI_ASSERT(m_tkUserInstances.IsEmpty());
    //CLI_ASSERT(m_tkRunningContexts.IsEmpty()); // We cannot ensure everything is correctly stopped on destruction.

    GetTraces().Trace(TRACE_EXEC_CTX) << "Execution context manager " << this << ": End of execution context manager." << endl;

    ReleaseInput();

    for (int i=0; i<STREAM_TYPES_COUNT; i++)
    {
        if (m_artStream[i].pcliStream != NULL)
        {
            m_artStream[i].pcliStream->FreeInstance(__CALL_INFO__);
            m_artStream[i].pcliStream = NULL;
        }
    }
}

void ExecutionContextManager::UseInstance(ExecutionContext& CLI_UserInstance)
{
    m_tkUserInstances.AddHead(& CLI_UserInstance);
    GetTraces().Trace(TRACE_EXEC_CTX) << "Execution context manager " << this << ": One more user " << & CLI_UserInstance << " for this manager -> " << m_tkUserInstances.GetCount() << " users." << endl;
}

void ExecutionContextManager::FreeInstance(ExecutionContext& CLI_UserInstance)
{
    // First of all, remove the instance from execution list.
    CloseDown(CLI_UserInstance);

    for (tk::Queue<ExecutionContext*>::Iterator it = m_tkUserInstances.GetIterator(); m_tkUserInstances.IsValid(it); m_tkUserInstances.MoveNext(it))
    {
        if (const ExecutionContext* const pcli_ExecutionContext = m_tkUserInstances.GetAt(it))
        {
            if (pcli_ExecutionContext == & CLI_UserInstance)
            {
                m_tkUserInstances.Remove(it);
                break;
            }
        }
    }
    GetTraces().Trace(TRACE_EXEC_CTX) << "Execution context manager " << this << ": One less user " << & CLI_UserInstance << " for this manager -> " << m_tkUserInstances.GetCount() << " users." << endl;

    // Auto-deletion management.
    if (m_tkUserInstances.IsEmpty())
    {
        GetTraces().Trace(TRACE_EXEC_CTX) << "Execution context manager " << this << ": Auto-deletion." << endl;
        delete this;
    }
}

void ExecutionContextManager::StopAllExecutions(void)
{
    GetTraces().Trace(TRACE_EXEC_CTX) << "Execution context manager " << this << ": Stopping all executions." << endl;

    tk::Queue<ExecutionContext*> tk_Contexts(m_tkRunningContexts);
    while (! tk_Contexts.IsEmpty())
    {
        if (ExecutionContext* const pcli_Context = tk_Contexts.RemoveHead())
        {
            pcli_Context->StopExecution();
        }
    }
}

const bool ExecutionContextManager::OpenUp(ExecutionContext& CLI_Context, IODevice& CLI_IODevice)
{
    GetTraces().Trace(TRACE_EXEC_CTX) << "Execution context manager " << this << ": Starting execution context " << & CLI_Context << "." << endl;

    bool b_Res = true;

    // First of all, register the execution context.
    const bool b_DoOpenDevices = m_tkRunningContexts.IsEmpty();
    m_tkRunningContexts.AddHead(& CLI_Context);
    GetTraces().Trace(TRACE_EXEC_CTX) << "Execution context manager " << this << ": " << m_tkRunningContexts.GetCount() << " execution contexts currently running, "
                                                                                        << "current: " << GetCurrentContext() << "." << endl;
    if (b_DoOpenDevices)
    {
        GetTraces().Trace(TRACE_EXEC_CTX) << "Execution context manager " << this << ": Opening devices." << endl;
    }

    // Then set device references.
    if (b_Res && b_DoOpenDevices)
    {
        // Input.
        ReleaseInput();
        m_pcliInput = & CLI_IODevice;
        m_pcliInput->UseInstance(__CALL_INFO__);

        // Output.
        for (int i=0; i<STREAM_TYPES_COUNT; i++)
        {
            if (m_artStream[i].pcliStream == NULL)
            {
                m_artStream[i].pcliStream = & CLI_IODevice;
                m_artStream[i].pcliStream->UseInstance(__CALL_INFO__);
            }
            m_artStream[i].bEnable = true; // Enable all output streams by default.
        }
    }

    // Open up devices.
    if (b_Res && b_DoOpenDevices)
    {
        // Input.
        if (m_pcliInput == NULL)
        {
            CLI_ASSERT(false);
            b_Res = false;
        }
        else if (! m_pcliInput->OpenUp(__CALL_INFO__))
        {
            GetStream(ERROR_STREAM) << m_pcliInput->GetLastError().GetString(m_eLang) << endl;
            b_Res = false;
        }

        // Output.
        for (int i=0; i<STREAM_TYPES_COUNT; i++)
        {
            if (m_artStream[i].pcliStream == NULL)
            {
                CLI_ASSERT(false);
                b_Res = false;
            }
            else if (! m_artStream[i].pcliStream->OpenUp(__CALL_INFO__))
            {
                GetStream(ERROR_STREAM) << m_artStream[i].pcliStream->GetLastError().GetString(m_eLang) << endl;
                b_Res = false;
            }
        }
    }

    // Trace device management.
    if (b_Res && b_DoOpenDevices)
    {
        if (! GetTraces().SetStream(const_cast<OutputDevice&>(GetStream(ERROR_STREAM))))
        {
            b_Res = false;
        }
        else
        {
            GetTraces().Trace(TRACE_EXEC_CTX) << "Execution context manager " << this << ": " << GetStream(ERROR_STREAM).GetDebugName() << " set for traces." << endl;
        }
    }

    // Non-blocking device management.
    if (b_Res && b_DoOpenDevices)
    {
        if (NonBlockingIODevice* const pcli_NonBlockingDevice = dynamic_cast<NonBlockingIODevice*>(m_pcliInput))
        {
            pcli_NonBlockingDevice->SetExecutionContextManager(this);
        }
    }

    return b_Res;
}

const bool ExecutionContextManager::CloseDown(ExecutionContext& CLI_Context)
{
    GetTraces().Trace(TRACE_EXEC_CTX) << "Execution context manager " << this << ": Ending execution context " << & CLI_Context << "." << endl;

    bool b_Res = true;

    // Determine whether devices should be closed.
    const bool b_DoCloseDevices = ((m_tkRunningContexts.GetCount() == 1) && (m_tkRunningContexts.GetHead() == & CLI_Context));
    if (b_DoCloseDevices)
    {
        GetTraces().Trace(TRACE_EXEC_CTX) << "Execution context manager " << this << ": Closing devices." << endl;
    }

    // Non-blocking device management.
    if (b_DoCloseDevices)
    {
        if (NonBlockingIODevice* const pcli_NonBlockingDevice = dynamic_cast<NonBlockingIODevice*>(m_pcliInput))
        {
            pcli_NonBlockingDevice->SetExecutionContextManager(NULL);
        }
    }

    // Trace device management.
    if (b_DoCloseDevices)
    {
        GetTraces().Trace(TRACE_EXEC_CTX) << "Execution context manager " << this << ": Unsetting " << GetStream(ERROR_STREAM).GetDebugName() << " for traces." << endl;
        if (! GetTraces().UnsetStream(const_cast<OutputDevice&>(GetStream(ERROR_STREAM))))
        {
            b_Res = false;
        }
    }

    // Close down devices.
    if (b_DoCloseDevices)
    {
        // Output.
        for (int i=0; i<STREAM_TYPES_COUNT; i++)
        {
            if (m_artStream[i].pcliStream == NULL)
            {
                CLI_ASSERT(false);
                b_Res = false;
            }
            else if (! m_artStream[i].pcliStream->CloseDown(__CALL_INFO__))
            {
                GetStream(ERROR_STREAM) << m_artStream[i].pcliStream->GetLastError().GetString(m_eLang) << endl;
                b_Res = false;
            }
        }

        // Input device.
        if (m_pcliInput == NULL)
        {
            CLI_ASSERT(false);
            b_Res = false;
        }
        else if (! m_pcliInput->CloseDown(__CALL_INFO__))
        {
            GetStream(ERROR_STREAM) << m_pcliInput->GetLastError().GetString(m_eLang) << endl;
            b_Res = false;
        }
    }

    // Unset device references.
    if (b_DoCloseDevices)
    {
        // Output.
        for (int i=0; i<STREAM_TYPES_COUNT; i++)
        {
            if (m_artStream[i].pcliStream == m_pcliInput)
            {
                m_artStream[i].pcliStream->FreeInstance(__CALL_INFO__);
                m_artStream[i].pcliStream = NULL;
            }
        }

        // Input.
        ReleaseInput();
    }

    // Unregister the execution context.
    bool b_ContextFound = false;
    for (tk::Queue<ExecutionContext*>::Iterator it = m_tkRunningContexts.GetIterator(); m_tkRunningContexts.IsValid(it); m_tkRunningContexts.MoveNext(it))
    {
        if (const ExecutionContext* const pcli_ExecutionContext = m_tkRunningContexts.GetAt(it))
        {
            if (pcli_ExecutionContext == & CLI_Context)
            {
                m_tkRunningContexts.Remove(it);
                b_ContextFound = true;
                break;
            }
        }
    }
    if (! b_ContextFound)
    {
        b_Res = false;
    }
    GetTraces().Trace(TRACE_EXEC_CTX) << "Execution context manager " << this << ": " << m_tkRunningContexts.GetCount() << " execution contexts currently running, "
                                                                                        << "current: " << GetCurrentContext() << "." << endl;

    return b_Res;
}

void ExecutionContextManager::ReleaseInput(void)
{
    if (m_pcliInput != NULL)
    {
        // Non-blocking device management.
        if (NonBlockingIODevice* const pcli_NonBlockingIODevice = dynamic_cast<NonBlockingIODevice*>(m_pcliInput))
        {
            pcli_NonBlockingIODevice->SetExecutionContextManager(NULL);
        }

        m_pcliInput->FreeInstance(__CALL_INFO__);
        m_pcliInput = NULL;
    }
}

const bool ExecutionContextManager::IsRunning(void) const
{
    return (
        (m_pcliInput != NULL)                   // First check the input device reference is still set.
        && (! m_tkRunningContexts.IsEmpty())    // Then check there are contexts currently running.
    );
}

const bool ExecutionContextManager::IsRunning(const ExecutionContext& CLI_Context) const
{
    // First check the execution context manager is running itself.
    if (IsRunning())
    {
        // Then look for the execution context in the running contexts list.
        for (tk::Queue<ExecutionContext*>::Iterator it = m_tkRunningContexts.GetIterator(); m_tkRunningContexts.IsValid(it); m_tkRunningContexts.MoveNext(it))
        {
            if (const ExecutionContext* const pcli_ExecutionContext = m_tkRunningContexts.GetAt(it))
            {
                if (pcli_ExecutionContext == & CLI_Context)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

ExecutionContext* const ExecutionContextManager::GetCurrentContext(void)
{
    if (! m_tkRunningContexts.IsEmpty())
    {
        if (ExecutionContext* const pcli_ExecutionContext = m_tkRunningContexts.GetHead())
        {
            return pcli_ExecutionContext;
        }
    }

    return NULL;
}

const IODevice& ExecutionContextManager::GetInput(void) const
{
    if (m_pcliInput != NULL)
    {
        return *m_pcliInput;
    }

    return IODevice::GetStdIn();
}

const OutputDevice& ExecutionContextManager::GetStream(const STREAM_TYPE E_StreamType) const
{
    CLI_ASSERT((E_StreamType >= 0) && (E_StreamType < STREAM_TYPES_COUNT));
    if ((E_StreamType >= 0) && (E_StreamType < STREAM_TYPES_COUNT))
    {
        if (m_artStream[E_StreamType].bEnable)
        {
            if (m_artStream[E_StreamType].pcliStream != NULL)
            {
                return *m_artStream[E_StreamType].pcliStream;
            }
        }
    }

    // Default to null device.
    return OutputDevice::GetNullDevice();
}

const bool ExecutionContextManager::SetStream(const STREAM_TYPE E_StreamType, OutputDevice& CLI_Stream)
{
    // ALL_STREAMS management.
    if (E_StreamType == ALL_STREAMS)
    {
        for (int i = 0; i < STREAM_TYPES_COUNT; i++)
        {
            if (! SetStream((STREAM_TYPE) i, CLI_Stream))
            {
                return false;
            }
        }

        return true;
    }

    if ((E_StreamType >= 0) && (E_StreamType < STREAM_TYPES_COUNT))
    {
        // Free previous reference.
        if (OutputDevice* const pcli_Stream = m_artStream[E_StreamType].pcliStream)
        {
            bool b_Res = true;

            // Unreference the device right now.
            m_artStream[E_StreamType].pcliStream = NULL;
            if (IsRunning())
            {
                if (! pcli_Stream->CloseDown(__CALL_INFO__))
                {
                    GetStream(ERROR_STREAM) << pcli_Stream->GetLastError().GetString(GetLang()) << endl;
                    b_Res = false;
                }
            }
            pcli_Stream->FreeInstance(__CALL_INFO__);

            if (! b_Res)
            {
                // Abort on error.
                return false;
            }
        }

        // Store next reference.
        {
            CLI_Stream.UseInstance(__CALL_INFO__);
            if (IsRunning())
            {
                if (! CLI_Stream.OpenUp(__CALL_INFO__))
                {
                    // Store nothing on error.
                    GetStream(ERROR_STREAM) << CLI_Stream.GetLastError().GetString(GetLang()) << endl;
                    CLI_Stream.FreeInstance(__CALL_INFO__);
                    return false;
                }
            }
            // Do not store the reference until opening is done.
            m_artStream[E_StreamType].pcliStream = & CLI_Stream;
        }

        return true;
    }

    CLI_ASSERT(false);
    return false;
}

const bool ExecutionContextManager::StreamEnabled(const STREAM_TYPE E_StreamType) const
{
    if ((E_StreamType >= 0) && (E_StreamType < STREAM_TYPES_COUNT))
    {
        return m_artStream[E_StreamType].bEnable;
    }

    CLI_ASSERT(false);
    return false;
}

const bool ExecutionContextManager::EnableStream(const STREAM_TYPE E_StreamType, const bool B_Enable)
{
    // ALL_STREAMS management.
    if (E_StreamType == ALL_STREAMS)
    {
        for (int i = 0; i < STREAM_TYPES_COUNT; i++)
        {
            if (! EnableStream((STREAM_TYPE) i, B_Enable))
            {
                return false;
            }
        }

        return true;
    }

    if ((E_StreamType >= 0) && (E_StreamType < STREAM_TYPES_COUNT))
    {
        m_artStream[E_StreamType].bEnable = B_Enable;
        return true;
    }

    CLI_ASSERT(false);
    return false;
}

void ExecutionContextManager::SetLang(const ResourceString::LANG E_Lang)
{
    m_eLang = E_Lang;
}

const ResourceString::LANG ExecutionContextManager::GetLang(void) const
{
    return m_eLang;
}

void ExecutionContextManager::SetBeep(const bool B_Enable)
{
    m_bBeep = B_Enable;
}

const bool ExecutionContextManager::GetBeep(void) const
{
    return m_bBeep;
}

void ExecutionContextManager::Beep(void)
{
    if (GetBeep())
    {
        GetStream(ERROR_STREAM).Beep();
    }
}
