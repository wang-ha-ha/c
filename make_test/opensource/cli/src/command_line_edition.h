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
//! @brief CmdLineEdition class definition.

#ifndef _CLI_CMD_LINE_EDITION_H_
#define _CLI_CMD_LINE_EDITION_H_

#include "cli/namespace.h"
#include "cli/object.h"
#include "cli/tk.h"
#include "cli/io_device.h"


CLI_NS_BEGIN(cli)

    // Forward declarations.
    class OutputDevice;


    //! @brief Command line edition objet.
    class CmdLineEdition : public Object
    {
    public:
        //! @brief Copy constructor.
        //!
        //! Useful so that CmdLineEdition objects can stored in a tk::Queue<CmdLineEdition> member in CmdLineHistory.
        CmdLineEdition(const CmdLineEdition&);

        //! @brief Default constructor.
        explicit CmdLineEdition(void);

        //! @brief Destructor.
        virtual ~CmdLineEdition(void);

    public:
        //! @brief Assignment operator.
        //! @return The CmdLineEdition instance itself.
        CmdLineEdition& operator=(const CmdLineEdition&);

        //! @brief Set the object.
        //! @note Does not print out anything.
        void Set(
            const tk::String& TK_Left,  //!< Left part of the command line.
            const tk::String& TK_Right  //!< Right part of the command line.
            );

        //! @brief Reset the object.
        //! @note Does not print out anything.
        void Reset(void);

    public:
        //! @brief Insert mode setting.
        //! @note Default is on.
        void SetInsertMode(
            const bool B_InsertMode //!< Insert Mode.
            );

        //! @brief Insert mode retrieval.
        //! @return Current insert mode.
        const bool GetInsertMode(void) const;

    public:
        //! @brief Character addition.
        void Put(
            const OutputDevice& CLI_OutputDevice,   //!< Output device.
            const KEY E_Char                        //!< Character to add.
            );

        //! @brief String addition.
        void Put(
            const OutputDevice& CLI_OutputDevice,   //!< Output device.
            const tk::String& TK_String             //!< String to add.
            );

    private:
        //! @brief KEY list addition. Inner implementation.
        void Put(
            const OutputDevice& CLI_OutputDevice,   //!< Output device.
            const tk::Queue<KEY>& TK_Keys           //!< KEY list.
            );

    public:
        //! @brief Line deletion.
        void CleanAll(
            const OutputDevice& CLI_OutputDevice    //!< Output device.
            );

        //! @brief Character deletion
        //! @note   If the deletion would move the cursor out of the bounds of the line,
        //!         the cursor gets stuck either at the beginning (when deleting backward) or at the end (when deleting forward).
        void Delete(
            const OutputDevice& CLI_OutputDevice,   //!< Output device.
            const int I_Count                       //!< Number of characters to delete.
                                                    //!< A positive value means deleting characters forward (from the right of the cursor).
                                                    //!< A negative value means deleting characters backward (from the left of the cursor, ie backspace).
            );

    public:
        //! @brief Command line display.
        //! @note Displays the current command line, positionning the cursor at the correct place.
        void PrintCmdLine(
            const OutputDevice& CLI_OutputDevice    //!< Output device.
            ) const;

        //! @brief Moves the cursor.
        //! @note   If the call would move the cursor out of the bounds of the line,
        //!         the cursor gets stuck either at the beginning (when moving backward) or at the end (when moving forward).
        void MoveCursor(
            const OutputDevice& CLI_OutputDevice,   //!< Output device.
            const int I_Count                       //!< Number of characters to switch.
                                                    //!< A positive value means moving forward (on the right).
                                                    //!< A negative value means moving backward (on the left).
            );

        //! @brief Moves the cursor to the next line.
        void NextLine(
            const OutputDevice& CLI_OutputDevice    //!< Output device.
            );

        //! @brief Moves the cursor at the beginning of the line.
        void Home(
            const OutputDevice& CLI_OutputDevice    //!< Output device.
            );

        //! @brief Moves the cursor at the end of the line.
        void End(
            const OutputDevice& CLI_OutputDevice    //!< Output device.
            );

    public:
        //! @brief Current command line.
        //! @return The current command line.
        const tk::String GetLine(void) const;

        //! @brief Right part of the command line accessor.
        //! @return The right part of the command line.
        const tk::String GetRight(void) const;

        //! @brief Left part of the command line accessor.
        //! @return The left part of the command line.
        const tk::String GetLeft(void) const;

        //! @brief Next word retrieval.
        //! @return The next word after the cursor.
        //! @note A word means until the next blank.
        const tk::String GetNextWord(void) const;

        //! @brief Previous word retrieval.
        //! @return The previous word before the cursor.
        //! @note A word means until the next blank.
        const tk::String GetPrevWord(void) const;

    private:
        //! Left part of the command line.
        tk::Queue<KEY> m_tkLeft;

        //! Right part of the command line.
        tk::Queue<KEY> m_tkRight;

        //! Insert mode. Default is true.
        bool m_bInsertMode;
    };

CLI_NS_END(cli)

#endif // _CLI_CMD_LINE_EDITION_H_
