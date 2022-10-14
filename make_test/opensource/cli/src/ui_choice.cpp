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

#include "cli/ui_choice.h"
#include "cli/shell.h"
#include "command_line_edition.h"


CLI_NS_BEGIN(cli)

    CLI_NS_BEGIN(ui)

        //! @brief Choice to string conversion.
        //! @return Resource string at the given position in the list. Empty string when an error occurred.
        static const ResourceString Choice2Str(
                const int I_Choice,                             //!< Choice index to convert.
                const tk::Queue<ResourceString>& TK_Choices     //!< Choice list.
                )
        {
            // Loop over the choice list until the number of elements to skip has been reached.
            tk::Queue<ResourceString>::Iterator it =  TK_Choices.GetIterator();
            for (   int i_ElementsToSkip = I_Choice;
                    (i_ElementsToSkip >= 0) && TK_Choices.IsValid(it);
                    i_ElementsToSkip --)
            {
                if (i_ElementsToSkip == 0)
                {
                    return TK_Choices.GetAt(it);
                }
                else
                {
                    TK_Choices.MoveNext(it);
                }
            }

            // Default to an empty string.
            return ResourceString();
        }

        // Unlike ui::Int and ui::Float implementations, ui::Line default value is not used for ui::Choice.
        // Default value management will rather rely on ui::Line::ResetToDefault() overriding.

        Choice::Choice(const int I_DefaultChoice, const tk::Queue<ResourceString>& TK_Choices)
          : Line(tk::String(10), -1, -1),
            m_iDefaultChoice(I_DefaultChoice), m_tkChoices(TK_Choices), m_eLang(ResourceString::LANG_EN)
        {
        }

        Choice::Choice(ExecutionContext& CLI_ParentContext, const int I_DefaultChoice, const tk::Queue<ResourceString>& TK_Choices)
          : Line(CLI_ParentContext, tk::String(10), -1, -1),
            m_iDefaultChoice(I_DefaultChoice), m_tkChoices(TK_Choices), m_eLang(ResourceString::LANG_EN)
        {
        }

        Choice::~Choice(void)
        {
        }

        const int Choice::GetChoice(void) const
        {
            const tk::String str_Line = Line::GetLine();
            int i_Choice = 0;
            tk::Queue<int> tk_Near(10);
            for (   tk::Queue<ResourceString>::Iterator it = m_tkChoices.GetIterator();
                    m_tkChoices.IsValid(it);
                    m_tkChoices.MoveNext(it), i_Choice++)
            {
                const tk::String str_Choice = m_tkChoices.GetAt(it).GetString(m_eLang);

                // Check whether the line matches the choice exactly.
                if (str_Line == str_Choice)
                {
                    return i_Choice;
                }

                // Check whether the line starts like the choice (case insensitive).
                if (str_Line.ToUpper() == str_Choice.SubString(0, str_Line.GetLength()).ToUpper())
                {
                    tk_Near.AddTail(i_Choice);
                }
            }

            // If only one near result, then this one is the choice.
            if (tk_Near.GetCount() == 1)
            {
                return tk_Near.GetHead();
            }

            // No matching choice.
            return -1;
        }

        const ResourceString Choice::GetstrChoice(void) const
        {
            // First retrieve the choice enterd in its index form.
            int i_Choice = GetChoice();
            if (i_Choice >= 0)
            {
                return Choice2Str(i_Choice, m_tkChoices);
            }
            else
            {
                return ResourceString();
            }
        }

        void Choice::ResetToDefault(void)
        {
            // Remember the shell language setting
            // so that post-execution calls to GetChoice() do not need a shell instance reference anymore.
            m_eLang = GetLang();

            Line::SetLine(Choice2Str(m_iDefaultChoice, m_tkChoices).GetString(m_eLang), false, true);
        }

        void Choice::OnKey(const KEY E_KeyCode)
        {
            switch (E_KeyCode)
            {
            case NULL_KEY:
                EndControl(false);
                break;
            case KEY_UP:
                MoveChoice(-1, 1);
                break;
            case KEY_DOWN:
                MoveChoice(1, 1);
                break;
            case PAGE_UP:
                MoveChoice(-1, m_tkChoices.GetCount() / 10);
                break;
            case PAGE_DOWN:
                MoveChoice(1, m_tkChoices.GetCount() / 10);
                break;
            case ENTER:
                if (GetChoice() >= 0)
                {
                    Line::SetLine(GetstrChoice().GetString(m_eLang), true, true);
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

        void Choice::MoveChoice(const int I_Way, const unsigned int UI_Increment)
        {
            const int i_Increment = I_Way * (int) ((UI_Increment == 0) ? 1 : UI_Increment);
            const int i_Choice = GetChoice();

            if (i_Choice < 0)
            {
                // Undefined choice: restore default.
                Beep();
                ResetToDefault();
            }
            else if (i_Choice + i_Increment < 0)
            {
                // Top of choice list.
                // Possibly beep.
                if (i_Choice <= 0)
                {
                    Beep();
                }
                // Reprint.
                Line::SetLine(Choice2Str(0, m_tkChoices).GetString(m_eLang), false, true);
            }
            else if (i_Choice + i_Increment >= (int) m_tkChoices.GetCount())
            {
                // Bottom of choice list.
                // Possibly beep.
                if (i_Choice >= ((int) m_tkChoices.GetCount()) - 1)
                {
                    Beep();
                }
                // Reprint.
                Line::SetLine(Choice2Str(m_tkChoices.GetCount() - 1, m_tkChoices).GetString(m_eLang), false, true);
            }
            else
            {
                // Move to next element.
                Line::SetLine(Choice2Str(GetChoice() + i_Increment, m_tkChoices).GetString(m_eLang), false, true);
            }
        }

    CLI_NS_END(ui)

CLI_NS_END(cli)

