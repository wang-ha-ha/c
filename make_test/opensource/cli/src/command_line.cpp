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

#include "cli/command_line.h"
#include "cli/io_device.h"
#include "cli/menu.h"
#include "cli/syntax_node.h"
#include "cli/endl.h"
#include "cli/param.h"
#include "cli/traces.h"
#include "cli/assert.h"
#include "consistency.h"
#include "constraints.h"

CLI_NS_USE(cli)


//! @brief Command line trace class singleton redirection.
#define TRACE_CMD_LINE GetCmdLineTraceClass()
//! @brief Command line trace class singleton.
static const TraceClass& GetCmdLineTraceClass(void)
{
    static const TraceClass cli_CmdLineTraceClass("CLI_CMD_LINE", Help()
        .AddHelp(Help::LANG_EN, "Command line parsing")
        .AddHelp(Help::LANG_FR, "Analyse de lignes de commande"));
    return cli_CmdLineTraceClass;
}
//! @brief Command line splitting trace class singleton redirection.
#define TRACE_CMD_LINE_SPLIT GetCmdLineSplitTraceClass()
//! @brief Command line splitting trace class singleton.
static const TraceClass& GetCmdLineSplitTraceClass(void)
{
    static const TraceClass cli_CmdLineSplitTraceClass("CLI_CMD_LINE_SPLIT", Help()
        .AddHelp(Help::LANG_EN, "Split of command lines")
        .AddHelp(Help::LANG_FR, "Césure de mots sur analyse de lignes de commande"));
    return cli_CmdLineSplitTraceClass;
}


CommandLine::CommandLine(void)
  : m_cliElements(MAX_CMD_LINE_WORD_COUNT), m_cliAutoDelete(MAX_CMD_LINE_WORD_COUNT),
    m_pcliMenu(NULL),
    m_strLastWord(MAX_WORD_LENGTH), m_bLastWordValid(false),
    m_iNumBackspacesForCompletion(0)
{
    EnsureTraces();
}

CommandLine::~CommandLine(void)
{
    // Auto-deletion list.
    while (! m_cliAutoDelete.IsEmpty())
    {
        if (const Element* const pcli_Element = m_cliAutoDelete.RemoveHead())
        {
            delete pcli_Element;
        }
    }
}

