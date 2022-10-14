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

#include "cli/keyword.h"
#include "cli/param_string.h"
#include "cli/syntax_tag.h"
#include "cli/endl.h"
#include "cli/command_line.h"
#include "cli/shell.h"
#include "cli/io_device.h"
#include "cli/assert.h"
#include "traces_menu.h"
#include "consistency.h"

CLI_NS_USE(cli)


TracesMenu::TracesMenu(void)
  : Menu("traces", Help()
        .AddHelp(Help::LANG_EN, "Traces")
        .AddHelp(Help::LANG_FR, "Traces")),
    m_pcliShowNode(NULL), m_pcliShowFilterNode(NULL), m_pcliShowClassesNode(NULL),
    m_pcliNoNode(NULL), m_pcliTraceNode(NULL), m_pcliFilterParam(NULL), m_pcliAllFilterNode(NULL)
{
    EnsureTraces();
}

TracesMenu::~TracesMenu(void)
{
}

void TracesMenu::SetCli(Cli& CLI_Cli)
{
    Menu::SetCli(CLI_Cli);
    {   Help cli_Help(Help()
            .AddHelp(Help::LANG_EN, "Show traces behavior")
            .AddHelp(Help::LANG_FR, "Affichage du comportement du système de traces"));
        m_pcliShowNode = dynamic_cast<Keyword*>(& AddElement(new Keyword("show", cli_Help)));
        {   Help cli_Help(Help()
                .AddHelp(Help::LANG_EN, "Show current filter")
                .AddHelp(Help::LANG_FR, "Affichage du filtre courant"));
            m_pcliShowFilterNode = dynamic_cast<Keyword*>(& m_pcliShowNode->AddElement(new Keyword("filter", cli_Help)));
            m_pcliShowFilterNode->AddElement(new Endl(cli_Help)); }
        {   Help cli_Help(Help()
                .AddHelp(Help::LANG_EN, "Show all trace classes")
                .AddHelp(Help::LANG_FR, "Affichage de la liste des classes"));
            m_pcliShowClassesNode = dynamic_cast<Keyword*>(& m_pcliShowNode->AddElement(new Keyword("classes", cli_Help)));
            m_pcliShowClassesNode->AddElement(new Endl(cli_Help)); }}
    {   SyntaxTag* pcli_Tag = dynamic_cast<SyntaxTag*>(& AddElement(new SyntaxTag(false)));
        {   Help cli_Help(Help()
                .AddHelp(Help::LANG_EN, "Trace setting")
                .AddHelp(Help::LANG_FR, "Configuration du système de traces"));
            m_pcliTraceNode = dynamic_cast<Keyword*>(& pcli_Tag->AddElement(new Keyword("trace", cli_Help)));
            {   Help cli_Help(Help()
                    .AddHelp(Help::LANG_EN, "Filter selection")
                    .AddHelp(Help::LANG_FR, "Sélection d'un filtre"));
                m_pcliTraceFilterNode = dynamic_cast<Keyword*>(& m_pcliTraceNode->AddElement(new Keyword("filter", cli_Help)));
                {   Help cli_Help(Help()
                        .AddHelp(Help::LANG_EN, "Trace class name")
                        .AddHelp(Help::LANG_FR, "Nom d'une classe de traces"));
                    m_pcliFilterParam = dynamic_cast<ParamString*>(& m_pcliTraceFilterNode->AddElement(new ParamString(cli_Help)));
                    {
                        Help cli_Help(Help()
                            .AddHelp(Help::LANG_EN, "Enable/disable traces for the given trace filter")
                            .AddHelp(Help::LANG_EN, "Activer/désactiver les traces pour le filtre donné"));
                        m_pcliFilterParam->AddElement(new Endl(cli_Help)); }}}
            {   Help cli_Help(Help()
                    .AddHelp(Help::LANG_EN, "All traces")
                    .AddHelp(Help::LANG_FR, "Toutes les traces"));
                m_pcliAllFilterNode = dynamic_cast<Keyword*>(& m_pcliTraceNode->AddElement(new Keyword("all", cli_Help)));
                {
                    Help cli_Help(Help()
                        .AddHelp(Help::LANG_EN, "Enable/disable all traces")
                        .AddHelp(Help::LANG_EN, "Activer/désactiver toutes les traces"));
                    m_pcliAllFilterNode->AddElement(new Endl(cli_Help)); }}}
        Help cli_Help(Help()
            .AddHelp(Help::LANG_EN, "Remove traces")
            .AddHelp(Help::LANG_FR, "Suppression de traces"));
        m_pcliNoNode = dynamic_cast<Keyword*>(& AddElement(new Keyword("no", cli_Help)));
        m_pcliNoNode->AddElement(new SyntaxRef(*pcli_Tag)); }
}

