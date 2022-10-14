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
#include "ui_text.h"


CLI_NS_BEGIN(cli)

    CLI_NS_BEGIN(ui)

        Text::Text(const unsigned int UI_MaxLines, const unsigned int UI_MaxLineLength)
          : OutputDevice("less", false),
            m_tkLines(UI_MaxLines), m_bNewLineRequired(true), m_uiMaxLineLength(UI_MaxLineLength)
        {
        }

        Text::~Text(void)
        {
        }

        const bool Text::OpenDevice(void)
        {
            return true;
        }

        const bool Text::CloseDevice(void)
        {
            return true;
        }

        void Text::PutString(const char* const STR_Out) const
        {
            for (const char* pc_Out = STR_Out; (pc_Out != NULL) && (*pc_Out != '\0'); pc_Out ++)
            {
                if (m_bNewLineRequired)
                {
                    m_tkLines.AddTail(tk::String(m_uiMaxLineLength));
                    m_bNewLineRequired = false;
                }

                if (*pc_Out == '\n')
                {
                    m_bNewLineRequired = true;
                }
                else
                {
                    CLI_ASSERT(! m_tkLines.IsEmpty());
                    if (! m_tkLines.IsEmpty())
                    {
                        m_tkLines.GetTail().Append(*pc_Out);
                    }
                }
            }
        }

        void Text::Beep(void) const
        {
            // Do nothing.
        }

        void Text::CleanScreen(void) const
        {
            // Not sure there is an interest for cleaning a 'less' object...
            // Whatever...
            m_tkLines.Reset();
        }

        const bool Text::WouldOutput(const OutputDevice& CLI_Device) const
        {
            if (OutputDevice::WouldOutput(CLI_Device))
            {
                return true;
            }

            return false;
        }

        void Text::Begin(TextIterator& it) const
        {
            // Set cursor at the beginning of the text.
            it.m_tkTopLine = m_tkLines.GetIterator();
            it.m_uiTopChar = 0;
            it.m_bBottomIsUpToDate = false; // Bottom position is out of date until PrintPage() is called.
        }

        const bool Text::PageUp(TextIterator& it) const
        {
            // Try to skeep as many lines as the screen is high.
            for (unsigned int ui = 0; ui < it.m_uiPageHeight; ui ++)
            {
                if (! LineUp(it))
                {
                    // If the iterator could not be moved one single line up, return false.
                    return (ui > 0);
                }
            }

            return true;
        }

        const bool Text::LineUp(TextIterator& it) const
        {
            if (m_tkLines.IsValid(it.m_tkTopLine))
            {
                // Check whether the current top character position is not aligned to the beginning of a line.
                if (it.m_uiTopChar >= it.m_cliScreenInfo.GetSafeWidth())
                {
                    it.m_uiTopChar -= it.m_cliScreenInfo.GetSafeWidth();
                    it.m_bBottomIsUpToDate = false; // Bottom position is out of date until PrintPage() is called.
                    return true;
                }

                // Try to decrement the current top line position.
                if (m_tkLines.MovePrevious(it.m_tkTopLine))
                {
                    CLI_ASSERT(it.m_cliScreenInfo.GetSafeWidth() > 0);
                    if (it.m_cliScreenInfo.GetSafeWidth() <= 0)
                    {
                        // Should not happen unless there is a bug in OutputDevice::ScreenDimension.
                        // If ever, let's start at the beginning of the upper line.
                        it.m_uiTopChar = 0;
                    }
                    else
                    {
                        it.m_uiTopChar = m_tkLines.GetAt(it.m_tkTopLine).GetLength();
                        if (m_tkLines.GetAt(it.m_tkTopLine).GetLength() % it.m_cliScreenInfo.GetSafeWidth() == 0)
                        {
                            it.m_uiTopChar -= it.m_cliScreenInfo.GetSafeWidth();
                        }
                        else
                        {
                            it.m_uiTopChar -= m_tkLines.GetAt(it.m_tkTopLine).GetLength() % it.m_cliScreenInfo.GetSafeWidth();
                        }
                    }
                    it.m_bBottomIsUpToDate = false; // Bottom position is out of date until PrintPage() is called.

                    return true;
                }
                else
                {
                    // Restore position to beginning.
                    CLI_ASSERT(it.m_uiTopChar == 0);
                    Begin(it);
                }
            }

            return false;
        }

        const bool Text::LineDown(TextIterator& it, const OutputDevice* const PCLI_Out) const
        {
            // First check bottom position is up to date according to top position.
            if (! it.m_bBottomIsUpToDate)
            {
                PrintPage(it, OutputDevice::GetNullDevice(), false);
            }

            // Check whether the bottom line can be incremented.
            if (it.m_bBottomIsUpToDate && m_tkLines.IsValid(it.m_tkBottomLine))
            {
                CLI_ASSERT(m_tkLines.IsValid(it.m_tkTopLine));
                if (m_tkLines.IsValid(it.m_tkTopLine))
                {
                    // OK let's go.
                    it.m_uiTopChar += it.m_cliScreenInfo.GetSafeWidth();
                    if (it.m_uiTopChar >= m_tkLines.GetAt(it.m_tkTopLine).GetLength())
                    {
                        m_tkLines.MoveNext(it.m_tkTopLine);
                        it.m_uiTopChar = 0;
                    }

                    if (PCLI_Out != NULL)
                    {
                        PrintBottomLine(it, *PCLI_Out);
                    }
                    else
                    {
                        it.m_uiBottomChar += it.m_cliScreenInfo.GetSafeWidth();
                        if (it.m_uiBottomChar >= m_tkLines.GetAt(it.m_tkBottomLine).GetLength())
                        {
                            m_tkLines.MoveNext(it.m_tkBottomLine);
                            it.m_uiBottomChar = 0;
                        }
                    }

                    return true;
                }
            }

            return false;
        }

        const bool Text::PageDown(TextIterator& it, const OutputDevice* const PCLI_Out) const
        {
            // Try to skeep as many lines as the screen is high -1 for the status line.
            for (unsigned int ui = 0; ui < it.m_uiPageHeight; ui ++)
            {
                if (! LineDown(it, PCLI_Out))
                {
                    // If the display could not be moved one single line down, return false.
                    return (ui > 0);
                }
            }

            return true;
        }

        void Text::End(TextIterator& it, const OutputDevice* const PCLI_Out) const
        {
            // Keep moving one line down until it stucks.
            while (LineDown(it, PCLI_Out));
        }

        void Text::PrintPage(TextIterator& it, const OutputDevice& CLI_Out, const bool B_FillPageWithBlankLines) const
        {
            // Align current bottom position with current top position.
            it.m_tkBottomLine = it.m_tkTopLine;
            it.m_uiBottomChar = it.m_uiTopChar;
            it.m_bBottomIsUpToDate = true; // Not already true, but the flag is set right away.

            // Then let current bottom position move one page down while printing the page.
            unsigned int ui_LineCount = 0;
            for (   ;
                    m_tkLines.IsValid(it.m_tkBottomLine)    // While there are still lines to display
                    && (ui_LineCount < it.m_uiPageHeight);  // Fill the page
                    ui_LineCount ++)
            {
                PrintBottomLine(it, CLI_Out);
            }
            // Possibly finish with blank lines.
            if (B_FillPageWithBlankLines)
            {
                for (; ui_LineCount < it.m_uiPageHeight; ui_LineCount ++)
                {
                    CLI_Out << endl;
                }
            }
        }

        void Text::PrintBottomLine(TextIterator& it, const OutputDevice& CLI_Out) const
        {
            for (   unsigned int ui_CharCount = 0;
                    m_tkLines.IsValid(it.m_tkBottomLine);  // While there are still lines to display.
                    ui_CharCount ++)
            {
                const tk::String tk_Line = m_tkLines.GetAt(it.m_tkBottomLine);

                if (it.m_uiBottomChar >= tk_Line.GetLength())
                {
                    // End of line.
                    CLI_ASSERT(it.m_cliScreenInfo.GetSafeWidth() > 0);
                    if ((! it.m_cliScreenInfo.GetbWrapLines())
                        // When line wrapping is on, a line ending at the right end of the screen should not cause an empty line below.
                        // Thus, print a carriage return only in the following cases:
                        || (it.m_uiBottomChar == 0)                                         // Regular empty line.
                        || (it.m_cliScreenInfo.GetSafeWidth() <= 0)                         // (Avoid invalid modulo, should never occur however)
                        || (it.m_uiBottomChar % it.m_cliScreenInfo.GetSafeWidth() != 0))    // Line not ending at the right end the screen.
                    {
                        CLI_Out << endl;
                    }
                    m_tkLines.MoveNext(it.m_tkBottomLine);
                    it.m_uiBottomChar = 0;
                    break;
                }
                else if (ui_CharCount >= it.m_cliScreenInfo.GetSafeWidth())
                {
                    // Out of line.
                    if (! it.m_cliScreenInfo.GetbWrapLines())
                    {
                        CLI_Out << endl;
                    }
                    break;
                }
                else
                {
                    // Regular character display.
                    CLI_Out << tk_Line.GetChar(it.m_uiBottomChar);
                    it.m_uiBottomChar ++;
                }
            }
        }


        static const tk::Queue<tk::String>::Iterator InitIterator(void)
        {
            tk::Queue<tk::String> it(0);
            return it.GetIterator();
        }

        TextIterator::TextIterator(const OutputDevice::ScreenInfo& CLI_ScreenInfo, const unsigned int UI_PageHeight)
          : m_cliScreenInfo(CLI_ScreenInfo), m_uiPageHeight(UI_PageHeight),
            m_tkTopLine(InitIterator()), m_uiTopChar(0),
            m_tkBottomLine(InitIterator()), m_uiBottomChar(0), m_bBottomIsUpToDate(false)
        {
        }

        TextIterator::TextIterator(const TextIterator& it)
          : m_cliScreenInfo(it.m_cliScreenInfo), m_uiPageHeight(it.m_uiPageHeight),
            m_tkTopLine(it.m_tkTopLine), m_uiTopChar(it.m_uiTopChar),
            m_tkBottomLine(it.m_tkBottomLine), m_uiBottomChar(it.m_uiBottomChar), m_bBottomIsUpToDate(it.m_bBottomIsUpToDate)
        {
        }

    CLI_NS_END(ui)

CLI_NS_END(cli)

