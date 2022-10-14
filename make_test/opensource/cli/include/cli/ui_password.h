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


//! @file
//! @author Alexis Royer
//! @brief Password class definition.

#ifndef _CLI_UI_PASSWORD_H_
#define _CLI_UI_PASSWORD_H_

#include "cli/ui.h"


CLI_NS_BEGIN(cli)

    // Forward declarations.
    class CmdLineEdition;


    CLI_NS_BEGIN(ui)

        //! @brief Password user interface object.
        class Password : public UI
        {
        public:
            //! @brief Top execution context constructor.
            explicit Password(
                const bool B_DisplayStars,              //!< true in order to display '*' for each character, false for no display at all.
                const int I_MinPasswordLength,          //!< Minimum password length required. -1 if not set.
                const int I_MaxPasswordLength           //!< Maximum password length required. -1 if not set.
                );

            //! @brief Child execution context constructor.
            explicit Password(
                ExecutionContext& CLI_ParentContext,    //!< Parent execution context.
                const bool B_DisplayStars,              //!< true in order to display '*' for each character, false for no display at all.
                const int I_MinPasswordLength,          //!< Minimum password length required. -1 if not set.
                const int I_MaxPasswordLength           //!< Maximum password length required. -1 if not set.
                );

            //! @brief Destructor.
            virtual ~Password(void);

        private:
            //! @brief No default constructor.
            explicit Password(void);
            //! @brief No copy constructor.
            Password(const Password&);
            //! @brief No assignment operator.
            Password& operator=(const Password&);

        public:
            //! @brief Password retrieval.
            //! @return Password entered by the user.
            const tk::String GetPassword(void) const;

        protected:
            // cli::ui::UI interface implementation.
            virtual void Reset(void);
            virtual void ResetToDefault(void);
        public:
            // Inherit doxygen comments from cli::ExecutionContext interface documentation.
            virtual void OnKey(const KEY E_KeyCode);

        private:
            const bool m_bDisplayStars;         //!< Star display feature.
            const int m_iMinPasswordLength;     //!< Minimum line length required.
            const int m_iMaxPasswordLength;     //!< Maximum line length required.
            CmdLineEdition& m_cliPassword;      //!< Password in edition: receives the actual password characters.
            CmdLineEdition& m_cliLine;          //!< Line in edition: displays star characters or nothing.
        };

    CLI_NS_END(ui)

CLI_NS_END(cli)

#endif // _CLI_UI_PASSWORD_H_
