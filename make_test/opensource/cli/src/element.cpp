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

#include "cli/cli.h"
#include "cli/shell.h"
#include "cli/element.h"
#include "cli/assert.h"
#include "constraints.h"

CLI_NS_USE(cli)


Element::Element(const char* const STR_Keyword, const Help& CLI_Help)
  : m_strKeyword(MAX_WORD_LENGTH, STR_Keyword),
    m_cliHelp(CLI_Help),
    m_pcliCli(NULL)
{
}

Element::~Element(void)
{
}

const tk::String Element::GetKeyword(void) const
{
    return m_strKeyword;
}

const Help& Element::GetHelp(void) const
{
    return m_cliHelp;
}

const bool Element::FindElements(
        Element::List& CLI_ExactList,
        Element::List& CLI_NearList,
        const char* const STR_Keyword
        ) const
{
    // Should be overridden.
    CLI_ASSERT(false);
    return false;
}

void Element::SetCli(Cli& CLI_Cli)
{
    m_pcliCli = & CLI_Cli;
}

Cli& Element::GetCli(void)
{
    CLI_ASSERT(m_pcliCli != NULL);
    return *m_pcliCli;
}

const Cli& Element::GetCli(void) const
{
    CLI_ASSERT(m_pcliCli != NULL);
    return *m_pcliCli;
}

Shell& Element::GetShell(void) const
{
    return GetCli().GetShell();
}

const OutputDevice& Element::GetOutputStream(void) const
{
    return GetShell().GetStream(OUTPUT_STREAM);
}

const OutputDevice& Element::GetErrorStream(void) const
{
    return GetShell().GetStream(ERROR_STREAM);
}

