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
//! @brief SyntaxTag and SyntaxRef classes definition.

#ifndef _CLI_SYNTAX_TAG_H_
#define _CLI_SYNTAX_TAG_H_

#include "cli/namespace.h"
#include "cli/syntax_node.h"
#include "cli/tk.h"


CLI_NS_BEGIN(cli)

    //! @brief Syntax tag element class.
    class SyntaxTag : public SyntaxNode
    {
    public:
        //! @brief Constructor.
        explicit SyntaxTag(
            const bool B_Hollow     //!< Hollow attribute.
            );

        //! @brief Destructor.
        virtual ~SyntaxTag(void);

    private:
        //! @brief No default constructor.
        explicit SyntaxTag(void);
        //! @brief No copy constructor.
        SyntaxTag(const SyntaxTag&);
        //! @brief No assignment operator.
        SyntaxTag& operator=(const SyntaxTag&);

    public:
        // Inherit doxygen comments from cli::Element documentation.
        virtual const tk::String GetKeyword(void) const;

        //! @brief Retrieves the hollow attribute.
        //! @return true for hollow tag, false otherwise.
        const bool GetbHollow(void) const;

    private:
        //! Hollow attribute.
        const bool m_bHollow;
    };


    //! @brief Syntax tag reference element.
    class SyntaxRef : public Element
    {
    public:
        //! @brief Constructor.
        explicit SyntaxRef(
            const SyntaxTag& CLI_Tag        //!< Referenced tag element.
            );

        //! @brief Destructor.
        virtual ~SyntaxRef(void);

    private:
        //! @brief No default constructor.
        explicit SyntaxRef(void);
        //! @brief No copy constructor.
        SyntaxRef(const SyntaxRef&);
        //! @brief No assignment operator.
        SyntaxRef& operator=(const SyntaxRef&);

    public:
        // Inherit doxygen comments from cli::Element documentation.
        virtual const tk::String GetKeyword(void) const;

        //! @brief Tag reference accessor.
        //! @return Tag reference.
        const SyntaxTag& GetTag(void) const;

    private:
        //! Tag reference.
        const SyntaxTag* const m_pcliTag;
    };

CLI_NS_END(cli)

#endif // _CLI_SYNTAX_TAG_H_
