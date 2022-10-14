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

#include "constraints.h"
#include "cli/ui_int.h"
#include "cli/param_int.h"
#include "cli/string_device.h"
#include "cli/shell.h"
#include "command_line_edition.h"


CLI_NS_BEGIN(cli)

    CLI_NS_BEGIN(ui)

        //! @brief Integer to string conversion.
        //! @return String result.
        static const tk::String Int2Str(
                const int I_Int     //!< Integer to convert.
                )
        {
            StringDevice cli_Str(MAX_WORD_LENGTH, false);
            cli_Str << I_Int;
            return cli_Str.GetString();
        }


        Int::Int(const int I_DefaultValue, const int I_MinValue, const int I_MaxValue)
          : Line(Int2Str(I_DefaultValue), -1, -1),
            m_iDefaultValue(I_DefaultValue), m_iMinValue(I_MinValue), m_iMaxValue(I_MaxValue)
        {
        }

        Int::Int(ExecutionContext& CLI_ParentContext, const int I_DefaultValue, const int I_MinValue, const int I_MaxValue)
          : Line(CLI_ParentContext, Int2Str(I_DefaultValue), -1, -1),
            m_iDefaultValue(I_DefaultValue), m_iMinValue(I_MinValue), m_iMaxValue(I_MaxValue)
        {
        }

        Int::~Int(void)
        {
        }

        const int Int::GetInt(void) const
        {
            Help cli_Help; ParamInt cli_Int(cli_Help);
            if (cli_Int.SetstrValue(Line::GetLine()))
            {
                return (int) cli_Int;
            }

            return m_iDefaultValue;
        }

        void Int::ResetToDefault(void)
        {
            Line::ResetToDefault();
        }

        void Int::OnKey(const KEY E_KeyCode)
        {
            switch (E_KeyCode)
            {
            case NULL_KEY:
                EndControl(false);
                break;
            case KEY_UP:
                if (GetInt() < m_iMinValue)
                {
                    // Completely out of bounds: return to min value.
                    Beep();
                    Line::SetLine(Int2Str(m_iMinValue), false, false);
                }
                else if (GetInt() < m_iMaxValue)
                {
                    Line::SetLine(Int2Str(GetInt() + 1), false, false);
                }
                else
                {
                    // Upper bound already reached: ensure max value.
                    Beep();
                    Line::SetLine(Int2Str(m_iMaxValue), false, false);
                }
                break;
            case KEY_DOWN:
                if (GetInt() > m_iMaxValue)
                {
                    // Completely out of bounds: return to max value.
                    Beep();
                    Line::SetLine(Int2Str(m_iMaxValue), false, false);
                }
                else if (GetInt() > m_iMinValue)
                {
                    Line::SetLine(Int2Str(GetInt() - 1), false, false);
                }
                else
                {
                    // Lower bound already reached: ensure min value.
                    Beep();
                    Line::SetLine(Int2Str(m_iMinValue), false, false);
                }
                break;
            case PAGE_UP:
                if (GetInt() >= m_iMaxValue)
                {
                    // Upper bound already reached.
                    Beep();
                }
                // Ensure max value.
                Line::SetLine(Int2Str(m_iMaxValue), false, false);
                break;
            case PAGE_DOWN:
                if (GetInt() <= m_iMinValue)
                {
                    // Lower bound already reached.
                    Beep();
                }
                // Ensure min value.
                Line::SetLine(Int2Str(m_iMinValue), false, false);
                break;
            case ENTER:
                if ((GetInt() >= m_iMinValue) && (GetInt() <= m_iMaxValue))
                {
                    // Reprint understood value to avoid confusions from the user.
                    Line::SetLine(Int2Str(GetInt()), true, false);
                    EndControl(true);
                }
                else
                {
                    Beep();
                }
                break;
            default:
                Line::OnKey(E_KeyCode);
                break;
            }
        }

    CLI_NS_END(ui)

CLI_NS_END(cli)

