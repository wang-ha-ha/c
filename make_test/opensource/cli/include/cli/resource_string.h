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
//! @brief Help class definition.

#ifndef _CLI_STRING_H_
#define _CLI_STRING_H_

#include "cli/namespace.h"
#include "cli/object.h"
#include "cli/tk.h"


CLI_NS_BEGIN(cli)

    //! @brief ResourceString class.
    //!
    //! Objects of this class permits string definition of different languages.
    class ResourceString : public Object
    {
    public:
        //! @brief Default constructor.
        explicit ResourceString(void);

        //! @brief Copy constructor.
        ResourceString(
            const ResourceString& CLI_String    //!< Source string object.
            );

        //! @brief Destructor.
        virtual ~ResourceString(void);

    public:
        //! @brief Assignment operator.
        //! @return The ResourceString instance itself.
        ResourceString& operator=(
            const ResourceString& STR_String    //!< Source string object.
            );

    public:
        //! @brief Languages identifiers.
        typedef enum
        {
            LANG_EN,        //!< English.
            LANG_FR,        //!< French.
            LANG_COUNT,     //!< Languages count.
            LANG_DEFAULT = LANG_EN
        } LANG;

    public:
        //! @brief Resets the resource string.
        //! @return The ResourceString instance itself.
        //! @author [contrib: Oleg Smolsky, 2010, based on CLI 2.5]
        ResourceString& Reset(void);

        //! @brief Adds resource for a given language.
        //! @return The ResourceString instance itself.
        //!
        //! If a string already exists for the given language,
        //! then it is overwritten.
        ResourceString& SetString(
            const LANG E_Lang,              //!< Language identifier.
            const char* const STR_String    //!< String.
            );

        //! @brief Determines whether a string exists for a given language.
        //! @return true: The resource string has a string set for the given language.
        //! @return false: The resource string has no string set for the given language.
        const bool HasString(
            const LANG E_Lang               //!< Language identifier.
            ) const;

        //! @brief Access to the string of a given language.
        //! @return The string defined for the given language.
        //!
        //! If the string does not exist for the given language, the defaultt language is used.
        const tk::String GetString(
            const LANG E_Lang               //!< Language identifier.
            ) const;

        //! @brief Determines whether the resource string is empty (ie no string has been set for any language).
        //! @return true: The resource string is empty.
        //! @return false: The resource string is not empty.
        const bool IsEmpty(void) const;

    public:
        //! @brief String concatenation in the aim of a resource definition.
        //! @return The concatenation of the string arguments.
        static const tk::String Concat(
            const char* const STR_1,        //!< First string.
            const char* const STR_2         //!< Second string.
            );

        //! @brief String concatenation in the aim of a resource definition.
        //! @return The concatenation of the string arguments.
        static const tk::String Concat(
            const char* const STR_1,        //!< First string.
            const char* const STR_2,        //!< Second string.
            const char* const STR_3         //!< Third string.
            );

        //! @brief String concatenation in the aim of a resource definition.
        //! @return The concatenation of the string arguments.
        static const tk::String Concat(
            const char* const STR_1,        //!< First string.
            const char* const STR_2,        //!< Second string.
            const char* const STR_3,        //!< Third string.
            const char* const STR_4         //!< Fourth string.
            );

        //! @brief String concatenation in the aim of a resource definition.
        //! @return The concatenation of the string arguments.
        static const tk::String Concat(
            const char* const STR_1,        //!< First string.
            const char* const STR_2,        //!< Second string.
            const char* const STR_3,        //!< Third string.
            const char* const STR_4,        //!< Fourth string.
            const char* const STR_5         //!< Fifth string.
            );

    private:
        //! Help map type shortcut.
        typedef tk::Map<const LANG, tk::String> LangStringMap;
        //! Help map.
        LangStringMap m_mapStrings;
    };

    //! @brief Addition operator.
    //! @return The concatenation of the resource string arguments.
    //!
    //! Concats the strings of the input resource string objects for each language.
    const ResourceString operator+(
        const ResourceString& CLI_Str1,     //!< First member.
        const ResourceString& CLI_Str2      //!< Second member.
        );

CLI_NS_END(cli)

#endif // _CLI_STRING_H_

