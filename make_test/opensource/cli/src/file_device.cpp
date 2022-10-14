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

#include <stdio.h>
#include <string.h>

#include "cli/file_device.h"
#include "cli/assert.h"
#include "consistency.h"
#include "constraints.h"
#include "utils.h"

CLI_NS_USE(cli)


//! @brief Maximum input buffer length;
static const unsigned int MAX_INPUT_BUFFER_LEN = 1024;


InputFileDevice::InputFileDevice(
        const char* const STR_FileName,
        OutputDevice& CLI_Output,
        const bool B_AutoDelete)
  : IODevice(tk::String::Concat(MAX_DEVICE_NAME_LENGTH, "input-file[", STR_FileName, "]"), B_AutoDelete),
    m_strFileName(MAX_FILE_PATH_LENGTH, STR_FileName), m_pfFile(NULL), m_bEnableSpecialCharacters(false),
    m_cliOutput(CLI_Output),
    m_tkInputBuffer(MAX_INPUT_BUFFER_LEN),
    m_iCurrentLine(0), m_iCurrentColumn(0), m_iNextLine(1), m_iNextColumn(1)
{
    EnsureCommonDevices();

    m_cliOutput.UseInstance(__CALL_INFO__);
}

InputFileDevice::~InputFileDevice(void)
{
    m_cliOutput.FreeInstance(__CALL_INFO__);
}

InputFileDevice& InputFileDevice::EnableSpecialCharacters(const bool B_EnableSpecialCharacters)
{
    m_bEnableSpecialCharacters = B_EnableSpecialCharacters;
    return *this;
}

const bool InputFileDevice::OpenDevice(void)
{
    CLI_ASSERT(& m_cliOutput != this);
    if (! m_cliOutput.OpenUp(__CALL_INFO__))
    {
        m_cliLastError = m_cliOutput.GetLastError();
        return false;
    }

    if (m_pfFile == NULL)
    {
        m_pfFile = fopen(m_strFileName, "r");
        if (m_pfFile == NULL)
        {
            m_cliLastError
                .SetString(ResourceString::LANG_EN, ResourceString::Concat("Cannot open input file '", m_strFileName, "'"))
                .SetString(ResourceString::LANG_FR, ResourceString::Concat("Impossible d'ouvrir le fichier d'entrée '", m_strFileName, "'"));
            return false;
        }
    }

    return true;
}

const bool InputFileDevice::CloseDevice(void)
{
    bool b_Res = true;

    if (m_pfFile != NULL)
    {
        if (fclose(m_pfFile) != 0)
        {
            m_cliLastError
                .SetString(ResourceString::LANG_EN, ResourceString::Concat("Cannot close input file '", m_strFileName, "'"))
                .SetString(ResourceString::LANG_FR, ResourceString::Concat("Impossible de fermer le fichier d'entrée '", m_strFileName, "'"));
            b_Res = false;
        }
        m_pfFile = NULL;
    }

    if (! m_cliOutput.CloseDown(__CALL_INFO__))
    {
        m_cliLastError = m_cliOutput.GetLastError();
        b_Res = false;
    }

    return b_Res;
}

const KEY InputFileDevice::GetKey(void) const
{
    if (m_pfFile != NULL)
    {
        // While there are still characters to read.
        while ((! m_tkInputBuffer.IsEmpty()) || (! feof(m_pfFile)))
        {
            // Check the input buffer
            if (m_tkInputBuffer.IsEmpty())
            {
                if ((m_pfFile != NULL) && (! feof(m_pfFile)))
                {
                    char str_Buffer[MAX_INPUT_BUFFER_LEN];
                    size_t ui_Bytes = fread(str_Buffer, sizeof(char), sizeof(str_Buffer), m_pfFile);
                    if (ui_Bytes > 0)
                    {
                        for (size_t ui=0; (ui<ui_Bytes) && (ui<sizeof(str_Buffer)); ui++)
                        {
                            m_tkInputBuffer.AddTail(str_Buffer[ui]);
                        }
                    }
                    else
                    {
                        m_cliLastError
                            .SetString(ResourceString::LANG_EN, ResourceString::Concat("Error while reading input file '", m_strFileName, "'"))
                            .SetString(ResourceString::LANG_FR, ResourceString::Concat("Erreur de lecture du fichier d'entrée '", m_strFileName, "'"));
                        return NULL_KEY;
                    }
                }
            }

            // Read the next character.
            if (! m_tkInputBuffer.IsEmpty())
            {
                char c_Char = m_tkInputBuffer.RemoveHead();

                // Next line/column management & special character management.
                switch (c_Char)
                {
                case '\n':
                    m_iCurrentLine = m_iNextLine;       m_iNextLine ++;
                    m_iCurrentColumn = m_iNextColumn;   m_iNextColumn = 1;
                    break;
                case '?':
                    if ((m_iNextLine == m_iCurrentLine) && (m_iNextColumn == m_iCurrentColumn) && (! m_bEnableSpecialCharacters))
                    {
                        // Income of the effective '?' character, after it has been escaped (see below).
                        // This time, step over.
                        m_iNextColumn ++;
                    }
                    else if (! m_bEnableSpecialCharacters)
                    {
                        // Escape the '?' character. Post it back. Do not step over yet.
                        m_iCurrentLine = m_iNextLine;
                        m_iCurrentColumn = m_iNextColumn;
                        c_Char = '\\';
                        m_tkInputBuffer.AddHead('?');
                    }
                    else
                    {
                        // Just step over.
                        m_iCurrentLine = m_iNextLine;
                        m_iCurrentColumn = m_iNextColumn ++;
                    }
                    break;
                case '\t':
                    if (! m_bEnableSpecialCharacters)
                    {
                        // Change tabs into spaces.
                        c_Char = ' ';
                    }
                    // Step over.
                    m_iCurrentLine = m_iNextLine;
                    m_iCurrentColumn = m_iNextColumn ++;
                default:
                    // Just step over.
                    m_iCurrentLine = m_iNextLine;
                    m_iCurrentColumn = m_iNextColumn ++;
                    break;
                }

                // Character analysis.
                const KEY e_Key = Char2Key(c_Char);
                if (e_Key == NULL_KEY)
                {
                    m_cliLastError
                        .SetString(ResourceString::LANG_EN, ResourceString::Concat("Error while reading input file '", m_strFileName, "'"))
                        .SetString(ResourceString::LANG_FR, ResourceString::Concat("Erreur de lecture du fichier d'entrée '", m_strFileName, "'"));
                }
                else if (e_Key == FEED_MORE)
                {
                    // Let's move on the next character
                }
                else
                {
                    return e_Key;
                }
            }
        }
    }

    return NULL_KEY;
}

