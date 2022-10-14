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
//! @brief SyntaxNode class definition.

#ifndef _CLI_SYNTAX_NODE_H_
#define _CLI_SYNTAX_NODE_H_

#include "cli/namespace.h"
#include "cli/element.h"
#include "cli/tk.h"


CLI_NS_BEGIN(cli)

    // Forward declarations.
    class Help;


    //! @brief Syntax node elements.
    //!
    //! Syntax node elements are element that accepts child elements.
    //! They store a list of possible childs,
    //! and offer operations based on this list.
    class SyntaxNode : public Element
    {
    public:
        //! @brief Constructor.
        explicit SyntaxNode(
            const char* const STR_Keyword,  //!< Keyword of the element.
            const Help& CLI_Help            //!< Corresponding help.
            );

        //! @brief Destructor.
        virtual ~SyntaxNode(void);

    private:
        //! @brief No default constructor.
        explicit SyntaxNode(void);
        //! @brief No copy constructor.
        SyntaxNode(const SyntaxNode&);
        //! @brief No assignment operator.
        SyntaxNode& operator=(const SyntaxNode&);

    public:
        //! @brief Possible element addition.
        //! @return The element added.
        Element& AddElement(
            Element* const PCLI_Element     //!< New element.
            );

        //! @brief Element removal.
        //! @return true if the element has been actually removed, false otherwise.
        const bool RemoveElement(
            const Element* const PCLI_Element,  //!< Element to remove.
            const bool B_AutoDelete             //!< Auto-deletion flag. true have the given object being automatically deleted when removed.
            );

        // Inherit doxygen comments from cli::Element documentation.
        virtual const bool FindElements(Element::List& CLI_ExactList, Element::List& CLI_NearList, const char* const STR_Keyword) const;

    private:
        //! List of possible child elements.
        Element::List m_cliElements;
    };

CLI_NS_END(cli)

#endif // _CLI_SYNTAX_NODE_H_
