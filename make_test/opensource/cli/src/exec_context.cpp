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

#include "cli/exec_context.h"
#include "cli/non_blocking_io_device.h"
#include "cli/traces.h"

#include "exec_context_manager.h"
#include "constraints.h"

CLI_NS_USE(cli)


// ExecutionContext implementation.

ExecutionContext::ExecutionContext(void)
  : m_cliManager(* new ExecutionContextManager()), // Auto-deleted on FreeInstance().
    m_tkResults(MAX_EXECUTION_CONTEXTS)
{
    GetTraces().Trace(TRACE_EXEC_CTX) << "Execution context " << this << ": New top execution context." << endl;
    m_cliManager.UseInstance(*this);
}

ExecutionContext::ExecutionContext(ExecutionContext& CLI_ParentContext)
  : m_cliManager(CLI_ParentContext.m_cliManager),
    m_tkResults(MAX_EXECUTION_CONTEXTS)
{
    GetTraces().Trace(TRACE_EXEC_CTX) << "Execution context " << this << ": New child execution context." << endl;
    m_cliManager.UseInstance(*this);
}

ExecutionContext::~ExecutionContext(void)
{
    GetTraces().Trace(TRACE_EXEC_CTX) << "Execution context " << this << ": End of execution context." << endl;
    m_cliManager.FreeInstance(*this);
}

ExecutionContextManager& ExecutionContext::GetContextManager(void)
{
    return m_cliManager;
}

const IODevice& ExecutionContext::GetInput(void) const
{
    return m_cliManager.GetInput();
}

const OutputDevice& ExecutionContext::GetStream(const STREAM_TYPE E_StreamType) const
{
    return m_cliManager.GetStream(E_StreamType);
}

const bool ExecutionContext::SetStream(const STREAM_TYPE E_StreamType, OutputDevice& CLI_Stream)
{
    return m_cliManager.SetStream(E_StreamType, CLI_Stream);
}

const bool ExecutionContext::StreamEnabled(const STREAM_TYPE E_StreamType) const
{
    return m_cliManager.StreamEnabled(E_StreamType);
}

const bool ExecutionContext::EnableStream(const STREAM_TYPE E_StreamType, const bool B_Enable)
{
    return m_cliManager.EnableStream(E_StreamType, B_Enable);
}

void ExecutionContext::SetLang(const ResourceString::LANG E_Lang)
{
    m_cliManager.SetLang(E_Lang);
}

const ResourceString::LANG ExecutionContext::GetLang(void) const
{
    return m_cliManager.GetLang();
}

void ExecutionContext::SetBeep(const bool B_Enable)
{
    m_cliManager.SetBeep(B_Enable);
}

const bool ExecutionContext::GetBeep(void) const
{
    return m_cliManager.GetBeep();
}

void ExecutionContext::Beep(void)
{
    m_cliManager.Beep();
}

void ExecutionContext::Run(IODevice& CLI_IODevice)
{
    if (BeginExecution(CLI_IODevice))
    {
        if (dynamic_cast<NonBlockingIODevice*>(& CLI_IODevice) == NULL)
        {
            // Call the main blocking loop.
            MainBlockingLoop();
            // Once the main loop is done, terminate the thing.
            FinishExecution();
        }
    }
    else
    {
        FinishExecution();
    }
}

void ExecutionContext::Run(void)
{
    Run(const_cast<IODevice&>(GetInput()));
}

const bool ExecutionContext::BeginExecution(IODevice& CLI_IODevice)
{
    GetTraces().Trace(TRACE_EXEC_CTX) << "Execution context " << this << ": Starting execution." << endl;

    if (m_cliManager.OpenUp(*this, CLI_IODevice))
    {
        // Let the actual execution context make its own start ups.
        if (OnStartExecution())
        {
            return true;
        }
    }

    return false;
}

void ExecutionContext::MainBlockingLoop(void)
{
    // While there are menus in the menu stack, it means we are still waiting for command lines.
    while (IsRunning())
    {
        // Get an input key.
        const KEY e_Key = m_cliManager.GetInput().GetKey();
        if (e_Key != NULL_KEY)
        {
            // Process the input key.
            ProcessKey(e_Key);
        }
        else
        {
            // End of input.
            FinishExecution();
        }
    }
}

void ExecutionContext::ProcessKey(const KEY E_KeyCode)
{
    const OutputDevice& cli_Trace = GetTraces().Trace(TRACE_EXEC_CTX) << "Execution context " << this << ": ";
    switch (E_KeyCode)
    {
    case ENTER: cli_Trace << "Key " << E_KeyCode << "/\\n"; break;
    default:    cli_Trace << "Key " << E_KeyCode << "/" << (char) E_KeyCode; break;
    }
    cli_Trace << " received." << endl;

    if (IsRunning())
    {
        if (E_KeyCode != NULL_KEY)
        {
            // Process the input key.
            OnKey(E_KeyCode);
        }
        else
        {
            // End of input.
            FinishExecution();
        }
    }
}

const bool ExecutionContext::FinishExecution(void)
{
    GetTraces().Trace(TRACE_EXEC_CTX) << "Execution context " << this << ": Ending execution." << endl;

    bool b_Res = true;

    // Let the actual execution context make its own liberations.
    if (! OnStopExecution())
    {
        b_Res = false;
    }

    // Do not terminate twice.
    if (IsRunning())
    {
        if (! m_cliManager.CloseDown(*this))
        {
            b_Res = false;
        }

        // Call registered result interfaces at the very end.
        tk::Queue<ExecutionResult*> tk_Results(m_tkResults.GetCount());
        while (! m_tkResults.IsEmpty())
        {
            tk_Results.AddTail(m_tkResults.RemoveHead());
        }
        while (! tk_Results.IsEmpty())
        {
            if (ExecutionResult* const pcli_ExecutionResult = tk_Results.RemoveTail())
            {
                GetTraces().Trace(TRACE_EXEC_CTX) << "Execution context " << this << ": Notifying result to interface " << pcli_ExecutionResult << "." << endl;
                pcli_ExecutionResult->OnResult(*this);
            }
        }
    }

    return b_Res;
}

const bool ExecutionContext::IsRunning(void) const
{
    return m_cliManager.IsRunning(*this);
}

void ExecutionContext::StopExecution(void)
{
    FinishExecution();
}

void ExecutionContext::StopAllExecutions(void)
{
    m_cliManager.StopAllExecutions();
}


// ExecutionResult implementation.

ExecutionResult::ExecutionResult(void)
{
}

ExecutionResult::~ExecutionResult(void)
{
}

void ExecutionResult::WatchResult(ExecutionContext& CLI_Context)
{
    GetTraces().Trace(TRACE_EXEC_CTX) << "Execution context " << & CLI_Context << ": Registering result interface " << this << "." << endl;
    if (! CLI_Context.m_tkResults.AddTail(this))
    {
        GetTraces().Trace(TRACE_EXEC_CTX) << "Execution context " << & CLI_Context << ": Could not register result interface " << this << "." << endl;
    }
}
