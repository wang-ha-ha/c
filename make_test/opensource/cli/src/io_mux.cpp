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

#include <string.h> // memset

#include "cli/io_mux.h"
#include "cli/shell.h"
#include "cli/traces.h"
#include "cli/assert.h"
#include "consistency.h"
#include "constraints.h"

CLI_NS_USE(cli)


//! @brief Shell trace class singleton redirection.
#define TRACE_IOMUX GetIOMuxTraceClass()
//! @brief Shell trace class singleton.
static const TraceClass& GetIOMuxTraceClass(void)
{
    static const TraceClass cli_IOMuxTraceClass("CLI_IOMUX", Help()
        .AddHelp(Help::LANG_EN, "Input/output multiplexer traces")
        .AddHelp(Help::LANG_FR, "Traces de multiplexage d'entrées/sorties"));
    return cli_IOMuxTraceClass;
}


IOMux::IOMux(const bool B_AutoDelete)
  : IODevice("mux", B_AutoDelete),
    m_qDevices(MAX_IO_MUX_INPUTS)
{
    EnsureCommonDevices();
    EnsureTraces();

    GetTraces().Trace(TRACE_IOMUX) << "New " << GetDebugName() << endl;
}

IOMux::~IOMux(void)
{
    GetTraces().Trace(TRACE_IOMUX) << "Deleting " << GetDebugName() << endl;

    ResetDeviceList();
}

const bool IOMux::OpenDevice(void)
{
    GetTraces().SafeTrace(TRACE_IOMUX, *this) << "Opening " << GetDebugName() << endl;

    bool b_Res = true;

    if (IODevice* const pcli_Input = CheckCurrentDevice())
    {
        GetTraces().SafeTrace(TRACE_IOMUX, *this) << GetDebugName() << ": opening input " << pcli_Input->GetDebugName() << endl;
        if (! pcli_Input->OpenUp(__CALL_INFO__))
        {
            m_cliLastError = pcli_Input->GetLastError();
            b_Res = false;
        }
    }
    else
    {
        // Note: m_cliLastError has been set within CheckCurrentDevice()
        b_Res = false;
    }

    if (! b_Res)
    {
        CloseDevice();
    }
    return b_Res;
}

const bool IOMux::CloseDevice(void)
{
    GetTraces().SafeTrace(TRACE_IOMUX, *this) << "Closing " << GetDebugName() << endl;

    bool b_Res = true;

    while (! m_qDevices.IsEmpty())
    {
        if (! ReleaseFirstDevice())
        {
            // Note: m_cliLastError has been set within ReleaseFirstDevice()
            b_Res = false;
        }
    }

    return b_Res;
}

void IOMux::PutString(const char* const STR_Out) const
{
    if (const OutputDevice* const pcli_Output = GetCurrentDevice())
    {
        if (! pcli_Output->WouldOutput(*this)) // Avoid infinite loops.
        {
            pcli_Output->PutString(STR_Out);
        }
    }
}

void IOMux::Beep(void) const
{
    if (const OutputDevice* const pcli_Output = GetCurrentDevice())
    {
        if (! pcli_Output->WouldOutput(*this)) // Avoid infinite loops.
        {
            pcli_Output->Beep();
        }
    }
}

void IOMux::CleanScreen(void) const
{
    if (const OutputDevice* const pcli_Output = GetCurrentDevice())
    {
        if (! pcli_Output->WouldOutput(*this)) // Avoid infinite loops.
        {
            pcli_Output->CleanScreen();
        }
    }
}

const bool IOMux::WouldOutput(const OutputDevice& CLI_Device) const
{
    // Check self instance first.
    if (IODevice::WouldOutput(CLI_Device))
    {
        return true;
    }

    // Then check output stream.
    if (const OutputDevice* const pcli_Output = GetCurrentDevice())
    {
        if (pcli_Output->WouldOutput(CLI_Device))
        {
            return true;
        }
    }

    return false;
}

const KEY IOMux::GetKey(void) const
{
    for (   const IODevice* pcli_Input = CheckCurrentDevice();
            pcli_Input != NULL;
            pcli_Input = const_cast<IOMux*>(this)->SwitchNextDevice())
    {
        KEY e_Key = NULL_KEY;
        if (! pcli_Input->WouldInput(*this)) // Protection against infinite loop.
        {
            e_Key = pcli_Input->GetKey();
        }

        if (e_Key != NULL_KEY)
        {
            return e_Key;
        }
    }

    if (m_cliLastError.IsEmpty())
    {
        m_cliLastError
            .SetString(ResourceString::LANG_EN, "IOMux: No valid input device")
            .SetString(ResourceString::LANG_FR, "IOMux: Entrée de caractère invalide");
    }
    return cli::NULL_KEY;
}

const ResourceString IOMux::GetLocation(void) const
{
    if (const IODevice* const pcli_Input = GetCurrentDevice())
    {
        if (! pcli_Input->WouldInput(*this)) // Avoid infinite loops.
        {
            return pcli_Input->GetLocation();
        }
    }

    return ResourceString();
}

const bool IOMux::WouldInput(const IODevice& CLI_Device) const
{
    // Check self instance first.
    if (IODevice::WouldInput(CLI_Device))
    {
        return true;
    }

    // Then check current input stream.
    if (const IODevice* const pcli_Input = GetCurrentDevice())
    {
        if (pcli_Input->WouldInput(CLI_Device))
        {
            return true;
        }
    }
    // Other input streams do not need to be checked yet.:
    // only GetKey() does switch to next input device automatically, and GetKey() does check WouldInput() before calling GetKey() on the next device.

    return false;
}

