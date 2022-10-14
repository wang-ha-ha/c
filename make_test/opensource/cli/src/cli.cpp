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

#ifndef CLI_NO_REGEX
    #include <regex.h>
    #include <string.h>
#endif

#include "cli/cli.h"
#include "cli/menu.h"
#include "cli/shell.h"
#include "cli/keyword.h"
#include "cli/endl.h"
#include "cli/command_line.h"
#include "cli/traces.h"
#include "cli/assert.h"
#include "config_menu.h"
#include "traces_menu.h"
#include "consistency.h"
#include "constraints.h"

CLI_NS_USE(cli)


//! @brief Cli trace class singleton redirection.
#define TRACE_CLI GetCliTraceClass()
//! @brief Cli trace class singleton.
static const TraceClass& GetCliTraceClass(void)
{
    static const TraceClass cli_CliTraceClass("CLI", Help()
        .AddHelp(Help::LANG_EN, "CLI traces")
        .AddHelp(Help::LANG_FR, "Traces de CLI"));
    return cli_CliTraceClass;
}


//! @brief Cli registry singleton.
static Cli::List& GetCliRegistry(void)
{
    static Cli::List cli_CliRegistry(MAX_CLI_REGISTRY_COUNT);
    return cli_CliRegistry;
}


Cli::Cli(const char* const STR_Name, const Help& CLI_Help)
  : Menu(STR_Name, CLI_Help),
    m_pcliShell(NULL),
    m_qMenus(MAX_MENU_PER_CLI),
    m_qCommentLinePatterns(MAX_COMMENT_PATTERNS_PER_CLI),
    m_pcliConfigMenu(NULL), m_pcliConfigMenuNode(NULL)
    #ifdef _DEBUG
    , m_pcliTracesMenu(NULL), m_pcliTracesMenuNode(NULL)
    #endif
{
    EnsureCommonDevices();
    EnsureTraces();

    // Set the CLI reference to oneself.
    Cli::SetCli(*this); // Do not rely on the virtual call.

    // Register this new CLI in the global CLI list.
    if (GetCliRegistry().AddTail(this))
    {
        GetTraces().Trace(TRACE_CLI) << "New CLI '" << GetName() << "'." << cli::endl;
    }
    else
    {
        GetTraces().Trace(TRACE_CLI) << "Error: New CLI '" << GetName() << "' could not be registered." << cli::endl;
    }
}

Cli::~Cli(void)
{
    GetTraces().Trace(TRACE_CLI) << "Deletion of CLI '" << GetName() << "'." << cli::endl;

    // Retrieve inner information before further deletions.
    // !!!  This method call is a little bit touchy since it is operated while the object is partially destructed at this point.
    //      However, IsConfigMenuEnabled() calls SyntaxNode features which part of this object has not been deleted yet.
    const bool b_ConfigMenuEnabled = IsConfigMenuEnabled();

    // Remove this CLI from the global CLI list.
    for (   Cli::List::Iterator it = GetCliRegistry().GetIterator();
            GetCliRegistry().IsValid(it);
            GetCliRegistry().MoveNext(it))
    {
        if (GetCliRegistry().GetAt(it) == this)
        {
            GetCliRegistry().Remove(it);
            break;
        }
    }

    // Destroy menus.
    while (! m_qMenus.IsEmpty())
    {
        if (const Menu* const pcli_Menu = m_qMenus.RemoveTail())
        {
            delete pcli_Menu;
        }
    }

    if (! b_ConfigMenuEnabled)
    {
        // When the config menu is disabled, m_pcliConfigMenuNode is not registered in SyntaxNode elements,
        // thus it not freed by the SyntaxNode destructor.
        delete m_pcliConfigMenuNode; // [contrib: Oleg Smolsky, 2010, based on CLI 2.5]
    }
}