const bool TracesMenu::ExecuteReserved(const CommandLine& CLI_CmdLine) const
{
    CommandLineIterator it(CLI_CmdLine);
    bool b_Show = true;

    if (! it.StepIt()) { return false; }
    else if (it == GetShowNode())
    {
        if (! it.StepIt()) { return false; }
        else if (it == GetShowFilterNode())
        {
            if (! it.StepIt()) { return false; }
            if (dynamic_cast<const Endl*>(*it))
            {
                ShowCurrentFilter();
                return true;
            }
        }
        else if (it == GetShowClassesNode())
        {
            if (! it.StepIt()) { return false; }
            if (dynamic_cast<const Endl*>(*it))
            {
                ShowAllClasses();
                return true;
            }
        }
    }
    else if (it == GetNoNode())
    {
        b_Show = false;
        if (! it.StepIt()) { return false; }
        else if (it == GetTraceNode())
        {
            if (! it.StepIt()) { return false; }
            goto trace_label;
        }
    }
    else if (it == GetTraceNode())
    {
        if (! it.StepIt()) { return false; }
trace_label:
        if (false) {}
        else if (it == GetFilterNode())
        {
            if (! it.StepIt()) { return false; }
            if (it == GetFilterParam())
            {
                if (! it.StepIt()) { return false; }
                if (dynamic_cast<const Endl*>(*it))
                {
                    SetFilter(GetFilterParam(), b_Show);
                    return true;
                }
            }
        }
        else if (it == GetAllFilterNode())
        {
            if (! it.StepIt()) { return false; }
            if (dynamic_cast<const Endl*>(*it))
            {
                SetAllFilter(b_Show);
                return true;
            }
        }
    }

    return Menu::ExecuteReserved(CLI_CmdLine);
}

const bool TracesMenu::Execute(const CommandLine& CLI_CmdLine) const
{
    return false;
}

void TracesMenu::ShowAllClasses(void) const
{
    const ResourceString cli_AllClasses = ResourceString()
        .SetString(ResourceString::LANG_EN, "All classes:")
        .SetString(ResourceString::LANG_FR, "Toutes les classes:");
    GetOutputStream() << cli_AllClasses.GetString(GetShell().GetLang()) << endl;
    DisplayClassList(GetTraces().GetAllClasses());
}

void TracesMenu::ShowCurrentFilter(void) const
{
    const ResourceString cli_CurrentFilter = ResourceString()
        .SetString(ResourceString::LANG_EN, "Current filter:")
        .SetString(ResourceString::LANG_FR, "Filtre courant:");
    GetOutputStream() << cli_CurrentFilter.GetString(GetShell().GetLang()) << endl;
    DisplayClassList(GetTraces().GetCurrentFilter());
}

void TracesMenu::DisplayClassList(const TraceClass::List& Q_Classes) const
{
    for (   TraceClass::List::Iterator it = Q_Classes.GetIterator();
            Q_Classes.IsValid(it);
            Q_Classes.MoveNext(it))
    {
        // Margin.
        unsigned int ui_Len = 0;
        for (; ui_Len < GetShell().GetHelpMargin(); ui_Len ++)
        {
            GetOutputStream() << " ";
        }

        // Display class name.
        {
            const tk::String str_Name = Q_Classes.GetAt(it).GetName();
            GetOutputStream() << "<" << str_Name << ">";
            ui_Len += (str_Name.GetLength() + 2);
        }

        // Offset.
        if (ui_Len >= GetShell().GetHelpOffset())
        {
            GetOutputStream() << " ";
            ui_Len ++;
        }
        for (; ui_Len < GetShell().GetHelpOffset(); ui_Len ++)
        {
            GetOutputStream() << " ";
        }

        // Display help.
        {
            const tk::String str_Help = Q_Classes.GetAt(it).GetHelp().GetString(GetShell().GetLang());
            GetOutputStream() << str_Help;
            ui_Len += str_Help.GetLength();
        }

        // End of line.
        GetOutputStream() << endl;
    }
}

void TracesMenu::SetFilter(const char* const STR_ClassName, const bool B_Show) const
{
    const TraceClass cli_Class(STR_ClassName, Help());
    GetTraces().SetFilter(cli_Class, B_Show);
}

void TracesMenu::SetAllFilter(const bool B_Show) const
{
    GetTraces().SetAllFilter(B_Show);
}

const Keyword& TracesMenu::GetShowNode(void) const
{
    CLI_ASSERT(m_pcliShowNode != NULL);
    return *m_pcliShowNode;
}

const Keyword& TracesMenu::GetShowFilterNode(void) const
{
    CLI_ASSERT(m_pcliShowFilterNode != NULL);
    return *m_pcliShowFilterNode;
}

const Keyword& TracesMenu::GetShowClassesNode(void) const
{
    CLI_ASSERT(m_pcliShowClassesNode != NULL);
    return *m_pcliShowClassesNode;
}

const Keyword& TracesMenu::GetNoNode(void) const
{
    CLI_ASSERT(m_pcliNoNode != NULL);
    return *m_pcliNoNode;
}

const Keyword& TracesMenu::GetTraceNode(void) const
{
    CLI_ASSERT(m_pcliTraceNode != NULL);
    return *m_pcliTraceNode;
}

const Keyword& TracesMenu::GetFilterNode(void) const
{
    CLI_ASSERT(m_pcliTraceFilterNode != NULL);
    return *m_pcliTraceFilterNode;
}

const ParamString& TracesMenu::GetFilterParam(void) const
{
    CLI_ASSERT(m_pcliFilterParam != NULL);
    return *m_pcliFilterParam;
}

const Keyword& TracesMenu::GetAllFilterNode(void) const
{
    CLI_ASSERT(m_pcliAllFilterNode != NULL);
    return *m_pcliAllFilterNode;
}