const bool IOMux::AddDevice(IODevice* const PCLI_Device)
{
    if (PCLI_Device != NULL)
    {
        GetTraces().SafeTrace(TRACE_IOMUX, *this) << GetDebugName() << ": adding " << PCLI_Device->GetDebugName() << endl;

        if (m_qDevices.AddTail(PCLI_Device))
        {
            PCLI_Device->UseInstance(__CALL_INFO__);
            return true;
        }
        else
        {
            m_cliLastError
                .SetString(ResourceString::LANG_EN, "IOMux: Cannot add device to the device list")
                .SetString(ResourceString::LANG_FR, "IOMux: Impossible d'ajouter un périphérique d'entrée / sortie à la liste");
        }
    }
    else
    {
        m_cliLastError
            .SetString(ResourceString::LANG_EN, "IOMux: Cannot add a NULL input device")
            .SetString(ResourceString::LANG_FR, "IOMux: Impossible d'ajouter un périphérique d'entrée / sortie invalide");
    }

    return false;
}

const IODevice* const IOMux::GetCurrentDevice(void) const
{
    if (! m_qDevices.IsEmpty())
    {
        return m_qDevices.GetHead();
    }

    return NULL;
}

const IODevice* const IOMux::SwitchNextDevice(void)
{
    // Terminate head device.
    if (! m_qDevices.IsEmpty())
    {
        if (! ReleaseFirstDevice())
        {
            // Note: m_cliLastError has been set within ReleaseFirstInputDevice()
            return NULL;
        }
    }

    // Prepare next device.
    if (IODevice* const pcli_CurrentDevice = CheckCurrentDevice())
    {
        GetTraces().SafeTrace(TRACE_IOMUX, *this) << GetDebugName() << ": moving to next input " << pcli_CurrentDevice->GetDebugName() << endl;
        if (GetOpenUsers() > 0)
        {
            if (! pcli_CurrentDevice->OpenUp(__CALL_INFO__))
            {
                // Open failure.
                m_cliLastError = pcli_CurrentDevice->GetLastError();
                return NULL;
            }
        }

        // Successful return.
        return pcli_CurrentDevice;
    }
    else
    {
        GetTraces().SafeTrace(TRACE_IOMUX, *this) << GetDebugName() << ": no more input" << endl;
        // Note: m_cliLastError has been set within CheckInput()
        if (m_cliLastError.IsEmpty())
        {
            m_cliLastError
                .SetString(ResourceString::LANG_EN, "IOMux: No input / output device")
                .SetString(ResourceString::LANG_FR, "IOMux: Pas de périphérique d'entrée / sortie");
        }
    }

    // Default return indicates a failure.
    return NULL;
}

const bool IOMux::ResetDeviceList(void)
{
    bool b_Res = true;

    while (! m_qDevices.IsEmpty())
    {
        if (! ReleaseFirstDevice())
        {
            // Note: m_cliLastError has been set within ReleaseFirstInputDevice()
            b_Res = false;
        }
    }

    return b_Res;
}

IODevice* const IOMux::CreateDevice(void)
{
    return NULL;
}

IODevice* const IOMux::CheckCurrentDevice(void) const
{
    // m_qDevices is not empty => return the first valid device.
    while (! m_qDevices.IsEmpty())
    {
        if (IODevice* const pcli_CurrentDevice = m_qDevices.GetHead())
        {
            return pcli_CurrentDevice;
        }
        else
        {
            m_qDevices.RemoveHead();
        }
    }

    // Input needed.
    if (IODevice* const pcli_NewDevice = const_cast<IOMux*>(this)->CreateDevice())
    {
        if (! const_cast<IOMux*>(this)->AddDevice(pcli_NewDevice))
        {
            // Note: m_cliLastError has been set within AddInput().
            return NULL;
        }

        if (GetOpenUsers() > 0)
        {
            // Input should be opened.
            if (! pcli_NewDevice->OpenUp(__CALL_INFO__))
            {
                // Open failure.
                m_cliLastError = pcli_NewDevice->GetLastError();
                return NULL;
            }
        }
    }

    if (! m_qDevices.IsEmpty())
    {
        if (IODevice* const pcli_CurrentDevice = m_qDevices.GetHead())
        {
            // Successful return.
            return pcli_CurrentDevice;
        }
    }

    // No more input.
    m_cliLastError
        .SetString(ResourceString::LANG_EN, "IOMux: No more input")
        .SetString(ResourceString::LANG_FR, "IOMux: Aucun périphérique d'entrée");
    return NULL;
}

const bool IOMux::ReleaseFirstDevice(void)
{
    if (! m_qDevices.IsEmpty())
    {
        bool b_Res = true;

        if (IODevice* const pcli_FirstDevice = m_qDevices.RemoveHead())
        {
            if (GetOpenUsers() > 0)
            {
                // Close the device if needed.
                if (! pcli_FirstDevice->CloseDown(__CALL_INFO__))
                {
                    m_cliLastError = pcli_FirstDevice->GetLastError();
                    b_Res = false;
                }
            }
            // Free the device instance.
            pcli_FirstDevice->FreeInstance(__CALL_INFO__);
        }

        return b_Res;
    }
    else
    {
        m_cliLastError
            .SetString(ResourceString::LANG_EN, "IOMux: No more input / output")
            .SetString(ResourceString::LANG_FR, "IOMux: Aucun périphérique d'entrée / sortie");
        return false;
    }
}
