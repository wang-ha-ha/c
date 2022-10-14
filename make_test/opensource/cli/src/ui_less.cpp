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
#include "cli/ui_less.h"
#include "cli/shell.h"
#include "cli/string_device.h"
#include "ui_text.h"
#include "command_line_edition.h"


CLI_NS_BEGIN(cli)

    CLI_NS_BEGIN(ui)

        Less::Less(const unsigned int UI_MaxLines, const unsigned int UI_MaxLineLength)
          : UI(),
            m_uiText(* new Text(UI_MaxLines, UI_MaxLineLength)), m_puiTextIt(NULL),
            m_cliLessLine(* new CmdLineEdition())
        {
        }

        Less::Less(ExecutionContext& CLI_ParentContext, const unsigned int UI_MaxLines, const unsigned int UI_MaxLineLength)
          : UI(CLI_ParentContext),
            m_uiText(* new Text(UI_MaxLines, UI_MaxLineLength)), m_puiTextIt(NULL),
            m_cliLessLine(* new CmdLineEdition())
        {
        }

        Less::~Less(void)
        {
            delete & m_uiText;
            if (m_puiTextIt != NULL)
            {
                delete m_puiTextIt;
                m_puiTextIt = NULL;
            }
            delete & m_cliLessLine;
        }

        const OutputDevice& Less::GetText(void)
        {
            return m_uiText;
        }

        void Less::Reset(void)
        {
            // Nothing to do.
        }

        void Less::ResetToDefault(void)
        {
            // Very first display.
            if (m_puiTextIt == NULL)
            {
                const OutputDevice::ScreenInfo cli_ScreenInfo = GetStream(OUTPUT_STREAM).GetScreenInfo();
                m_puiTextIt = new TextIterator(cli_ScreenInfo, cli_ScreenInfo.GetSafeHeight() - 1);
            }
            CLI_ASSERT(m_puiTextIt != NULL);
            if (m_puiTextIt != NULL) { m_uiText.Begin(*m_puiTextIt); }
            PrintScreen();
        }

        void Less::OnKey(const KEY E_KeyCode)
        {
            // Ensure m_puiTextIt is valid.
            CLI_ASSERT(m_puiTextIt != NULL);
            if (m_puiTextIt == NULL) { Quit(); }
            else {
                switch (E_KeyCode)
                {
                case KEY_BEGIN:
                    m_uiText.Begin(*m_puiTextIt);
                    PrintScreen();
                    break;
                case PAGE_UP:
                    if (m_uiText.PageUp(*m_puiTextIt)) PrintScreen();
                    else Beep();
                    break;
                case KEY_UP:
                    if (m_uiText.LineUp(*m_puiTextIt)) PrintScreen();
                    else Beep();
                    break;
                case KEY_DOWN:
                case ENTER:
                    if (m_uiText.LineDown(*m_puiTextIt, NULL)) PrintScreen();
                    else Beep();
                    break;
                case PAGE_DOWN:
                case SPACE:
                    if (m_uiText.PageDown(*m_puiTextIt, NULL)) PrintScreen();
                    else Beep();
                    break;
                case KEY_END:
                    m_uiText.End(*m_puiTextIt, NULL);
                    PrintScreen();
                    break;
                case KEY_q:
                case KEY_Q:
                case ESCAPE:
                case BREAK:
                case LOGOUT:
                case NULL_KEY:
                    // Stop display
                    Quit();
                    break;
                default:
                    // Non managed character: beep.
                    Beep();
                    break;
                }
            }
        }

        void Less::PrintScreen(void)
        {
            const OutputDevice::ScreenInfo cli_ScreenInfo = GetStream(OUTPUT_STREAM).GetScreenInfo();

            // Then let current bottom position move one page down while printing the page.
            const StringDevice cli_Out(cli_ScreenInfo.GetSafeHeight() * (cli_ScreenInfo.GetSafeWidth() + 1), false);
            CLI_ASSERT(m_puiTextIt != NULL);
            if (m_puiTextIt != NULL) { m_uiText.PrintPage(*m_puiTextIt, cli_Out, true); }

            // Eventually clean out the screen and print out the computed string.
            m_cliLessLine.Reset();
            GetStream(OUTPUT_STREAM).CleanScreen();
            GetStream(OUTPUT_STREAM) << cli_Out.GetString();
            m_cliLessLine.Put(GetStream(OUTPUT_STREAM), COLUMN);
        }

        void Less::Quit(void)
        {
            m_cliLessLine.CleanAll(GetStream(OUTPUT_STREAM));
            EndControl(true);
        }

    CLI_NS_END(ui)

CLI_NS_END(cli)

