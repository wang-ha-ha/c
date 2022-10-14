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
//! @brief Endl class definition.

#ifndef _CLI_ENDL_H_
#define _CLI_ENDL_H_

#include "cli/namespace.h"
#include "cli/element.h"


CLI_NS_BEGIN(cli)

    // Forward declarations.
    class Menu;
    class MenuRef;
    class Help;


    //! @brief End of command line element.
    //!
    //! CLI element used to terminate a command line.
    //! A menu reference can be optionally attached.
    class Endl : public Element
    {
    public:
        //! @brief Constructor.
        explicit Endl(
            const Help& CLI_Help    //!< Help.
            );

        //! @brief Destructor.
        virtual ~Endl(void);

    private:
        //! @brief No default constructor.
        explicit Endl(void);
        //! @brief No copy constructor.
        Endl(const Endl&);
        //! @brief No assignment operator.
        Endl& operator=(const Endl&);

    public:
        //! @brief Attaches the optional menu.
        //! @return The menu object reference.
        //!
        //! Stores the menu object reference
        //! and gives it to the corresponding CLI object for ownership.
        Menu& SetMenu(
            Menu* const PCLI_Menu           //!< Newly created menu object.
            );

        //! @brief Attaches the optional menu.
        //! @return The menu object reference.
        MenuRef& SetMenuRef(
            MenuRef* const PCLI_MenuRef     //!< Menu reference.
            );

        //! @brief Optional menu reference accessor.
        //! @return Menu reference if set, NULL otherwise.
        const MenuRef* const GetMenuRef(void) const;

    private:
        //! Optional menu reference.
        const MenuRef* m_pcliMenuRef;
    };

CLI_NS_END(cli)

#endif // _CLI_ENDL_H_

