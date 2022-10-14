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

#include "cli/ui_password.h"
#include "cli/shell.h"
#include "command_line_edition.h"


CLI_NS_BEGIN(cli)

    CLI_NS_BEGIN(ui)

        Password::Password(const bool B_DisplayStars, const int I_MinPasswordLength, const int I_MaxPasswordLength)
          : UI(),
            m_bDisplayStars(B_DisplayStars), m_iMinPasswordLength(I_MinPasswordLength), m_iMaxPasswordLength(I_MaxPasswordLength),
            m_cliPassword(* new CmdLineEdition()), m_cliLine(* new CmdLineEdition())
        {
        }

        Password::Password(ExecutionContext& CLI_ParentContext, const bool B_DisplayStars, const int I_MinPasswordLength, const int I_MaxPasswordLength)
          : UI(CLI_ParentContext),
            m_bDisplayStars(B_DisplayStars), m_iMinPasswordLength(I_MinPasswordLength), m_iMaxPasswordLength(I_MaxPasswordLength),
            m_cliPassword(* new CmdLineEdition()), m_cliLine(* new CmdLineEdition())
        {
        }

        Password::~Password(void)
        {
            delete & m_cliPassword;
            delete & m_cliLine;
        }

        const tk::String Password::GetPassword(void) const
        {
            return m_cliPassword.GetLine();
        }

        void Password::Reset(void)
        {
            m_cliPassword.Reset();
            m_cliLine.Reset();
        }

        void Password::ResetToDefault(void)
        {
            const OutputDevice& ECHO = GetStream(ECHO_STREAM);

            m_cliPassword.Reset();
            m_cliLine.CleanAll(ECHO);
        }

        void Password::OnKey(const KEY E_KeyCode)
        {
            const OutputDevice& NOECHO = OutputDevice::GetNullDevice();
            const OutputDevice& ECHO = GetStream(ECHO_STREAM);

            switch (E_KeyCode)
            {
            case NULL_KEY:
                EndControl(false);
                break;
            //case KEY_UP:
            //case KEY_DOWN:
            //case PAGE_UP:
            //case PAGE_DOWN:
            case KEY_BEGIN:
                m_cliPassword.Home(NOECHO);
                if (m_bDisplayStars)
                {
                    m_cliLine.Home(ECHO);
                }
                break;
            case KEY_END:
                m_cliPassword.End(NOECHO);
                if (m_bDisplayStars)
                {
                    m_cliLine.End(ECHO);
                }
                break;
            case KEY_LEFT:
                m_cliPassword.MoveCursor(NOECHO, -1);
                if (! m_cliLine.GetLeft().IsEmpty())
                {
                    m_cliLine.MoveCursor(ECHO, -1);
                }
                else
                {
                    if (m_bDisplayStars) {
                        Beep();
                    }
                }
                break;
            case KEY_RIGHT:
                m_cliPassword.MoveCursor(NOECHO, 1);
                if (! m_cliLine.GetRight().IsEmpty())
                {
                    m_cliLine.MoveCursor(ECHO, 1);
                }
                else
                {
                    if (m_bDisplayStars)
                    {
                        Beep();
                    }
                }
                break;
            case BACKSPACE:
                m_cliPassword.Delete(NOECHO, -1);
                if (! m_cliLine.GetLeft().IsEmpty())
                {
                    m_cliLine.Delete(ECHO, -1);
                }
                else
                {
                    if (m_bDisplayStars)
                    {
                        Beep();
                    }
                }
                break;
            case DELETE:
                m_cliPassword.Delete(NOECHO, 1);
                if (! m_cliLine.GetRight().IsEmpty())
                {
                    m_cliLine.Delete(ECHO, 1);
                }
                else
                {
                    if (m_bDisplayStars)
                    {
                        Beep();
                    }
                }
                break;
            case ENTER:
                if (((m_iMinPasswordLength < 0) || (m_cliPassword.GetLine().GetLength() >= (unsigned int) m_iMinPasswordLength))
                    && ((m_iMaxPasswordLength < 0) || (m_cliPassword.GetLine().GetLength() <= (unsigned int) m_iMaxPasswordLength)))
                {
                    m_cliLine.NextLine(ECHO);
                    EndControl(true);
                }
                else
                {
                    Beep();
                }
                break;
            case BREAK:
            case ESCAPE:
            case LOGOUT:
                EndControl(false);
                break;
            case TAB:
                Beep();
                break;
            case CLS:
                m_cliPassword.Reset();
                m_cliLine.CleanAll(ECHO);
                break;

            case QUESTION:
            case KEY_a: case KEY_aacute: case KEY_agrave: case KEY_auml: case KEY_acirc:
            case KEY_b: case KEY_c: case KEY_ccedil: case KEY_d:
            case KEY_e: case KEY_eacute: case KEY_egrave: case KEY_euml: case KEY_ecirc:
            case KEY_f: case KEY_g: case KEY_h:
            case KEY_i: case KEY_iacute: case KEY_igrave: case KEY_iuml: case KEY_icirc:
            case KEY_j: case KEY_k: case KEY_l: case KEY_m: case KEY_n:
            case KEY_o: case KEY_oacute: case KEY_ograve: case KEY_ouml: case KEY_ocirc:
            case KEY_p: case KEY_q: case KEY_r: case KEY_s: case KEY_t:
            case KEY_u: case KEY_uacute: case KEY_ugrave: case KEY_uuml: case KEY_ucirc:
            case KEY_v: case KEY_w: case KEY_x: case KEY_y: case KEY_z:

            case KEY_A: case KEY_B: case KEY_C: case KEY_D: case KEY_E: case KEY_F:
            case KEY_G: case KEY_H: case KEY_I: case KEY_J: case KEY_K: case KEY_L:
            case KEY_M: case KEY_N: case KEY_O: case KEY_P: case KEY_Q: case KEY_R:
            case KEY_S: case KEY_T: case KEY_U: case KEY_V: case KEY_W: case KEY_X:
            case KEY_Y: case KEY_Z:

            case KEY_0: case KEY_1: case KEY_2: case KEY_3: case KEY_4: case KEY_5:
            case KEY_6: case KEY_7: case KEY_8: case KEY_9:

            case PLUS:
            case MINUS:
            case STAR:
            case SLASH:
            case LOWER_THAN:
            case GREATER_THAN:
            case EQUAL:
            case PERCENT:

            case SPACE:
            case UNDERSCORE:
            case AROBASE:
            case SHARP:
            case AMPERCENT:
            case DOLLAR:
            case BACKSLASH:
            case PIPE:
            case TILDE:
            case SQUARE:
            case EURO:
            case POUND:
            case MICRO:
            case PARAGRAPH:

            case EXCLAMATION:
            case COLUMN:
            case DOT:
            case COMA:
            case SEMI_COLUMN:
            case QUOTE:
            case DOUBLE_QUOTE:

            case OPENING_BRACE:
            case CLOSING_BRACE:
            case OPENING_CURLY_BRACE:
            case CLOSING_CURLY_BRACE:
            case OPENING_BRACKET:
            case CLOSING_BRACKET:
                if ((m_iMaxPasswordLength < 0) || (m_cliPassword.GetLine().GetLength() < (unsigned int) m_iMaxPasswordLength))
                {
                    m_cliPassword.Put(NOECHO, E_KeyCode);
                    if (m_bDisplayStars)
                    {
                        m_cliLine.Put(ECHO, STAR);
                    }
                }
                else
                {
                    if (m_bDisplayStars)
                    {
                        Beep();
                    }
                }
                break;

            case INSERT:
                m_cliPassword.SetInsertMode(! m_cliPassword.GetInsertMode());
                m_cliLine.SetInsertMode(! m_cliLine.GetInsertMode());
                break;

            default:
                // Non managed character. Just ignore.
                break;
            }
        }

    CLI_NS_END(ui)

CLI_NS_END(cli)

