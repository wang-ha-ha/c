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
#include "cli/ui_float.h"
#include "cli/param_float.h"
#include "cli/string_device.h"
#include "cli/shell.h"
#include "command_line_edition.h"


CLI_NS_BEGIN(cli)

    CLI_NS_BEGIN(ui)

        //! @brief Float to string conversion.
        //! @return String result.
        static const tk::String Float2Str(
                const double D_Float    //!< Float to convert.
                )
        {
            StringDevice cli_Str(MAX_WORD_LENGTH, false);
            cli_Str << D_Float;
            return cli_Str.GetString();
        }


        Float::Float(const double D_DefaultValue, const double D_MinValue, const double D_MaxValue)
          : Line(Float2Str(D_DefaultValue), -1, -1),
            m_dDefaultValue(D_DefaultValue), m_dMinValue(D_MinValue), m_dMaxValue(D_MaxValue)
        {
        }

        Float::Float(ExecutionContext& CLI_ParentContext, const double D_DefaultValue, const double D_MinValue, const double D_MaxValue)
          : Line(CLI_ParentContext, Float2Str(D_DefaultValue), -1, -1),
            m_dDefaultValue(D_DefaultValue), m_dMinValue(D_MinValue), m_dMaxValue(D_MaxValue)
        {
        }

        Float::~Float(void)
        {
        }

        const double Float::GetFloat(void) const
        {
            Help cli_Help; ParamFloat cli_Float(cli_Help);
            if (cli_Float.SetstrValue(Line::GetLine()))
            {
                return (double) cli_Float;
            }

            return m_dDefaultValue;
        }

        void Float::ResetToDefault(void)
        {
            Line::ResetToDefault();
        }

        void Float::OnKey(const KEY E_KeyCode)
        {
            switch (E_KeyCode)
            {
            case NULL_KEY:
                EndControl(false);
                break;
            //case KEY_UP:
            //case KEY_DOWN:
            case PAGE_UP:
                if (GetFloat() >= m_dMaxValue)
                {
                    // Upper bound already reached
                    Beep();
                }
                // Ensure max value.
                Line::SetLine(Float2Str(m_dMaxValue), false, false);
                break;
            case PAGE_DOWN:
                if (GetFloat() <= m_dMinValue)
                {
                    // Lower bound already reached
                    Beep();
                }
                // Ensure min value.
                Line::SetLine(Float2Str(m_dMinValue), false, false);
                break;
            case ENTER:
                if ((GetFloat() >= m_dMinValue) && (GetFloat() <= m_dMaxValue))
                {
                    // Reprint understood value to avoid confusions from the user.
                    double d_Value = GetFloat();
                    Line::SetLine(Float2Str(d_Value), true, false);
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

