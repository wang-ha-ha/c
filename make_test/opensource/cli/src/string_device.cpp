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

#include "cli/string_device.h"

#include "constraints.h"


CLI_NS_USE(cli)


StringDevice::StringDevice(const unsigned int UI_OutputMaxLen, const bool B_AutoDelete)
  : OutputDevice(tk::String(MAX_DEVICE_NAME_LENGTH, "string"), B_AutoDelete),
    m_tkString(UI_OutputMaxLen)
{
}

StringDevice::~StringDevice(void)
{
}

const bool StringDevice::OpenDevice(void)
{
    // Nothing to do.
    return true;
}

const bool StringDevice::CloseDevice(void)
{
    // Nothing to do.
    return true;
}

void StringDevice::PutString(const char* const STR_Out) const
{
    m_tkString.Append(STR_Out);
}

void StringDevice::Beep(void) const
{
    // Nothing to do.
}

void StringDevice::CleanScreen(void) const
{
    m_tkString.Reset();
}

const tk::String& StringDevice::GetString(void) const
{
    return m_tkString;
}

StringDevice& StringDevice::Reset(void)
{
    m_tkString.Reset();

    return *this;
}