const ResourceString InputFileDevice::GetLocation(void) const
{
    ResourceString cli_Location;

    char str_Location[1024];
    memset(str_Location, '\0', sizeof(str_Location));
    const int i_Res = snprintf(str_Location, sizeof(str_Location), "%s:%d", (const char* const) m_strFileName, GetCurrentLine());
    CheckSnprintfResult(str_Location, sizeof(str_Location), i_Res);
    for (int i_Lang = 0; i_Lang < ResourceString::LANG_COUNT; i_Lang ++)
    {
        cli_Location.SetString((ResourceString::LANG) i_Lang, str_Location);
    }
    return cli_Location;
}

void InputFileDevice::PutString(const char* const STR_Out) const
{
    if (! m_cliOutput.WouldOutput(*this)) // Avoid infinite loops.
    {
        m_cliOutput.PutString(STR_Out);
    }
}

void InputFileDevice::Beep(void) const
{
    if (! m_cliOutput.WouldOutput(*this)) // Avoid infinite loops.
    {
        m_cliOutput.Beep();
    }
}

void InputFileDevice::CleanScreen(void) const
{
    if (! m_cliOutput.WouldOutput(*this)) // Avoid infinite loops.
    {
        m_cliOutput.CleanScreen();
    }
}

const bool InputFileDevice::WouldOutput(const OutputDevice& CLI_Device) const
{
    return (IODevice::WouldOutput(CLI_Device) || m_cliOutput.WouldOutput(CLI_Device));
}

const tk::String InputFileDevice::GetFileName(void) const
{
    return m_strFileName;
}

const int InputFileDevice::GetCurrentLine(void) const
{
    return m_iCurrentLine;
}

const int InputFileDevice::GetCurrentColumn(void) const
{
    return m_iCurrentColumn;
}


OutputFileDevice::OutputFileDevice(const char* const STR_FileName, const bool B_AutoDelete)
  : OutputDevice(tk::String::Concat(MAX_DEVICE_NAME_LENGTH, "output-file[", STR_FileName, "]"), B_AutoDelete),
    m_strFileName(MAX_FILE_PATH_LENGTH, STR_FileName), m_pfFile(NULL)
{
}

OutputFileDevice::~OutputFileDevice(void)
{
}

const bool OutputFileDevice::OpenDevice(void)
{
    if (m_pfFile == NULL)
    {
        m_pfFile = fopen(m_strFileName, "w");
        if (m_pfFile == NULL)
        {
            m_cliLastError
                .SetString(ResourceString::LANG_EN, ResourceString::Concat("Cannot open output file '", m_strFileName, "'"))
                .SetString(ResourceString::LANG_FR, ResourceString::Concat("Impossible d'ouvrir le fichier de sortie '", m_strFileName, "'"));
            return false;
        }
    }

    return true;
}

const bool OutputFileDevice::CloseDevice(void)
{
    bool b_Res = true;

    if (m_pfFile != NULL)
    {
        if (fclose(m_pfFile) != 0)
        {
            m_cliLastError
                .SetString(ResourceString::LANG_EN, ResourceString::Concat("Cannot close output file '", m_strFileName, "'"))
                .SetString(ResourceString::LANG_FR, ResourceString::Concat("Impossible de fermer le fichier de sortie '", m_strFileName, "'"));
            b_Res = false;
        }
        m_pfFile = NULL;
    }

    return b_Res;
}

void OutputFileDevice::PutString(const char* const STR_Out) const
{
    if (m_pfFile != NULL)
    {
        fwrite(STR_Out, sizeof(char), strlen(STR_Out), m_pfFile);
    }
}

void OutputFileDevice::Beep(void) const
{
}

const tk::String OutputFileDevice::GetFileName(void) const
{
    return m_strFileName;
}

