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
//! @brief Text class definition.

#ifndef _CLI_UI_TEXT_H_
#define _CLI_UI_TEXT_H_

#include "cli/io_device.h"


CLI_NS_BEGIN(cli)

    CLI_NS_BEGIN(ui)

        // Forward declarations.
        class TextIterator;


        //! @brief Simple line user interface object.
        class Text : public OutputDevice
        {
        public:
            //! @brief Constructor.
            explicit Text(
                const unsigned int UI_MaxLines,     //!< Maximum number of lines
                const unsigned int UI_MaxLineLength //!< Maximum length of lines
                );

            //! @brief Destructor.
            virtual ~Text(void);

        private:
            //! @brief No default constructor.
            explicit Text(void);
            //! @brief No copy constructor.
            Text(const Text&);
            //! @brief No assignment operator.
            Text& operator=(const Text&);

        // cli::OutputDevice interface implementation.
        protected:
            virtual const bool OpenDevice(void);
            virtual const bool CloseDevice(void);
        public:
            // Inherit doxygen comments from cli::OutputDevice interface documentation.
            virtual void PutString(const char* const STR_Out) const;
            // Inherit doxygen comments from cli::OutputDevice interface documentation.
            virtual void Beep(void) const;
            // Inherit doxygen comments from cli::OutputDevice interface documentation.
            virtual void CleanScreen(void) const;
            // Inherit doxygen comments from cli::OutputDevice interface documentation.
            virtual const bool WouldOutput(const OutputDevice& CLI_Device) const;

        public:
            //! @brief Retrieves a text head iterator.
            void Begin(
                TextIterator& it                                //!< Current iterator.
                ) const;

            //! @brief Moves iterator one page up.
            //! @return true if the iterator has changed, false otherwise.
            const bool PageUp(
                TextIterator& it                                //!< Current iterator.
                ) const;

            //! @brief Moves iterator one line up.
            //! @return true if the iterator has changed, false otherwise.
            const bool LineUp(
                TextIterator& it                                //!< Current iterator.
                ) const;

            //! @brief Moves iterator one line down.
            //! @return true if the iterator has changed, false otherwise.
            const bool LineDown(
                TextIterator& it,                               //!< Current iterator.
                const OutputDevice* const PCLI_Out              //!< Output device, if the bottom line should be output by the way.
                ) const;

            //! @brief Moves iterator one page down.
            //! @return true if the iterator has changed, false otherwise.
            const bool PageDown(
                TextIterator& it,                               //!< Current iterator.
                const OutputDevice* const PCLI_Out              //!< Output device, if the page should be output by the way.
                ) const;

            //! @brief Retrieves a text end iterator.
            void End(
                TextIterator& it,                               //!< Current iterator.
                const OutputDevice* const PCLI_Out              //!< Output device, if the end of the text should be output by the way.
                ) const;

        public:
            //! @brief Print out a page of text.
            //!
            //! Updates automatically the bottom position of the iterator one page below the top position.
            void PrintPage(
                TextIterator& it,                               //!< Iterator which top position is ready for a page display.
                const OutputDevice& CLI_Out,                    //!< Output device to use for display.
                const bool B_FillPageWithBlankLines             //!< true when incomplete pages should be filled with blank lines.
                ) const;

        private:
            //! @brief Print out one more bottom line.
            //!
            //! Pushes the bottom position one line down.
            void PrintBottomLine(
                TextIterator& it,                               //!< Iterator which bottom position is ready for a line display.
                const OutputDevice& CLI_Out                     //!< Output device to use for display.
                ) const;

        private:
            mutable tk::Queue<tk::String> m_tkLines;            //!< List of lines to display.
            mutable bool m_bNewLineRequired;                    //!< Flag indicating whether a new line is required for next characters.
            const unsigned int m_uiMaxLineLength;               //!< Maximum length of lines.
        };

        //! @brief Text iterator class.
        class TextIterator
        {
        private:
            //! @brief No default constructor.
            explicit TextIterator(void);

        public:
            //! @brief Regular constructor.
            explicit TextIterator(
                const OutputDevice::ScreenInfo& CLI_ScreenInfo, //!< Screen characteristics to take in account.
                const unsigned int UI_PageHeight                //!< Number of lines to display in a page.
                );

            //! @brief Copy constructor.
            explicit TextIterator(
                const TextIterator& it                          //!< Text iterator to copy.
                );

            //! @brief Assignment operator.
            //! @return The TextIterator instance itself.
            TextIterator& operator=(
                const TextIterator& it                          //!< Text iterator to copy.
                );

        private:
            const OutputDevice::ScreenInfo m_cliScreenInfo; //!< Screen characteristics.
            const unsigned int m_uiPageHeight;              //!< Number of lines in a page.

            tk::Queue<tk::String>::Iterator m_tkTopLine;    //!< Top line position.
            unsigned int m_uiTopChar;                       //!< Character position in the top line.
            tk::Queue<tk::String>::Iterator m_tkBottomLine; //!< Bottom line position.
            unsigned int m_uiBottomChar;                    //!< Character position in the bottom line.
            bool m_bBottomIsUpToDate;                       //!< Flag indicating whether the bottom position is up to date according to the top position.

            friend class Text;
        };

    CLI_NS_END(ui)

CLI_NS_END(cli)

#endif // _CLI_UI_TEXT_H_