const bool CommandLine::Parse(
        const Menu& CLI_Menu,
        const tk::String& STR_Line,
        const bool B_Execution
        )
{
    m_pcliMenu = & CLI_Menu;

    // Reset the error message.
    m_cliError = ResourceString();

    // Split the line into words.
    const Element* pcli_Node = & CLI_Menu;
    tk::Queue<tk::String> vstr_Words(MAX_CMD_LINE_WORD_COUNT);
    int i_LastWordPosition = -1;
    if (! Split(STR_Line, vstr_Words, i_LastWordPosition))
    {
        GetTraces().Trace(TRACE_CMD_LINE) << "Could not split '" << STR_Line << "' (" << m_cliError.GetString(ResourceString::LANG_EN) << ")" << endl;
        return false;
    }

    // Determine whether the last word should be parsed.
    int i_WordCount = vstr_Words.GetCount();
    if ((! B_Execution)
        && (! vstr_Words.IsEmpty())
        && (i_LastWordPosition >= 0))
    {
        i_WordCount --;
        if (! m_strLastWord.Set(vstr_Words.GetTail()))
        {
            GetTraces().Trace(INTERNAL_ERROR)
                << "CommandLine::Parse(): "
                << "Not enough space in m_strLastWord for the word '" << vstr_Words.GetTail() << "'"
                << endl;
        }
        m_bLastWordValid = true;
        m_iNumBackspacesForCompletion = (STR_Line.GetLength() - i_LastWordPosition);
    }
    else
    {
        m_strLastWord.Set("");
        m_bLastWordValid = false;
        m_iNumBackspacesForCompletion = 0;
    }

    // For each word, match the right element.
    tk::Queue<tk::String>::Iterator it = vstr_Words.GetIterator();
    for (   int i=0;
            vstr_Words.IsValid(it) && (i < i_WordCount);
            i ++)
    {
        // Search for elements that match the current word.
        GetTraces().Trace(TRACE_CMD_LINE) << "Word " << i << " '" << vstr_Words.GetAt(it) << "'" << endl;
        Element::List cli_ExactList(MAX_CMD_LINE_WORD_COUNT), cli_NearList(MAX_CMD_LINE_WORD_COUNT);
        if (! pcli_Node->FindElements(cli_ExactList, cli_NearList, vstr_Words.GetAt(it)))
        {
            m_cliError
                .SetString(ResourceString::LANG_EN, "Internal error")
                .SetString(ResourceString::LANG_FR, "Erreur interne");
            GetTraces().Trace(TRACE_CMD_LINE) << m_cliError.GetString(ResourceString::LANG_EN) << endl;
            return false;
        }

        if (cli_NearList.GetCount() == 0)
        {
            if (vstr_Words.GetAt(it) == "\n")
            {
                if (i == 0)
                {
                    // End of line
                    return true;
                }
                else
                {
                    // Incomplete command
                    m_cliError
                        .SetString(ResourceString::LANG_EN, "Incomplete command")
                        .SetString(ResourceString::LANG_FR, "Commande incomplète");
                    GetTraces().Trace(TRACE_CMD_LINE) << m_cliError.GetString(ResourceString::LANG_EN) << endl;
                    return false;
                }
            }
            else
            {
                // No matching element.
                // Syntax error.
                m_cliError
                    .SetString(ResourceString::LANG_EN, ResourceString::Concat("Syntax error next to '", vstr_Words.GetAt(it), "'"))
                    .SetString(ResourceString::LANG_FR, ResourceString::Concat("Erreur de syntaxe près de '", vstr_Words.GetAt(it), "'"));
                GetTraces().Trace(TRACE_CMD_LINE) << m_cliError.GetString(ResourceString::LANG_EN) << endl;
                return false;
            }
        }
        else if ((cli_ExactList.GetCount() > 1)
                || ((cli_ExactList.GetCount() == 0) && (cli_NearList.GetCount() > 1)))
        {
            // Ambiguous syntax
            m_cliError
                .SetString(ResourceString::LANG_EN, ResourceString::Concat("Ambiguous syntax next to '", vstr_Words.GetAt(it), "'"))
                .SetString(ResourceString::LANG_FR, ResourceString::Concat("Ambiguïté de syntaxe près de '", vstr_Words.GetAt(it), "'"));
            GetTraces().Trace(TRACE_CMD_LINE) << m_cliError.GetString(ResourceString::LANG_EN) << endl;
            return false;
        }
        else
        {
            // Element found.
            if (! cli_ExactList.IsEmpty())
            {
                // Prefer exactly matching elements if any.
                pcli_Node = cli_ExactList.GetHead();
            }
            else if (! cli_NearList.IsEmpty())
            {
                // First of the near elements otherwise.
                pcli_Node = cli_NearList.GetHead();
            }
            else
            {
                CLI_ASSERT(false);
            }
            // Add it to the command line.
            AddElement(pcli_Node);
        }

        vstr_Words.MoveNext(it);
    }

    // Check for final endl element.
    if (B_Execution && (i_WordCount > 0)
        && (dynamic_cast<const Endl*>(& GetLastElement()) == NULL))
    {
        m_cliError
            .SetString(ResourceString::LANG_EN, "Incomplete command")
            .SetString(ResourceString::LANG_FR, "Commande incomplète");
        GetTraces().Trace(TRACE_CMD_LINE) << m_cliError.GetString(ResourceString::LANG_EN) << endl;
        return false;
    }

    if (GetLastWord() != NULL)
    {
        GetTraces().Trace(TRACE_CMD_LINE) << "Last word '" << GetLastWord() << "'..." << endl;
    }
    return true;
}

const Element& CommandLine::GetLastElement(void) const
{
    if (m_cliElements.IsEmpty())
    {
        return *m_pcliMenu;
    }
    else
    {
        return *m_cliElements.GetTail();
    }
}

const char* const CommandLine::GetLastWord(void) const
{
    if (m_bLastWordValid)
    {
        return m_strLastWord;
    }
    else
    {
        return NULL;
    }
}

const int CommandLine::GetNumBackspacesForCompletion(void) const
{
    return m_iNumBackspacesForCompletion;
}

const ResourceString& CommandLine::GetLastError(void) const
{
    return m_cliError;
}

