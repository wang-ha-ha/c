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

#include "cli/io_device.h"
#include "cli/string_device.h"

#include "command_line_edition.h"
#include "constraints.h"
#include "encoding.h"


CLI_NS_USE(cli)


CmdLineEdition::CmdLineEdition(const CmdLineEdition& CLI_CmdLine)
  : m_tkLeft(CLI_CmdLine.m_tkLeft),
    m_tkRight(CLI_CmdLine.m_tkRight),
    m_bInsertMode(CLI_CmdLine.m_bInsertMode)
{
}

CmdLineEdition::CmdLineEdition(void)
  : m_tkLeft(MAX_CMD_LINE_LENGTH), m_tkRight(MAX_CMD_LINE_LENGTH), m_bInsertMode(true)
{
}

CmdLineEdition::~CmdLineEdition(void)
{
}

CmdLineEdition& CmdLineEdition::operator=(const CmdLineEdition& CLI_CmdLine)
{
    m_tkLeft.Reset();
    m_tkRight.Reset();

    for (tk::Queue<KEY>::Iterator it1 = CLI_CmdLine.m_tkLeft.GetIterator(); CLI_CmdLine.m_tkLeft.IsValid(it1); CLI_CmdLine.m_tkLeft.MoveNext(it1))
    {
        m_tkLeft.AddTail(CLI_CmdLine.m_tkLeft.GetAt(it1));
    }
    for (tk::Queue<KEY>::Iterator it2 = CLI_CmdLine.m_tkRight.GetIterator(); CLI_CmdLine.m_tkRight.IsValid(it2); CLI_CmdLine.m_tkRight.MoveNext(it2))
    {
        m_tkRight.AddTail(CLI_CmdLine.m_tkRight.GetAt(it2));
    }
    m_bInsertMode = CLI_CmdLine.m_bInsertMode;

    return *this;
}

void CmdLineEdition::Set(const tk::String& TK_Left, const tk::String& TK_Right)
{
    m_tkLeft.Reset();
    m_tkRight.Reset();

    // Simply set both parts of the command line.
    StringDecoder cli_Left((const char*) TK_Left);
    for (   KEY e_Left = cli_Left.GetKey();
            (e_Left != NULL_KEY) && (m_tkLeft.GetCount() < MAX_CMD_LINE_LENGTH);
            e_Left = cli_Left.GetKey())
    {
        m_tkLeft.AddTail(e_Left);
    }

    StringDecoder cli_Right((const char*) TK_Right);
    for (   KEY e_Right = cli_Right.GetKey();
            (e_Right != NULL_KEY) && (m_tkLeft.GetCount() + m_tkRight.GetCount() < MAX_CMD_LINE_LENGTH);
            e_Right = cli_Right.GetKey())
    {
        m_tkRight.AddTail(e_Right);
    }
}

void CmdLineEdition::Reset(void)
{
    // Simply reset both parts of the command line.
    m_tkLeft.Reset();
    m_tkRight.Reset();
}

void CmdLineEdition::SetInsertMode(const bool B_InsertMode)
{
    m_bInsertMode = B_InsertMode;
}

const bool CmdLineEdition::GetInsertMode(void) const
{
    return m_bInsertMode;
}

void CmdLineEdition::Put(const OutputDevice& CLI_OutputDevice, const KEY E_Char)
{
    tk::Queue<KEY> tk_Keys(1);
    tk_Keys.AddTail(E_Char);
    Put(CLI_OutputDevice, tk_Keys);
}

void CmdLineEdition::Put(const OutputDevice& CLI_OutputDevice, const tk::String& TK_String)
{
    tk::Queue<KEY> tk_Keys(MAX_CMD_LINE_LENGTH);

    StringDecoder cli_String((const char*) TK_String);
    while (const KEY e_Char = cli_String.GetKey())
    {
        tk_Keys.AddTail(e_Char);
    }

    // Process input keys.
    Put(CLI_OutputDevice, tk_Keys);
}

