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
//! @brief InputFileDevice and OutputFileDevice classes definition.

#ifndef _CLI_FILE_DEVICE_H_
#define _CLI_FILE_DEVICE_H_

#include <stdio.h>

#include "cli/namespace.h"
#include "cli/io_device.h"
#include "cli/tk.h"


CLI_NS_BEGIN(cli)

    //! @brief Input file device class.
    class InputFileDevice : public IODevice
    {
    public:
        //! @brief Constructor.
        explicit InputFileDevice(
            const char* const STR_FileName,     //!< Input file name.
            OutputDevice& CLI_Out,              //!< Output device.
                                                //!< This output device will be opened automatically
                                                //!< when the input device is opened.
            const bool B_AutoDelete             //!< Auto-deletion flag.
            );

        //! @brief Destructor.
        virtual ~InputFileDevice(void);

    private:
        //! @brief No default constructor.
        explicit InputFileDevice(void);
        //! @brief No copy constructor.
        InputFileDevice(const InputFileDevice&);
        //! @brief No assignment operator.
        InputFileDevice& operator=(const InputFileDevice&);

    public:
        //! @brief Special character enabling.
        //! @return Same InputFileDevice instance reference.
        InputFileDevice& EnableSpecialCharacters(
            const bool B_EnableSpecialCharacters    //!< true for enabling. false otherwise.
            );

    protected:
        // Inherit doxygen comments from cli::OutputDevice interface documentation.
        virtual const bool OpenDevice(void);
        // Inherit doxygen comments from cli::OutputDevice interface documentation.
        virtual const bool CloseDevice(void);
    public:
        // Inherit doxygen comments from cli::IODevice interface documentation.
        virtual const KEY GetKey(void) const;
        // Inherit doxygen comments from cli::IODevice interface documentation.
        virtual const ResourceString GetLocation(void) const;
        // Inherit doxygen comments from cli::OutputDevice interface documentation.
        virtual void PutString(const char* const STR_Out) const;
        // Inherit doxygen comments from cli::OutputDevice interface documentation.
        virtual void Beep(void) const;
        // Inherit doxygen comments from cli::OutputDevice interface documentation.
        virtual void CleanScreen(void) const;
        // Inherit doxygen comments from cli::OutputDevice interface documentation.
        virtual const bool WouldOutput(const OutputDevice& CLI_Device) const;

    public:
        //! @brief File name accessor.
        //! @return File name.
        const tk::String GetFileName(void) const;

        //! @brief Current line accessor.
        //! @return Current line, starting from 0.
        const int GetCurrentLine(void) const;

        //! @brief Current column accessor.
        //! @return Current column, starting from 0.
        const int GetCurrentColumn(void) const;

    private:
        //! Input file name.
        const tk::String m_strFileName;

        //! Input file.
        mutable FILE* m_pfFile;

        //! Special character enabling.
        bool m_bEnableSpecialCharacters;

        //! Output device.
        OutputDevice& m_cliOutput;

        //! Character buffer.
        mutable tk::Queue<char> m_tkInputBuffer;

        //! Current Line.
        mutable int m_iCurrentLine;

        //! Current column.
        mutable int m_iCurrentColumn;

        //! Next character line.
        mutable int m_iNextLine;

        //! Next column.
        mutable int m_iNextColumn;
    };

    //! @brief Output file device.
    class OutputFileDevice : public OutputDevice
    {
    public:
        //! @brief Constructor.
        explicit OutputFileDevice(
            const char* const STR_OutputFileName,   //!< Output file name.
            const bool B_AutoDelete                 //!< Auto-deletion flag.
            );

        //! @brief Destructor.
        virtual ~OutputFileDevice(void);

    private:
        //! @brief No default constructor.
        explicit OutputFileDevice(void);
        //! @brief No copy constructor.
        OutputFileDevice(const OutputFileDevice&);
        //! @brief No assignment operator.
        OutputFileDevice& operator=(const OutputFileDevice&);

    protected:
        // Inherit doxygen comments from cli::OutputDevice interface documentation.
        virtual const bool OpenDevice(void);
        // Inherit doxygen comments from cli::OutputDevice interface documentation.
        virtual const bool CloseDevice(void);
    public:
        // Inherit doxygen comments from cli::OutputDevice interface documentation.
        virtual void PutString(const char* const STR_Out) const;
        // Inherit doxygen comments from cli::OutputDevice interface documentation.
        virtual void Beep(void) const;

    public:
        //! @brief File name accessor.
        //! @return File name.
        const tk::String GetFileName(void) const;

    private:
        //! Output file name.
        const tk::String m_strFileName;

        //! Output file.
        mutable FILE* m_pfFile;
    };

CLI_NS_END(cli)

#endif // _CLI_FILE_DEVICE_H_