const bool CommandLine::Split(
        const tk::String& STR_Line,
        tk::Queue<tk::String>& VSTR_Words,
        int& I_LastWordPosition
        )
{
    I_LastWordPosition = -1;

    class Do { public:
        static const bool PushWord(
            const Element& CLI_Element, tk::Queue<tk::String>& VSTR_Words,
            const int I_Position, tk::String& STR_Word,
            ResourceString& CLI_Error)
        {
            if (VSTR_Words.AddTail(STR_Word))
            {
                GetTraces().Trace(TRACE_CMD_LINE_SPLIT) << "Word '" << STR_Word << "' pushed at " << I_Position << endl;
                STR_Word.Set("");
                return true;
            }
            else
            {
                CLI_Error
                    .SetString(ResourceString::LANG_EN, "Too many words in command line")
                    .SetString(ResourceString::LANG_FR, "Trop de mots dans la ligne de commande");
                GetTraces().Trace(TRACE_CMD_LINE_SPLIT) << CLI_Error.GetString(ResourceString::LANG_EN) << " on '" << STR_Word << "'" << endl;
                return false;
            }
        }
    };

    tk::String str_Word(MAX_WORD_LENGTH);
    bool b_EscapeMode = false;
    bool b_QuotedWord = false;
    for (unsigned int i=0; i<STR_Line.GetLength(); i++)
    {
        const char c = STR_Line[i];

        // End of line.
        if (c == '\n')
        {
            if (b_EscapeMode)
            {
                m_cliError
                    .SetString(ResourceString::LANG_EN, "Unterminated escape sequence")
                    .SetString(ResourceString::LANG_FR, "Séquence d'échappement incomplète");
                GetTraces().Trace(TRACE_CMD_LINE_SPLIT) << m_cliError.GetString(ResourceString::LANG_EN) << " on \\n" << endl;
                return false;
            }
            if (b_QuotedWord)
            {
                m_cliError
                    .SetString(ResourceString::LANG_EN, "Unterminated quoted string")
                    .SetString(ResourceString::LANG_FR, "Guillemets non fermés");
                GetTraces().Trace(TRACE_CMD_LINE_SPLIT) << m_cliError.GetString(ResourceString::LANG_EN) << " on \\n" << endl;
                return false;
            }
            if (! str_Word.IsEmpty())
            {
                if (! Do::PushWord(*m_pcliMenu, VSTR_Words, i, str_Word, m_cliError)) return false;
                GetTraces().Trace(TRACE_CMD_LINE_SPLIT) << "LastWordPosition set to -1 on \\n" << endl;
                I_LastWordPosition = -1;
            }
        }
        // Quote
        else if ((c == '"') && (! b_EscapeMode))
        {
            if (! b_QuotedWord)
            {
                // Beginning of quoted string.
                GetTraces().Trace(TRACE_CMD_LINE_SPLIT) << "Quoted string starting at " << i << endl;
                b_QuotedWord = true;
                // Push previous word if any.
                if (! str_Word.IsEmpty())
                {
                    if (! Do::PushWord(*m_pcliMenu, VSTR_Words, i, str_Word, m_cliError)) return false;
                }
                // Completion management.
                GetTraces().Trace(TRACE_CMD_LINE_SPLIT) << "LastWordPosition set to " << i << " on \"" << endl;
                I_LastWordPosition = i;
            }
            else
            {
                // End of quoted string.
                GetTraces().Trace(TRACE_CMD_LINE_SPLIT) << "Quoted string ending at " << i << endl;
                b_QuotedWord = false;
                // Push the word whatever.
                if (! Do::PushWord(*m_pcliMenu, VSTR_Words, i, str_Word, m_cliError)) return false;
                // No completion on quoted strings.
                GetTraces().Trace(TRACE_CMD_LINE_SPLIT) << "LastWordPosition set to -1 on \"" << endl;
                I_LastWordPosition = -1;
            }
        }
        // Blank characters.
        else if (((c == ' ') || (c == '\t')) && (! b_EscapeMode) && (! b_QuotedWord))
        {
            if (! str_Word.IsEmpty())
            {
                if (! Do::PushWord(*m_pcliMenu, VSTR_Words, i, str_Word, m_cliError)) return false;
                GetTraces().Trace(TRACE_CMD_LINE_SPLIT) << "LastWordPosition set to -1 on blank character" << endl;
                I_LastWordPosition = -1;
            }
        }
        // Escape character.
        else if ((c == '\\') && (! b_EscapeMode) && (i < STR_Line.GetLength() - 1))
        {
            GetTraces().Trace(TRACE_CMD_LINE_SPLIT) << "Escape mode starting at " << i << endl;
            b_EscapeMode = true;
            if (I_LastWordPosition < 0)
            {
                GetTraces().Trace(TRACE_CMD_LINE_SPLIT) << "LastWordPosition set to " << i << " on \\" << endl;
                I_LastWordPosition = i;
            }
        }
        // Non blank character.
        else
        {
            // Word expansion.
            if (! str_Word.Append(c))
            {
                // Internal error.
                m_cliError
                    .SetString(ResourceString::LANG_EN, ResourceString::Concat("Too long word '", str_Word.SubString(0, 10), "...'"))
                    .SetString(ResourceString::LANG_FR, ResourceString::Concat("Mot trop long '", str_Word.SubString(0, 10), "...'"));
                GetTraces().Trace(TRACE_CMD_LINE_SPLIT) << m_cliError.GetString(ResourceString::LANG_EN) << " on " << c << endl;
                return false;
            }
            if (I_LastWordPosition < 0)
            {
                GetTraces().Trace(TRACE_CMD_LINE_SPLIT) << "LastWordPosition set to " << i << " on non-blank character" << endl;
                I_LastWordPosition = i;
            }

            // Escape mode reset.
            if (b_EscapeMode)
            {
                GetTraces().Trace(TRACE_CMD_LINE_SPLIT) << "Escape mode ending at " << i << endl;
                b_EscapeMode = false;
            }
        }

        // End of line management.
        if (i >= STR_Line.GetLength() - 1)
        {
            if (! str_Word.IsEmpty())
            {
                if (! Do::PushWord(*m_pcliMenu, VSTR_Words, i, str_Word, m_cliError)) return false;
            }
        }

        if (c == '\n')
        {
            if (VSTR_Words.AddTail(tk::String(MAX_WORD_LENGTH, "\n")))
            {
                GetTraces().Trace(TRACE_CMD_LINE_SPLIT) << "\\n pushed at " << i << endl;
                GetTraces().Trace(TRACE_CMD_LINE_SPLIT) << "LastWordPosition set to -1 on \\n" << endl;
                I_LastWordPosition = -1;
                return true;
            }
            else
            {
                m_cliError
                    .SetString(ResourceString::LANG_EN, ResourceString::Concat("Too long word '", str_Word.SubString(0, 10), "...'"))
                    .SetString(ResourceString::LANG_FR, ResourceString::Concat("Mot trop long '", str_Word.SubString(0, 10), "...'"));
                GetTraces().Trace(TRACE_CMD_LINE_SPLIT) << m_cliError.GetString(ResourceString::LANG_EN) << " on \\n" << endl;
                return false;
            }
        }
    }

    if (b_EscapeMode)
    {
        m_cliError
            .SetString(ResourceString::LANG_EN, "Unterminated escape sequence")
            .SetString(ResourceString::LANG_FR, "Séquence d'échappement incomplète");
        GetTraces().Trace(TRACE_CMD_LINE_SPLIT) << m_cliError.GetString(ResourceString::LANG_EN) << " on end of line" << endl;
        return false;
    }
    if (b_QuotedWord)
    {
        m_cliError
            .SetString(ResourceString::LANG_EN, "Unterminated quoted string")
            .SetString(ResourceString::LANG_FR, "Guillemets non fermés");
        GetTraces().Trace(TRACE_CMD_LINE_SPLIT) << "Unterminated quoted string on end of line" << endl;
        return false;
    }
    return true;
}

