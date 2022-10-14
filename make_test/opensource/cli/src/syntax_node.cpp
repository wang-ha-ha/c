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

#include <stdlib.h>

#include "cli/syntax_node.h"
#include "cli/syntax_tag.h"
#include "cli/param.h"
#include "cli/traces.h"
#include "cli/assert.h"
#include "cli/io_device.h"
#include "constraints.h"

CLI_NS_USE(cli)


SyntaxNode::SyntaxNode(const char* const STR_Keyword, const Help& CLI_Help)
  : Element(STR_Keyword, CLI_Help),
    m_cliElements(MAX_WORDS_PER_NODE)
{
}

SyntaxNode::~SyntaxNode(void)
{
    while (! m_cliElements.IsEmpty())
    {
        if (const Element* const pcli_Element = m_cliElements.RemoveHead())
        {
            delete pcli_Element;
        }
    }
}

Element& SyntaxNode::AddElement(Element* const PCLI_Element)
{
    CLI_ASSERT(PCLI_Element != NULL);

    for (   Element::List::Iterator it = m_cliElements.GetIterator();
            m_cliElements.IsValid(it);
            m_cliElements.MoveNext(it))
    {
        if (const Element* const pcli_Element = m_cliElements.GetAt(it))
        {
            if (pcli_Element->GetKeyword() == PCLI_Element->GetKeyword())
            {
                if (PCLI_Element != pcli_Element)
                {
                    //! @warning Conflicting names:
                    //!         The behaviour is the following: deletion of the new element,
                    //!         and retrieval of the element with the same name.
                    //!         This could be convenient if both elements are keywords for instance,
                    //!         but there is absolutely no guarantee for any other conditions.
                    delete PCLI_Element;
                    return const_cast<Element&>(*pcli_Element);
                }
                else
                {
                    // Element already available from this syntaxe node.
                    // Do not add it again.
                    // Just return the reference.
                    return *PCLI_Element;
                }
            }
        }
    }

    // Regular behaviour.
    PCLI_Element->SetCli(GetCli());
    m_cliElements.AddTail(PCLI_Element);
    return *PCLI_Element;
}

const bool SyntaxNode::RemoveElement(const Element* const PCLI_Element, const bool B_AutoDelete)
{
    bool b_Res = false;

    for (   Element::List::Iterator it = m_cliElements.GetIterator();
            m_cliElements.IsValid(it);
            m_cliElements.MoveNext(it))
    {
        if (const Element* const pcli_Element = m_cliElements.GetAt(it))
        {
            if (pcli_Element == PCLI_Element)
            {
                // Element to remove found
                m_cliElements.Remove(it);
                if (B_AutoDelete)
                {
                    delete pcli_Element;
                }
                b_Res = true;
            }
        }
    }

    return b_Res;
}

const bool SyntaxNode::FindElements(
        Element::List& CLI_ExactList,
        Element::List& CLI_NearList,
        const char* const STR_Keyword
        ) const
{
    // For each child...
    for (   Element::List::Iterator it = m_cliElements.GetIterator();
            m_cliElements.IsValid(it);
            m_cliElements.MoveNext(it))
    {
        if (const Element* const pcli_Element = m_cliElements.GetAt(it))
        {
            if (false) {}
            else if (const SyntaxTag* const pcli_Tag = dynamic_cast<const SyntaxTag*>(pcli_Element))
            {
                // Propagate call over child non hollow tag.
                if (! pcli_Tag->GetbHollow())
                {
                    if (! pcli_Tag->FindElements(CLI_ExactList, CLI_NearList, STR_Keyword))
                    {
                        return false;
                    }
                }
            }
            else if (const SyntaxRef* const pcli_Ref = dynamic_cast<const SyntaxRef*>(pcli_Element))
            {
                // Propagate call over referenced tag.
                if (! pcli_Ref->GetTag().FindElements(CLI_ExactList, CLI_NearList, STR_Keyword))
                {
                    return false;
                }
            }
            else if (STR_Keyword == NULL)
            {
                // No keyword begun.
                // Retrieve all sub-elements.
                if (! CLI_NearList.AddTail(pcli_Element))
                {
                    GetTraces().Trace(INTERNAL_ERROR) << "SyntaxNode::FindElements(): Not enough space in CLI_ExactList." << endl;
                }
            }
            else
            {
                // A beginning of word has been given.
                const tk::String str_Keyword(MAX_WORD_LENGTH, STR_Keyword);

                if (const Param* const pcli_Param = dynamic_cast<const Param*>(pcli_Element))
                {
                    // If the child element is a parameter, check SetstrValue() works for it.
                    if (str_Keyword != "\n")
                    {
                        if (pcli_Param->SetstrValue(str_Keyword))
                        {
                            if (! CLI_NearList.AddTail(pcli_Param))
                            {
                                GetTraces().Trace(INTERNAL_ERROR) << "SyntaxNode::FindElements(): Not enough space in CLI_ExactList." << endl;
                            }
                            if (! CLI_ExactList.AddTail(pcli_Param))
                            {
                                GetTraces().Trace(INTERNAL_ERROR) << "SyntaxNode::FindElements(): Not enough space in CLI_ExactList." << endl;
                            }
                        }
                    }
                }
                else if (pcli_Element->GetKeyword().SubString(0, str_Keyword.GetLength()) == str_Keyword)
                {
                    // Check the beginning of the word for other elements.
                    if (! CLI_NearList.AddTail(pcli_Element))
                    {
                        GetTraces().Trace(INTERNAL_ERROR) << "SyntaxNode::FindElements(): Not enough space in CLI_ExactList." << endl;
                    }
                    if (str_Keyword.GetLength() == pcli_Element->GetKeyword().GetLength())
                    {
                        if (! CLI_ExactList.AddTail(pcli_Element))
                        {
                            GetTraces().Trace(INTERNAL_ERROR) << "SyntaxNode::FindElements(): Not enough space in CLI_ExactList." << endl;
                        }
                    }
                }
            }
        }
    }

    return true;
}

