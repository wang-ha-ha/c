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

#include "cli/resource_string.h"
#include "cli/traces.h"
#include "cli/io_device.h"
#include "constraints.h"

CLI_NS_USE(cli)


ResourceString::ResourceString(void)
  : m_mapStrings(LANG_COUNT)
{
}

ResourceString::ResourceString(const ResourceString& CLI_String)
  : m_mapStrings(CLI_String.m_mapStrings)
{
}

ResourceString::~ResourceString(void)
{
}

ResourceString& ResourceString::operator=(const ResourceString& CLI_String)
{
    for (   LANG e_Lang = (LANG) 0;
            e_Lang < LANG_COUNT;
            e_Lang = (LANG) (((int) e_Lang) + 1))
    {
        if (CLI_String.HasString(e_Lang))
        {
            SetString(e_Lang, CLI_String.GetString(e_Lang));
        }
        else
        {
            SetString(e_Lang, "");
        }
    }
    return *this;
}

// [contrib: Oleg Smolsky, 2010, based on CLI 2.5]
ResourceString& ResourceString::Reset(void)
{
    m_mapStrings.Reset();
    return *this;
}

ResourceString& ResourceString::SetString(const ResourceString::LANG E_Lang, const char* const STR_String)
{
    if (! m_mapStrings.SetAt(E_Lang, tk::String(MAX_RESOURCE_LENGTH, STR_String)))
    {
        GetTraces().Trace(INTERNAL_ERROR)
            << "Could not set resource '" << STR_String << "' "
            << "for language " << (int) E_Lang
            << endl;
    }
    return *this;
}

const bool ResourceString::HasString(const ResourceString::LANG E_Lang) const
{
    // The string for the given language is set if there is a key for it in the map.
    return m_mapStrings.IsSet(E_Lang);
}

const tk::String ResourceString::GetString(const ResourceString::LANG E_Lang) const
{
    if (const tk::String* const pstr_String = m_mapStrings.GetAt(E_Lang))
    {
        return *pstr_String;
    }
    else
    {
        return tk::String(MAX_RESOURCE_LENGTH, "");
    }
}

const bool ResourceString::IsEmpty(void) const
{
    for (int e_Lang = 0; e_Lang < LANG_COUNT; e_Lang ++)
    {
        if (! GetString((LANG) e_Lang).IsEmpty())
        {
            // If one language has a non empty string set, then the resource string is not empty.
            return false;
        }
    }

    // If no language has a string set, then the resource string is empty.
    return true;
}

const tk::String ResourceString::Concat(
        const char* const STR_1, const char* const STR_2)
{
    // Redirection to tk::String::Concat() with MAX_RESOURCE_LENGTH indication.
    return tk::String::Concat(MAX_RESOURCE_LENGTH, STR_1, STR_2);
}

const tk::String ResourceString::Concat(
        const char* const STR_1, const char* const STR_2, const char* const STR_3)
{
    // Redirection to tk::String::Concat() with MAX_RESOURCE_LENGTH indication.
    return tk::String::Concat(MAX_RESOURCE_LENGTH, STR_1, STR_2, STR_3);
}

const tk::String ResourceString::Concat(
        const char* const STR_1, const char* const STR_2, const char* const STR_3,
        const char* const STR_4)
{
    // Redirection to tk::String::Concat() with MAX_RESOURCE_LENGTH indication.
    return tk::String::Concat(MAX_RESOURCE_LENGTH, STR_1, STR_2, STR_3, STR_4);
}

const tk::String ResourceString::Concat(
        const char* const STR_1, const char* const STR_2, const char* const STR_3,
        const char* const STR_4, const char* const STR_5)
{
    // Redirection to tk::String::Concat() with MAX_RESOURCE_LENGTH indication.
    return tk::String::Concat(MAX_RESOURCE_LENGTH, STR_1, STR_2, STR_3, STR_4, STR_5);
}

const ResourceString cli::operator+(const ResourceString& CLI_Str1, const ResourceString& CLI_Str2)
{
    ResourceString cli_Result;
    for (int e_Lang = 0; e_Lang < ResourceString::LANG_COUNT; e_Lang ++)
    {
        // Concatenation for each language.
        cli_Result.SetString((ResourceString::LANG) e_Lang,
            ResourceString::Concat(
                CLI_Str1.GetString((ResourceString::LANG) e_Lang),
                CLI_Str2.GetString((ResourceString::LANG) e_Lang)));
    }
    return cli_Result;
}