CommandLine& CommandLine::AddElement(const Element* const PCLI_Element)
{
    if (const Param* const pcli_Param = dynamic_cast<const Param*>(PCLI_Element))
    {
        const Param* const pcli_Clone = pcli_Param->Clone();
        if (! m_cliElements.AddTail(pcli_Clone))
        {
            CLI_ASSERT(false);
        }
        if (! m_cliAutoDelete.AddTail(pcli_Clone))
        {
            CLI_ASSERT(false);
        }
    }
    else
    {
        if (! m_cliElements.AddTail(PCLI_Element))
        {
            CLI_ASSERT(false);
        }
    }
    return *this;
}


CommandLineIterator::CommandLineIterator(const CommandLine& CLI_CmdLine)
  : m_cliCmdLine(CLI_CmdLine),
    m_cliIterator(m_cliCmdLine.m_cliElements.GetIterator()), m_pcliCurrentElement(NULL)
{
}

CommandLineIterator::~CommandLineIterator(void)
{
}

const bool CommandLineIterator::StepIt(void)
{
    if (m_cliCmdLine.m_cliElements.IsValid(m_cliIterator))
    {
        m_pcliCurrentElement = m_cliCmdLine.m_cliElements.GetAt(m_cliIterator);
        m_cliCmdLine.m_cliElements.MoveNext(m_cliIterator);
        return (m_pcliCurrentElement != NULL);
    }
    else
    {
        return false;
    }
}

const bool CommandLineIterator::operator==(const Element& CLI_Element) const
{
    const cli::Param* const pcli_RefParam = dynamic_cast<const cli::Param*>(& CLI_Element);
    const cli::Param* const pcli_CmdLineParam = dynamic_cast<const cli::Param*>(m_pcliCurrentElement);

    if ((pcli_RefParam != NULL) && (pcli_CmdLineParam != NULL)
        && (pcli_CmdLineParam->GetCloned() == pcli_RefParam))
    {
        pcli_RefParam->CopyValue(*pcli_CmdLineParam);
        return true;
    }
    else
    {
        return (m_pcliCurrentElement == & CLI_Element);
    }
}

const Element* const CommandLineIterator::operator*(void) const
{
    return m_pcliCurrentElement;
}