const int Cli::FindFromName(Cli::List& CLI_CliList, const char* const STR_RegExp)
{
#ifndef CLI_NO_REGEX
    // Compile the regular expression.
    regex_t s_RegExp;
    memset(& s_RegExp, '\0', sizeof(regex_t));
    if (regcomp(& s_RegExp, STR_RegExp, 0) != 0)
    {
        return -1;
    }
#endif

    // Check the name for each CLI instance.
    const int i_InitialCount = CLI_CliList.GetCount();
    for (   Cli::List::Iterator it = GetCliRegistry().GetIterator();
            GetCliRegistry().IsValid(it);
            GetCliRegistry().MoveNext(it))
    {
        if (const Cli* const pcli_Cli = GetCliRegistry().GetAt(it))
        {
#ifndef CLI_NO_REGEX
            if (regexec(& s_RegExp, pcli_Cli->GetName(), 0, NULL, 0) == 0)
#endif
            {
                if (! CLI_CliList.AddTail(pcli_Cli))
                {
                    CLI_ASSERT(false);
                }
            }
        }
    }

#ifndef CLI_NO_REGEX
    // Free.
    regfree(& s_RegExp);
#endif

    return ((int) CLI_CliList.GetCount() - i_InitialCount);
}

Menu& Cli::AddMenu(Menu* const PCLI_Menu)
{
    if (PCLI_Menu != NULL)
    {
        PCLI_Menu->SetCli(*this);
        if (! m_qMenus.AddTail(PCLI_Menu))
        {
            GetTraces().Trace(INTERNAL_ERROR)
                << "Could not add '" << PCLI_Menu->GetKeyword() << "' "
                << "to CLI '" << GetName() << "'." << cli::endl;
        }
    }
    CLI_ASSERT(PCLI_Menu != NULL);
    return *PCLI_Menu;
}

const Menu* const Cli::GetMenu(const char* const STR_MenuName) const
{
    // First of all, look in the menu list
    for (   tk::Queue<const Menu*>::Iterator it = m_qMenus.GetIterator();
            m_qMenus.IsValid(it);
            m_qMenus.MoveNext(it))
    {
        if (const Menu* const pcli_Menu = m_qMenus.GetAt(it))
        {
            if (pcli_Menu->GetName() == STR_MenuName)
            {
                return pcli_Menu;
            }
        }
    }

    // Possibly look for the CLI itself.
    if (GetName() == STR_MenuName)
    {
        return this;
    }

    return NULL;
}

const bool Cli::AddCommentLinePattern(const char* const STR_Start)
{
    for (   tk::Queue<tk::String>::Iterator it = m_qCommentLinePatterns.GetIterator();
            m_qCommentLinePatterns.IsValid(it);
            m_qCommentLinePatterns.MoveNext(it))
    {
        if (m_qCommentLinePatterns.GetAt(it) == STR_Start)
        {
            return false;
        }
    }
    return m_qCommentLinePatterns.AddTail(tk::String(MAX_WORD_LENGTH, STR_Start));
}

const bool Cli::RemoveCommentLinePattern(const char* const STR_Start)
{
    for (   tk::Queue<tk::String>::Iterator it = m_qCommentLinePatterns.GetIterator();
            m_qCommentLinePatterns.IsValid(it);
            m_qCommentLinePatterns.MoveNext(it))
    {
        if (m_qCommentLinePatterns.GetAt(it) == STR_Start)
        {
            return m_qCommentLinePatterns.Remove(it);
        }
    }
    return false;
}

const tk::Queue<tk::String>& Cli::GetCommentLinePatterns(void) const
{
    return m_qCommentLinePatterns;
}

void Cli::SetShell(Shell& CLI_Shell) const
{
    m_pcliShell = & CLI_Shell;
}

Shell& Cli::GetShell(void) const
{
    if (m_pcliShell != NULL)
    {
        return *m_pcliShell;
    }
    else
    {
        static Shell cli_Shell(*this);
        return cli_Shell;
    }
}

