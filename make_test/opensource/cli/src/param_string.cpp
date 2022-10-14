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

#include "cli/param_string.h"
#include "cli/traces.h"
#include "cli/io_device.h"
#include "constraints.h"

CLI_NS_USE(cli)


ParamString::ParamString(const Help& CLI_Help)
  : Param("<string>", CLI_Help),
    m_strValue(MAX_WORD_LENGTH)
{
}

ParamString::ParamString(const char* const STR_Keyword, const Help& CLI_Help)
  : Param(STR_Keyword, CLI_Help),
    m_strValue(MAX_WORD_LENGTH)
{
}

ParamString::~ParamString(void)
{
}

ParamString::operator const char* const(void) const
{
    return m_strValue;
}

const bool ParamString::SetstrValue(const char* const STR_Value) const
{
    bool b_Result = false;
    if (Param::SetValue(STR_Value))
    {
        if (m_strValue.Set(STR_Value))
        {
            b_Result = true;
        }
        else
        {
            GetTraces().Trace(INTERNAL_ERROR) << "Not enough space for string parameter" << endl;
        }
    }
    return b_Result;
}

const Param& ParamString::CopyValue(const Param& CLI_Param) const
{
    SetstrValue(CLI_Param.GetKeyword());
    return *this;
}

const Param* const ParamString::Clone(void) const
{
    return InitClone(* new ParamString(GetHelp()));
}

