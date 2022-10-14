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

#include "cli/endl.h"
#include "cli/cli.h"
#include "cli/menu.h"

CLI_NS_USE(cli)


Endl::Endl(const Help& CLI_Help)
  : Element("\n", CLI_Help),
    m_pcliMenuRef(NULL)
{
}

Endl::~Endl(void)
{
    if (m_pcliMenuRef != NULL)
    {
        delete m_pcliMenuRef;
        m_pcliMenuRef = NULL;
    }
}

Menu& Endl::SetMenu(Menu* const PCLI_Menu)
{
    if (m_pcliMenuRef != NULL)
    {
        delete m_pcliMenuRef;
        m_pcliMenuRef = NULL;
    }

    GetCli().AddMenu(PCLI_Menu);
    PCLI_Menu->SetCli(GetCli());
    SetMenuRef(new MenuRef(*PCLI_Menu));
    return *PCLI_Menu;
}

MenuRef& Endl::SetMenuRef(MenuRef* const PCLI_MenuRef)
{
    if (m_pcliMenuRef != NULL)
    {
        delete m_pcliMenuRef;
        m_pcliMenuRef = NULL;
    }

    PCLI_MenuRef->SetCli(GetCli());
    m_pcliMenuRef = PCLI_MenuRef;
    return *PCLI_MenuRef;
}

const MenuRef* const Endl::GetMenuRef(void) const
{
    return m_pcliMenuRef;
}


