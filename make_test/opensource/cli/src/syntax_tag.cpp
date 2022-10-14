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

#include <stdio.h>
#include <string.h>

#include "cli/syntax_tag.h"
#include "cli/help.h"
#include "cli/traces.h"
#include "cli/io_device.h"
#include "cli/assert.h"
#include "constraints.h"
#include "utils.h"

CLI_NS_USE(cli)


SyntaxTag::SyntaxTag(const bool B_Hollow)
  : SyntaxNode("", Help()),
    m_bHollow(B_Hollow)
{
}

SyntaxTag::~SyntaxTag(void)
{
}

const tk::String SyntaxTag::GetKeyword(void) const
{
    static char str_Buffer[256];
    const int i_Res = snprintf(str_Buffer, sizeof(str_Buffer), "tag[%p]", this);
    CheckSnprintfResult(str_Buffer, sizeof(str_Buffer), i_Res);

    tk::String str_Keyword(MAX_WORD_LENGTH);
    if (! str_Keyword.Set(str_Buffer))
    {
        GetTraces().Trace(INTERNAL_ERROR) << "SyntaxTag::GetKeyword() could not construct the SyntaxTag keyword from string '" << str_Buffer << "'" << endl;
    }
    return str_Keyword;
}

const bool SyntaxTag::GetbHollow(void) const
{
    return m_bHollow;
}


SyntaxRef::SyntaxRef(const SyntaxTag& CLI_Tag)
  : Element("", Help()),
    m_pcliTag(& CLI_Tag)
{
}

SyntaxRef::~SyntaxRef(void)
{
}

const tk::String SyntaxRef::GetKeyword(void) const
{
    static char str_Buffer[256];
    const int i_Res = snprintf(str_Buffer, sizeof(str_Buffer), "ref[%p]", this);
    CheckSnprintfResult(str_Buffer, sizeof(str_Buffer), i_Res);

    tk::String str_Keyword(MAX_WORD_LENGTH);
    if (! str_Keyword.Set(str_Buffer))
    {
        GetTraces().Trace(INTERNAL_ERROR) << "SyntaxRef::GetKeyword() could not construct the SyntaxRef keyword from string '" << str_Buffer << "'" << endl;
    }
    if (m_pcliTag != NULL)
    {
        if ((! str_Keyword.Append(" -> "))
            || (! str_Keyword.Append(m_pcliTag->GetKeyword())))
        {
            GetTraces().Trace(INTERNAL_ERROR) << "SyntaxRef::GetKeyword() could not construct the SyntaxRef keyword from strings ' -> ' or m_pcliTag" << endl;
        }
    }
    return str_Keyword;
}

const SyntaxTag& SyntaxRef::GetTag(void) const
{
    CLI_ASSERT(m_pcliTag != NULL);
    return *m_pcliTag;
}

