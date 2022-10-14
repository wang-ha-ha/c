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

#include <stdlib.h>

#include "cli/param.h"
#include "cli/traces.h"
#include "cli/io_device.h"
#include "constraints.h"

CLI_NS_USE(cli)


Param::Param(const char* const STR_Keyword, const Help& CLI_Help)
  : SyntaxNode(STR_Keyword, CLI_Help),
    m_strValue(MAX_WORD_LENGTH),
    m_pcliCloned(NULL)
{
}

Param::~Param(void)
{
}

const tk::String Param::GetKeyword(void) const
{
    if (! m_strValue.IsEmpty())
    {
        return GetstrValue();
    }
    else
    {
        return SyntaxNode::GetKeyword();
    }
}

const bool Param::FindElements(
        Element::List& CLI_ExactList,
        Element::List& CLI_NearList,
        const char* const STR_Keyword
        ) const
{
    if (GetCloned() != NULL)
    {
        return GetCloned()->FindElements(CLI_ExactList, CLI_NearList, STR_Keyword);
    }
    else
    {
        return SyntaxNode::FindElements(CLI_ExactList, CLI_NearList, STR_Keyword);
    }
}

const tk::String Param::GetstrValue(void) const
{
    return m_strValue;
}

const Param* const Param::GetCloned(void) const
{
    return m_pcliCloned;
}

const bool Param::SetValue(const char* const STR_Value) const
{
    if (m_strValue.Set(STR_Value))
    {
        return true;
    }
    else
    {
        GetTraces().Trace(INTERNAL_ERROR) << "Could not store original string parameter value." << endl;
        return false;
    }
}

const Param* const Param::InitClone(Param& CLI_CloneParam) const
{
    CLI_CloneParam.SetCli(const_cast<Cli&>(GetCli()));
    CLI_CloneParam.SetstrValue(GetstrValue());
    CLI_CloneParam.SetCloned(*this);
    return & CLI_CloneParam;
}

const bool Param::SetCloned(const Param& CLI_Cloned)
{
    m_pcliCloned = & CLI_Cloned;
    return true;
}
