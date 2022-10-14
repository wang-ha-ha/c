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

#include "cli/assert.h"
#include "cli/resource_string.h"

#include "constraints.h"
#include "command_line_history.h"


CLI_NS_USE(cli)


CmdLineHistory::CmdLineHistory(const unsigned int UI_StackSize)
  : m_uiStackSize(UI_StackSize),
    m_qHistory(UI_StackSize + 1),   // History stack owns UI_HistoryCount + 1 elements for UI_HistoryCount previous and another one for the current line.
    m_uiNavigationIndex(0), m_bNavigation(false)
{
    m_qHistory.AddHead(CmdLineEdition());
}

CmdLineHistory::~CmdLineHistory(void)
{
}

const bool CmdLineHistory::Push(const CmdLineEdition& CLI_Line)
{
    if (// Check the line is not empty.
        (! CLI_Line.GetLine().IsEmpty())
        // Check it is not the same as the previous one.
        && ((m_qHistory.GetCount() <= 1) || (CLI_Line.GetLine() != GetLine(1).GetLine())))
    {
        // Limit to HISTORY_STACK_SIZE.
        while (m_qHistory.GetCount() > m_uiStackSize)
        {
            m_qHistory.RemoveTail();
        }

        // Ensure there is at least one element.
        if (m_qHistory.IsEmpty())
        {
            if (! m_qHistory.AddTail(CmdLineEdition()))
            {
                return false;
            }
        }

        // Backup the current line.
        m_qHistory.GetHead() = CLI_Line;

        // Add a new empty element.
        m_qHistory.AddHead(CmdLineEdition());
    }

    m_uiNavigationIndex = 0;
    m_bNavigation = false;

    return true;
}

const bool CmdLineHistory::Clear(void)
{
    while (! m_qHistory.IsEmpty())
    {
        m_qHistory.RemoveHead();
    }
    m_qHistory.AddHead(CmdLineEdition());

    return true;
}

const CmdLineEdition& CmdLineHistory::GetLine(const unsigned int UI_BackwardIndex) const
{
    int i = 0;
    tk::Queue<CmdLineEdition>::Iterator it = m_qHistory.GetIterator();
    for (i=UI_BackwardIndex; i>0; i--)
    {
        if (! m_qHistory.MoveNext(it))
        {
            break;
        }
    }

    if ((i==0) && m_qHistory.IsValid(it))
    {
        return m_qHistory.GetAt(it);
    }
    else
    {
        static CmdLineEdition cli_Error;
        CLI_ASSERT(false);
        return cli_Error;
    }
}

const unsigned int CmdLineHistory::GetCount(void) const
{
    return m_qHistory.GetCount();
}

const bool CmdLineHistory::SaveCurrentLine(const CmdLineEdition& CLI_CurrentLine)
{
    m_qHistory.GetHead() = CLI_CurrentLine;
    return true;
}

const bool CmdLineHistory::Navigate(
        CmdLineEdition& CLI_CmdLine, const OutputDevice& CLI_Stream,
        const int I_Navigation)
{
    bool b_Res = false;

    if (! m_qHistory.IsEmpty())
    {
        b_Res = true;

        // Initialize navigation if needed.
        if (! m_bNavigation)
        {
            m_uiNavigationIndex = 0;
            m_bNavigation = true;
        }

        // Determine which history line to select.
        int i_NavigationIndex = ((int) m_uiNavigationIndex) + I_Navigation;
        if (i_NavigationIndex >= ((int) m_qHistory.GetCount()))
        {
            i_NavigationIndex = ((int) m_qHistory.GetCount()) - 1;
            b_Res = false;
        }
        if (i_NavigationIndex < 0)
        {
            i_NavigationIndex = 0;
            b_Res = false;
        }

        // If this is the current line, save it into the history stack.
        if (m_uiNavigationIndex == 0)
        {
            m_qHistory.GetHead() = CLI_CmdLine;
        }
        // Clean up the current line.
        CLI_CmdLine.CleanAll(CLI_Stream);
        // Translate the history index.
        m_uiNavigationIndex = (unsigned int) i_NavigationIndex;
        // Print out this line.
        CLI_CmdLine = GetLine(m_uiNavigationIndex);
        CLI_CmdLine.PrintCmdLine(CLI_Stream);
    }

    return b_Res;
}

void CmdLineHistory::EnableNavigationMemory(const bool B_NavigationMemory)
{
    m_bNavigation = B_NavigationMemory;
}