void CmdLineEdition::Put(const OutputDevice& CLI_OutputDevice, const tk::Queue<KEY>& TK_Keys)
{
    for (   tk::Queue<KEY>::Iterator it1 = TK_Keys.GetIterator();
            TK_Keys.IsValid(it1) && (m_tkLeft.GetCount() + m_tkRight.GetCount() + ((m_bInsertMode || m_tkRight.IsEmpty()) ? 1 : 0) <= MAX_CMD_LINE_LENGTH);
            TK_Keys.MoveNext(it1))
    {
        // First of all, append the left part of the command line.
        m_tkLeft.AddTail(TK_Keys.GetAt(it1));

        // Print out the characters.
        CLI_OutputDevice << TK_Keys.GetAt(it1);

        if (! m_bInsertMode)
        {
            // Remove the first characters of the right part of the command line.
            if (! m_tkRight.IsEmpty())
            {
                m_tkRight.RemoveHead();
            }
        }
    }
    if (m_bInsertMode)
    {
        // Refresh the right part of the line.
        for (   tk::Queue<KEY>::Iterator it2 = m_tkRight.GetIterator();
                m_tkRight.IsValid(it2);
                m_tkRight.MoveNext(it2))
        {
            CLI_OutputDevice << m_tkRight.GetAt(it2);
        }

        // Move the cursor to the left.
        for (unsigned int ui = m_tkRight.GetCount(); ui > 0; ui --)
        {
            CLI_OutputDevice.PutString("\b");
        }
    }
}

void CmdLineEdition::CleanAll(const OutputDevice& CLI_OutputDevice)
{
    Delete(CLI_OutputDevice, (int) m_tkRight.GetCount());
    Delete(CLI_OutputDevice, - (int) m_tkLeft.GetCount());
}

void CmdLineEdition::Delete(const OutputDevice& CLI_OutputDevice, const int I_Count)
{
    if (I_Count > 0)
    {
        // Delete forward

        // Find out the pattern to keep, reduce the right part of the command line.
        unsigned int ui_CharCount = 0;
        for (ui_CharCount = 0; (ui_CharCount < (unsigned int) I_Count) && (! m_tkRight.IsEmpty()); ui_CharCount ++)
        {
            m_tkRight.RemoveHead();
        }
        // Print over the kept right part of the command line.
        CLI_OutputDevice << GetRight();
        // Blank the useless characters at the end of the line.
        for (unsigned int ui_Blank = ui_CharCount; ui_Blank > 0; ui_Blank --)
        {
            CLI_OutputDevice << " ";
        }
        // Move back the cursor.
        for (unsigned int ui_Back = m_tkRight.GetCount() + ui_CharCount; ui_Back > 0; ui_Back --)
        {
            CLI_OutputDevice << "\b";
        }
    }
    else if (I_Count < 0)
    {
        // Delete backward.

        // Find out the pattern to keep, reduce the left part of the commane line.
        unsigned int ui_CharCount = 0;
        for (ui_CharCount = 0; (ui_CharCount < (unsigned int) -I_Count) && (! m_tkLeft.IsEmpty()); ui_CharCount ++)
        {
            m_tkLeft.RemoveTail();
        }
        // Move back the cursor.
        for (unsigned int ui_Back1 = ui_CharCount; ui_Back1 > 0; ui_Back1 --)
        {
            CLI_OutputDevice << "\b";
        }
        // Print over the right part of the command line.
        CLI_OutputDevice << GetRight();
        // Blank the useless characters at the end of the line.
        for (unsigned int ui_Blank = ui_CharCount; ui_Blank > 0; ui_Blank --)
        {
            CLI_OutputDevice << " ";
        }
        // Move back the cursor.
        for (unsigned int ui_Back2 = m_tkRight.GetCount() + ui_CharCount; ui_Back2 > 0; ui_Back2 --)
        {
            CLI_OutputDevice << "\b";
        }
    }
}

void CmdLineEdition::PrintCmdLine(const OutputDevice& CLI_OutputDevice) const
{
    CLI_OutputDevice << GetLeft();
    CLI_OutputDevice << GetRight();
    for (unsigned int ui = m_tkRight.GetCount(); ui > 0; ui --)
    {
        CLI_OutputDevice << '\b';
    }
}

void CmdLineEdition::MoveCursor(const OutputDevice& CLI_OutputDevice, const int I_Count)
{
    if (I_Count > 0)
    {
        // Move forward
        for (unsigned int ui_Forward = 0; (ui_Forward < (unsigned int) I_Count) && (! m_tkRight.IsEmpty()); ui_Forward ++)
        {
            const KEY e_Key = m_tkRight.RemoveHead();
            m_tkLeft.AddTail(e_Key);
            CLI_OutputDevice << e_Key;
        }
    }
    else if (I_Count < 0)
    {
        // Move backward.
        for (unsigned int ui_Backward = 0; (ui_Backward < (unsigned int) -I_Count) && (! m_tkLeft.IsEmpty()); ui_Backward ++)
        {
            const KEY e_Key = m_tkLeft.RemoveTail();
            m_tkRight.AddHead(e_Key);
            CLI_OutputDevice.PutString("\b");
        }
    }
}

