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
#include "cli/ui_more.h"
#include "cli/shell.h"
#include "cli/string_device.h"
#include "ui_text.h"
#include "command_line_edition.h"


CLI_NS_BEGIN(cli)

    CLI_NS_BEGIN(ui)

        More::More(const unsigned int UI_MaxLines, const unsigned int UI_MaxLineLength)
          : UI(),
            m_uiText(* new Text(UI_MaxLines, UI_MaxLineLength)), m_puiTextIt(NULL),
            m_cliMoreLine(* new CmdLineEdition())
        {
        }

        More::More(ExecutionContext& CLI_ParentContext, const unsigned int UI_MaxLines, const unsigned int UI_MaxLineLength)
          : UI(CLI_ParentContext),
            m_uiText(* new Text(UI_MaxLines, UI_MaxLineLength)), m_puiTextIt(NULL),
            m_cliMoreLine(* new CmdLineEdition())
        {
        }

        More::~More(void)
        {
            delete & m_uiText;
            if (m_puiTextIt != NULL)
            {
                delete m_puiTextIt;
                m_puiTextIt = NULL;
            }
            delete & m_cliMoreLine;
        }

        const OutputDevice& More::GetText(void)
        {
            return m_uiText;
        }

        void More::Reset(void)
        {
            // Nothing to do.
        }

        void More::ResetToDefault(void)
        {
            // Very first display.
            const OutputDevice::ScreenInfo cli_ScreenInfo = GetStream(OUTPUT_STREAM).GetScreenInfo();
            if (m_puiTextIt == NULL)
            {
                m_puiTextIt = new TextIterator(cli_ScreenInfo, cli_ScreenInfo.GetSafeHeight() - 1);
            }
            CLI_ASSERT(m_puiTextIt != NULL);
            if (m_puiTextIt != NULL)
            {
                m_uiText.Begin(*m_puiTextIt);
                const StringDevice cli_Out(cli_ScreenInfo.GetSafeHeight() * (cli_ScreenInfo.GetSafeWidth() + 1), false);
                m_uiText.PrintPage(*m_puiTextIt, cli_Out, false);
                GetStream(OUTPUT_STREAM) << cli_Out.GetString();
            }
            ShowMoreMessage();
        }

        void More::OnKey(const KEY E_KeyCode)
        {
            // Ensure m_puiTextIt is valid.
            CLI_ASSERT(m_puiTextIt != NULL);
            if (m_puiTextIt == NULL) { Quit(); }
            else {
                switch (E_KeyCode)
                {
                case KEY_END:
                    HideMoreMessage();
                    // We are already at the bottom of a page, no need to optimize display so far, let it progress fluently.
                    // Optimize line by line at least.
                    for (bool b_LineDown = true; b_LineDown; )
                    {
                        const OutputDevice::ScreenInfo cli_ScreenInfo = GetStream(OUTPUT_STREAM).GetScreenInfo();
                        const StringDevice cli_Out(cli_ScreenInfo.GetSafeHeight() * (cli_ScreenInfo.GetSafeWidth() + 1), false);
                        b_LineDown = m_uiText.LineDown(*m_puiTextIt, & cli_Out);
                        GetStream(OUTPUT_STREAM) << cli_Out.GetString();
                    }
                    ShowMoreMessage();
                    break;
                case PAGE_DOWN:
                case SPACE:
                    // Print one more page
                    HideMoreMessage();
                    do {
                        // Output lines to a buffer output device.
                        const OutputDevice::ScreenInfo cli_ScreenInfo = GetStream(OUTPUT_STREAM).GetScreenInfo();
                        const StringDevice cli_Out(cli_ScreenInfo.GetSafeHeight() * (cli_ScreenInfo.GetSafeWidth() + 1), false);
                        m_uiText.PageDown(*m_puiTextIt, & cli_Out);
                        // Display in one call in order to optimize display.
                        GetStream(OUTPUT_STREAM) << cli_Out.GetString();
                    } while(0);
                    ShowMoreMessage();
                    break;
                case KEY_DOWN:
                case ENTER:
                    // Print one more line
                    HideMoreMessage();
                    m_uiText.LineDown(*m_puiTextIt, & GetStream(OUTPUT_STREAM));
                    ShowMoreMessage();
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
                    // Non managed character. Beep.
                    Beep();
                    break;
                }
            }
        }

        void More::ShowMoreMessage(void)
        {
            CLI_ASSERT(m_puiTextIt != NULL);
            if (m_puiTextIt != NULL)
            {
                m_cliMoreLine.Reset();
                TextIterator tmp(*m_puiTextIt);
                if (m_uiText.LineDown(tmp, NULL))
                {
                    // Still lines to display.
                    const ResourceString cli_MoreMessage = ResourceString()
                        .SetString(ResourceString::LANG_EN, "--- More ---")
                        .SetString(ResourceString::LANG_FR, "--- Plus ---");
                    m_cliMoreLine.Put(GetStream(OUTPUT_STREAM), cli_MoreMessage.GetString(GetLang()));
                    return;
                }
            }

            // If the input text is done (or something wrong occurred), terminate the UI execution.
            Quit();
        }

        void More::HideMoreMessage(void)
        {
            m_cliMoreLine.CleanAll(GetStream(OUTPUT_STREAM));
        }

        void More::Quit(void)
        {
            HideMoreMessage();
            EndControl(true);
        }

    CLI_NS_END(ui)

CLI_NS_END(cli)