void Cli::SetCli(Cli& CLI_Cli)
{
    Menu::SetCli(CLI_Cli);

    if (m_pcliConfigMenu == NULL)
    {
        m_pcliConfigMenu = dynamic_cast<ConfigMenu*>(& AddMenu(new ConfigMenu()));
        Help cli_Help(Help()
            .AddHelp(Help::LANG_EN, "CLI configuration menu")
            .AddHelp(Help::LANG_FR, "Menu de configuration du CLI"));
        m_pcliConfigMenuNode = dynamic_cast<Keyword*>(& AddElement(new Keyword("cli-config", cli_Help)));
        Endl* const pcli_Endl = dynamic_cast<Endl*>(& m_pcliConfigMenuNode->AddElement(new Endl(cli_Help)));
        pcli_Endl->SetMenuRef(new MenuRef(*m_pcliConfigMenu));
    }
    #ifdef _DEBUG
    if (m_pcliTracesMenu == NULL)
    {
        m_pcliTracesMenu = dynamic_cast<TracesMenu*>(& AddMenu(new TracesMenu()));
        Help cli_Help(Help()
            .AddHelp(Help::LANG_EN, "Traces menu")
            .AddHelp(Help::LANG_FR, "Menu de configuration de traces"));
        m_pcliTracesMenuNode = dynamic_cast<Keyword*>(& AddElement(new Keyword("traces", cli_Help)));
        Endl* const pcli_Endl = dynamic_cast<Endl*>(& m_pcliTracesMenuNode->AddElement(new Endl(cli_Help)));
        pcli_Endl->SetMenuRef(new MenuRef(*m_pcliTracesMenu));
    }
    #endif
}

const bool Cli::ExecuteReserved(const CommandLine& CLI_CommandLine) const
{
    CommandLineIterator it(CLI_CommandLine);

    if (! it.StepIt()) { return false; }
    else if (it == GetConfigMenuNode())
    {
        if (! it.StepIt()) { return false; }
        if (dynamic_cast<const Endl*>(*it))
        {
            // Do nothing but return true so that an execution error is not detected.
            return true;
        }
    }
    #ifdef _DEBUG
    else if (it == GetTracesMenuNode())
    {
        if (! it.StepIt()) { return false; }
        if (dynamic_cast<const Endl*>(*it))
        {
            // Do nothing but return true so that an execution error is not detected.
            return true;
        }
    }
    #endif

    return Menu::ExecuteReserved(CLI_CommandLine);
}

const bool Cli::OnError(const ResourceString& CLI_Location, const ResourceString& CLI_ErrorMessage) const
{
    // Default return is true, in order to have the shell display the error.
    return true;
}

void Cli::OnExit(void) const
{
}

ConfigMenu& Cli::GetConfigMenu(void)
{
    CLI_ASSERT(m_pcliConfigMenu != NULL);
    return *m_pcliConfigMenu;
}

const ConfigMenu& Cli::GetConfigMenu(void) const
{
    CLI_ASSERT(m_pcliConfigMenu != NULL);
    return *m_pcliConfigMenu;
}

const Keyword& Cli::GetConfigMenuNode(void) const
{
    CLI_ASSERT(m_pcliConfigMenuNode != NULL);
    return *m_pcliConfigMenuNode;
}

const bool Cli::IsConfigMenuEnabled(void) const
{
    CLI_ASSERT(m_pcliConfigMenuNode != NULL);

    Element::List cli_ExactList(10), cli_NearList(10);
    if (FindElements(cli_ExactList, cli_NearList, m_pcliConfigMenuNode->GetKeyword()))
    {
        if (cli_ExactList.GetCount() == 1)
        {
            if (cli_ExactList.GetHead() == m_pcliConfigMenuNode)
            {
                return true;
            }
        }
    }

    return false;
}

const bool Cli::EnableConfigMenu(const bool B_Enable)
{
    bool b_Res;
    CLI_ASSERT(m_pcliConfigMenuNode != NULL);
    if (B_Enable)
    {
        AddElement(m_pcliConfigMenuNode);
        b_Res = true;
    }
    else
    {
        b_Res = RemoveElement(m_pcliConfigMenuNode, false);
    }
    return b_Res;
}

#ifdef _DEBUG
TracesMenu& Cli::GetTracesMenu(void)
{
    CLI_ASSERT(m_pcliTracesMenu != NULL);
    return *m_pcliTracesMenu;
}

const TracesMenu& Cli::GetTracesMenu(void) const
{
    CLI_ASSERT(m_pcliTracesMenu != NULL);
    return *m_pcliTracesMenu;
}

const Keyword& Cli::GetTracesMenuNode(void) const
{
    CLI_ASSERT(m_pcliTracesMenuNode != NULL);
    return *m_pcliTracesMenuNode;
}
#endif
