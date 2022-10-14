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
#include "cli/assert.h"

#include "exec_context_manager.h"
#include "constraints.h"


CLI_NS_USE(cli)


NonBlockingIODevice::NonBlockingIODevice(const char* const STR_DbgName, const bool B_AutoDelete)
  : IODevice(STR_DbgName, B_AutoDelete),
    m_pcliExecutionContextManager(NULL)
{
}

NonBlockingIODevice::~NonBlockingIODevice(void)
{
}

const KEY NonBlockingIODevice::GetKey(void) const
{
    // As this device is non-blocking, this method should not be called.
    CLI_ASSERT(false);
    return NULL_KEY;
}

void NonBlockingIODevice::SetExecutionContextManager(ExecutionContextManager* const PCLI_ExecutionContextManager)
{
    m_pcliExecutionContextManager = PCLI_ExecutionContextManager;
}

const ExecutionContext* const NonBlockingIODevice::GetExecutionContext(void) const
{
    if (m_pcliExecutionContextManager != NULL)
    {
        if (ExecutionContext* const pcli_ExecutionContext = m_pcliExecutionContextManager->GetCurrentContext())
        {
            return pcli_ExecutionContext;
        }
    }

    return NULL;
}

void NonBlockingIODevice::OnKey(const KEY E_Key) const
{
    if (m_pcliExecutionContextManager != NULL)
    {
        if (ExecutionContext* const pcli_ExecutionContext = m_pcliExecutionContextManager->GetCurrentContext())
        {
            pcli_ExecutionContext->ProcessKey(E_Key);
        }
    }
}