void CmdLineEdition::NextLine(const OutputDevice& CLI_OutputDevice)
{
    CLI_OutputDevice << GetRight() << endl;
}

void CmdLineEdition::Home(const OutputDevice& CLI_OutputDevice)
{
    MoveCursor(CLI_OutputDevice, - (int) m_tkLeft.GetCount());
}

void CmdLineEdition::End(const OutputDevice& CLI_OutputDevice)
{
    MoveCursor(CLI_OutputDevice, (int) m_tkRight.GetCount());
}

const tk::String CmdLineEdition::GetLine(void) const
{
    return tk::String::Concat(MAX_CMD_LINE_LENGTH, GetLeft(), GetRight());
}

const tk::String CmdLineEdition::GetLeft(void) const
{
    StringDevice cli_Left(MAX_CMD_LINE_LENGTH * 3, false); // Special characters may be encoded with 3 bytes max in utf-8
    for (tk::Queue<KEY>::Iterator it = m_tkLeft.GetIterator(); m_tkLeft.IsValid(it); m_tkLeft.MoveNext(it))
    {
        cli_Left << m_tkLeft.GetAt(it);
    }
    return cli_Left.GetString();
}

const tk::String CmdLineEdition::GetRight(void) const
{
    StringDevice cli_Right(MAX_CMD_LINE_LENGTH * 3, false); // Special characters may be encoded with 3 bytes max in utf-8
    for (tk::Queue<KEY>::Iterator it = m_tkRight.GetIterator(); m_tkRight.IsValid(it); m_tkRight.MoveNext(it))
    {
        cli_Right << m_tkRight.GetAt(it);
    }
    return cli_Right.GetString();
}

const tk::String CmdLineEdition::GetNextWord(void) const
{
    StringDevice cli_Word(MAX_CMD_LINE_LENGTH * 3, false); // Special characters may be encoded with 3 bytes max in utf-8

    // Skip blank characters.
    tk::Queue<KEY>::Iterator it = m_tkRight.GetIterator();
    for ( ; m_tkRight.IsValid(it) && (m_tkRight.GetAt(it) == SPACE); m_tkRight.MoveNext(it))
    {
        cli_Word << m_tkRight.GetAt(it);
    }

    // Store characters until the next blank character.
    for ( ; m_tkRight.IsValid(it) && (m_tkRight.GetAt(it) != SPACE); m_tkRight.MoveNext(it))
    {
        cli_Word << m_tkRight.GetAt(it);
    }

    // Return the next word.
    return cli_Word.GetString();
}

const tk::String CmdLineEdition::GetPrevWord(void) const
{
    tk::Queue<KEY> tk_Word(MAX_CMD_LINE_LENGTH);

    // Reverse the left part.
    tk::Queue<KEY> tk_tfeL(MAX_CMD_LINE_LENGTH);
    for (tk::Queue<KEY>::Iterator it1 = m_tkLeft.GetIterator(); m_tkLeft.IsValid(it1); m_tkLeft.MoveNext(it1))
    {
        tk_tfeL.AddHead(m_tkLeft.GetAt(it1));
    }

    // Skip blank characters.
    tk::Queue<KEY>::Iterator it = tk_tfeL.GetIterator();
    for ( ; tk_tfeL.IsValid(it) && (tk_tfeL.GetAt(it) == SPACE); tk_tfeL.MoveNext(it))
    {
        tk_Word.AddHead(tk_tfeL.GetAt(it));
    }

    // Skip characters until the next blank character.
    for ( ; tk_tfeL.IsValid(it) && (tk_tfeL.GetAt(it) != SPACE); tk_tfeL.MoveNext(it))
    {
        tk_Word.AddHead(tk_tfeL.GetAt(it));
    }

    // Build the word from the queue.
    StringDevice cli_Word(MAX_CMD_LINE_LENGTH * 3, false); // Special characters may be encoded with 3 bytes max in utf-8
    for (tk::Queue<KEY>::Iterator it2 = tk_Word.GetIterator(); tk_Word.IsValid(it2); tk_Word.MoveNext(it2))
    {
        cli_Word << tk_Word.GetAt(it2);
    }

    // Return the previous word.
    return cli_Word.GetString();
}

