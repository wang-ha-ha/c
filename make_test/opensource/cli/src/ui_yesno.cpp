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

#include "cli/ui_yesno.h"
#include "cli/shell.h"


CLI_NS_BEGIN(cli)

    CLI_NS_BEGIN(ui)

        //! @brief Yes/no choice singleton.
        //! @return Yes/no only one object.
        static const tk::Queue<ResourceString>& GetYesNoChoice(void)
        {
            static tk::Queue<ResourceString> tk_YesNo(2);
            if (tk_YesNo.IsEmpty())
            {
                tk_YesNo.AddTail(ResourceString()
                    .SetString(ResourceString::LANG_EN, "Yes")
                    .SetString(ResourceString::LANG_FR, "Oui")
                );
                tk_YesNo.AddTail(ResourceString()
                    .SetString(ResourceString::LANG_EN, "No")
                    .SetString(ResourceString::LANG_FR, "Non")
                );
            }
            return tk_YesNo;
        }


        YesNo::YesNo(const bool B_DefaultAnswer)
          : Choice(B_DefaultAnswer ? 0 : 1, GetYesNoChoice())
        {
        }

        YesNo::YesNo(ExecutionContext& CLI_ParentContext, const bool B_DefaultAnswer)
          : Choice(CLI_ParentContext, B_DefaultAnswer ? 0 : 1, GetYesNoChoice())
        {
        }

        YesNo::~YesNo(void)
        {
        }

        const bool YesNo::GetYesNo(void) const
        {
            if (Choice::GetChoice() == 0)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        void YesNo::OnKey(const KEY E_KeyCode)
        {
            Choice::OnKey(E_KeyCode);
        }

    CLI_NS_END(ui)

CLI_NS_END(cli)

