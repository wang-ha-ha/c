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
//! @brief OutputDevice, IODevice and IOEndl classes defintion.

#ifndef _CLI_IO_DEVICE_H_
#define _CLI_IO_DEVICE_H_

#include <stdint.h> // int8_t, int16_t, int32_t, int64_t...

#include "cli/namespace.h"
#include "cli/object.h"
#include "cli/debug.h"
#include "cli/tk.h"
#include "cli/resource_string.h"


CLI_NS_BEGIN(cli)


    //! @brief Input keys.
    //! @return N/A (doxygen warning)
    typedef enum _KEY
    {
        NULL_KEY = '\0',    //!< Null key.

        BREAK = 3,          //!< Break (Ctrl+C).
        LOGOUT = 4,         //!< Logout (Ctrl+D).
        ENTER = 10,         //!< Enter.
        ESCAPE = 27,        //!< Escape.
        SPACE = 32,         //!< Space.
        BACKSPACE = 8,      //!< Backspace (changed from '\\b' to 8 in version 2.7 for ASCII compliance).
        DELETE = 127,       //!< Delete key (changed from 128 to 127 in version 2.7 for ASCII compliance).
        CLS = 501,          //!< Clean screen key (changed from 129 to 501 in order to avoid overlap with printable ASCII characters).
        INSERT = 502,       //!< Insert key (changed from 500 to 502 in order to avoid overlap with printable ASCII characters).
        TAB = '\t',         //!< Tab key

        KEY_0 = '0',
        KEY_1 = '1',
        KEY_2 = '2',
        KEY_3 = '3',
        KEY_4 = '4',
        KEY_5 = '5',
        KEY_6 = '6',
        KEY_7 = '7',
        KEY_8 = '8',
        KEY_9 = '9',
        KEY_a = 'a',
            KEY_aacute = 0xc3a1,    //!< Based on utf-8 encoding for 'á' (changed in version 2.9)
            KEY_agrave = 0xc3a0,    //!< Based on utf-8 encoding for 'à' (changed in version 2.9)
            KEY_auml = 0xc3a4,      //!< Based on utf-8 encoding for 'ä' (changed in version 2.9)
            KEY_acirc = 0xc3a2,     //!< Based on utf-8 encoding for 'â' (changed in version 2.9)
        KEY_b = 'b',
        KEY_c = 'c',
            KEY_ccedil = 0xc3a7,    //!< Based on utf-8 encoding for 'ç' (changed in version 2.9)
        KEY_d = 'd',
        KEY_e = 'e',
            KEY_eacute = 0xc3a9,    //!< Based on utf-8 encoding for 'é' (changed in version 2.9)
            KEY_egrave = 0xc3a8,    //!< Based on utf-8 encoding for 'è' (changed in version 2.9)
            KEY_euml = 0xc3ab,      //!< Based on utf-8 encoding for 'ü' (changed in version 2.9)
            KEY_ecirc = 0xc3aa,     //!< Based on utf-8 encoding for 'û' (changed in version 2.9)
        KEY_f = 'f',
        KEY_g = 'g',
        KEY_h = 'h',
        KEY_i = 'i',
            KEY_iacute = 0xc3ad,    //!< Based on utf-8 encoding for 'í' (changed in version 2.9)
            KEY_igrave = 0xc3ac,    //!< Based on utf-8 encoding for 'ì' (changed in version 2.9)
            KEY_iuml = 0xc3af,      //!< Based on utf-8 encoding for 'ï' (changed in version 2.9)
            KEY_icirc = 0xc3ae,     //!< Based on utf-8 encoding for 'î' (changed in version 2.9)
        KEY_j = 'j',
        KEY_k = 'k',
        KEY_l = 'l',
        KEY_m = 'm',
        KEY_n = 'n',
        KEY_o = 'o',
            KEY_oacute = 0xc3b3,    //!< Based on utf-8 encoding for 'ó' (changed in version 2.9)
            KEY_ograve = 0xc3b2,    //!< Based on utf-8 encoding for 'ò' (changed in version 2.9)
            KEY_ouml = 0xc3b6,      //!< Based on utf-8 encoding for 'ö' (changed in version 2.9)
            KEY_ocirc = 0xc3b4,     //!< Based on utf-8 encoding for 'ô' (changed in version 2.9)
        KEY_p = 'p',
        KEY_q = 'q',
        KEY_r = 'r',
        KEY_s = 's',
        KEY_t = 't',
        KEY_u = 'u',
            KEY_uacute = 0xc3ba,    //!< Based on utf-8 encoding for 'ú' (changed in version 2.9)
            KEY_ugrave = 0xc3b9,    //!< Based on utf-8 encoding for 'ù' (changed in version 2.9)
            KEY_uuml = 0xc3bc,      //!< Based on utf-8 encoding for 'ü' (changed in version 2.9)
            KEY_ucirc = 0xc3bb,     //!< Based on utf-8 encoding for 'û' (changed in version 2.9)
        KEY_v = 'v',
        KEY_w = 'w',
        KEY_x = 'x',
        KEY_y = 'y',
        KEY_z = 'z',

        KEY_A = 'A',
        KEY_B = 'B',
        KEY_C = 'C',
        KEY_D = 'D',
        KEY_E = 'E',
        KEY_F = 'F',
        KEY_G = 'G',
        KEY_H = 'H',
        KEY_I = 'I',
        KEY_J = 'J',
        KEY_K = 'K',
        KEY_L = 'L',
        KEY_M = 'M',
        KEY_N = 'N',
        KEY_O = 'O',
        KEY_P = 'P',
        KEY_Q = 'Q',
        KEY_R = 'R',
        KEY_S = 'S',
        KEY_T = 'T',
        KEY_U = 'U',
        KEY_V = 'V',
        KEY_W = 'W',
        KEY_X = 'X',
        KEY_Y = 'Y',
        KEY_Z = 'Z',

        PLUS = '+',
        MINUS = '-',
        STAR = '*',
        SLASH = '/',
        LOWER_THAN = '<',
        GREATER_THAN = '>',
        EQUAL = '=',
        PERCENT = '%',

        UNDERSCORE = '_',
        AROBASE = '@',
        SHARP = '#',
        AMPERCENT = '&',
        DOLLAR = '$',
        BACKSLASH = '\\',
        PIPE = '|',
        TILDE = '~',
        SQUARE = 0xc2b2,    //!< Based on utf-8 encoding for '²' (changed in version 2.9)
        EURO = 0xe282ac,    //!< Based on utf-8 encoding for '€' (changed in version 2.9)
        POUND = 0xc2a3,     //!< Based on utf-8 encoding for '£' (changed in version 2.9)
        MICRO = 0xc2b5,     //!< Based on utf-8 encoding for 'µ' (changed in version 2.9)
        PARAGRAPH = 0xc2a7, //!< Based on utf-8 encoding for '§' (changed in version 2.9)
        DEGREE = 0xc2b0,    //!< Based on utf-8 encoding for '°' (changed in version 2.9)
        COPYRIGHT = 0xc2a9, //!< Based on utf-8 encoding for '©' (changed in version 2.9)

        QUESTION = '?',
        EXCLAMATION = '!',
        COLUMN = ':',
        DOT = '.',
        COMA = ',',
        SEMI_COLUMN = ';',
        QUOTE = '\'',
        DOUBLE_QUOTE = '"',
        BACK_QUOTE = '`',

        OPENING_BRACE = '(',
        CLOSING_BRACE = ')',
        OPENING_CURLY_BRACE = '{',
        CLOSING_CURLY_BRACE = '}',
        OPENING_BRACKET = '[',
        CLOSING_BRACKET = ']',

        KEY_UP = 1001,              //!< Up arrow key.
        KEY_DOWN = 1002,            //!< Down arrow key.
        KEY_LEFT = 1003,            //!< Left arrow key.
        KEY_RIGHT = 1004,           //!< Right arrow key.
        PAGE_UP = 1005,             //!< Page up arrow key.
        PAGE_DOWN = 1006,           //!< Page down arrow key.
        PAGE_LEFT = 1007,           //!< Page left arrow key.
        PAGE_RIGHT = 1008,          //!< Page right arrow key.

        KEY_BEGIN = 1020,           //!< Begin key.
        KEY_END = 1021,             //!< End key.

        COPY = 2001,                //!< Copy.
        CUT = 2002,                 //!< Cut.
        PASTE = 2003,               //!< Paste.

        UNDO = 2004,                //!< Undo.
        REDO = 2005,                //!< Redo.
        PREVIOUS = 2006,            //!< Previous key.
        NEXT = 2007,                //!< Forward key.

        F1 = 0x0f000001,
        F2 = 0x0f000002,
        F3 = 0x0f000003,
        F4 = 0x0f000004,
        F5 = 0x0f000005,
        F6 = 0x0f000006,
        F7 = 0x0f000007,
        F8 = 0x0f000008,
        F9 = 0x0f000009,
        F10 = 0x0f00000a,
        F11 = 0x0f00000b,
        F12 = 0x0f00000c,

        FEED_MORE = 0xffffffff,     //!< Return code while analyzing input characters, till the encoding sequence is not fulfilled.
    } KEY;

    //! @brief End of line for input/output devices.
    //! @see endl
    class IOEndl : public Object
    {
    public:
        //! @brief Default constructor.
        explicit IOEndl(void) : Object() {}

    private:
        //! @brief No copy constructor.
        IOEndl(const IOEndl&);
        //! @brief No assignment operator.
        IOEndl& operator=(const IOEndl&);
    };

    //! @brief The common IOEndl object.
    //! @return Common IOEndl object.
    //!
    //! endl can be passed to OutputDevice to print carriage returns.
    extern const IOEndl endl;


    // Forward declarations.
    class StringEncoder;
    class StringDecoder;

    //! @brief Generic output device.
    //! @see endl
    class OutputDevice : public Object
    {
    protected:
        //! @brief Constructor.
        explicit OutputDevice(
            const char* const STR_DbgName,  //!< Debug name. Useful for traces only.
            const bool B_AutoDelete         //!< Auto-deletion flag.
            );

        //! @brief Destructor.
        virtual ~OutputDevice(void);

    private:
        //! @brief No default constructor.
        explicit OutputDevice(void);
        //! @brief No copy constructor.
        OutputDevice(const OutputDevice&);
        //! @brief No assignment operator.
        OutputDevice& operator=(const OutputDevice&);

    public:
        //! @brief Debug name accessor.
        //! @return Debug name.
        const tk::String GetDebugName(void) const;

    public:
        //! @brief Ensures instance validity.
        //! @return Total number of instance users after this call.
        const int UseInstance(
            const CallInfo& CLI_CallInfo    //!< Call information.
            );

        //! @brief Releases the instance.
        //! @return Total number of instance users after this call.
        //!
        //! If the auto-deletion flag has been set during construction,
        //! the object is auto-deleted when the number of users reaches 0 on this call.
        const int FreeInstance(
            const CallInfo& CLI_CallInfo    //!< Call information.
            );

        //! @brief Instance user count accessor.
        //! @return Number of instance users.
        const int GetInstanceUsers(void) const;

    public:
        //! @brief Checks the device is opened.
        //! @return true for success, false otherwise.
        //!
        //! Opens the device if not already opened.
        //! Acquire a lock on the open state in any case.
        const bool OpenUp(
            const CallInfo& CLI_CallInfo    //!< Call information.
            );

        //! @brief Indicates the device the client does not need the device to opened anymore.
        //! @return true for success, false otherwise.
        //!
        //! Releases the lock on the open state.
        //! When no more user need the device to be opened, it is closed straight forward.
        const bool CloseDown(
            const CallInfo& CLI_CallInfo    //!< Call information.
            );

        //! @brief Open state user count accessor.
        //! @return Number of users that have opened the device.
        const int GetOpenUsers(void) const;

    protected:
        //! @brief Device opening handler.
        //! @return true for success, false otherwise.
        //! @note Devices have to be prepared to be called several times through this method.
        //!       They should do the opening once only
        //!       (unless they have been closed between OpendDevice() calls),
        //!       and indicate no failure thereafter.
        virtual const bool OpenDevice(void) = 0;

        //! @brief Device closure handler.
        //! @return true for success, false otherwise.
        //! @note Devices have to be prepared to be called several times through this method.
        //!       They should do the closure once only
        //!       (unless they have been opened between CloseDevice() calls),
        //!       and indicate no failure thereafter.
        virtual const bool CloseDevice(void) = 0;

    public:
        #ifndef CLI_NO_STL
        //! @brief Output operator.
        //! @return The output device itself.
        //! @author [contrib: Oleg Smolsky, 2010, based on CLI 2.5]
        const OutputDevice& operator<<(
            const std::string& STR_Out  //!< Output string object.
            ) const;
        #endif

        //! @brief Output operator.
        //! @return The output device itself.
        const OutputDevice& operator<<(
            const tk::String& STR_Out  //!< Output string object.
            ) const;

        //! @brief Output operator.
        //! @return The output device itself.
        const OutputDevice& operator<<(
            const char* const STR_Out   //!< Output null terminated string.
            ) const;

        //! @brief Output operator.
        //! @return The output device itself.
        const OutputDevice& operator<<(
            const char C_Out            //!< Single character.
            ) const;

        //! @brief Output operator.
        //! @return The output device itself.
        const OutputDevice& operator<<(
            const KEY E_Key             //!< Single character as a KEY.
            ) const;

        //! @brief Output operator.
        //! @return The output device itself.
        const OutputDevice& operator<<(
            const uint8_t UI8_Out       //!< 8 bits unsigned integer number.
            ) const;

        //! @brief Output operator.
        //! @return The output device itself.
        const OutputDevice& operator<<(
            const int16_t I16_Out       //!< 16 bits integer number.
            ) const;

        //! @brief Output operator.
        //! @return The output device itself.
        const OutputDevice& operator<<(
            const uint16_t UI16_Out     //!< 16 bits unsigned integer number.
            ) const;

        //! @brief Output operator.
        //! @return The output device itself.
        const OutputDevice& operator<<(
            const int32_t I32_Out       //!< 32 bits integer number.
            ) const;

        //! @brief Output operator.
        //! @return The output device itself.
        const OutputDevice& operator<<(
            const uint32_t UI32_Out     //!< 32 bits unsigned integer number.
            ) const;

        //! @brief Output operator.
        //! @return The output device itself.
        const OutputDevice& operator<<(
            const int64_t I64_Out       //!< 64 bits integer number.
            ) const;

        //! @brief Output operator.
        //! @return The output device itself.
        const OutputDevice& operator<<(
            const uint64_t UI64_Out     //!< 64 bits unsigned integer number.
            ) const;

        //! @brief Output operator.
        //! @return The output device itself.
        const OutputDevice& operator<<(
            const float F_Out           //!< Float number.
            ) const;

        //! @brief Output operator.
        //! @return The output device itself.
        const OutputDevice& operator<<(
            const double D_Out          //!< Double number.
            ) const;

        //! @brief Output operator.
        //! @return The output device itself.
        const OutputDevice& operator<<(
            const void* const PV_Out    //!< Void address.
            ) const;

        //! @brief Output operator.
        //! @return The output device itself.
        const OutputDevice& operator<<(
            const IOEndl& CLI_IOEndl    //!< Carriage return.
            ) const;

    public:
        //! @brief Last error accessor.
        //! @return Last error resource string.
        const ResourceString GetLastError(void) const;

    public:
        //! @brief Null device singleton.
        //! @return Null device reference.
        static OutputDevice& GetNullDevice(void);

        //! @brief Standard output device singleton.
        //! @return Standard output device reference.
        static OutputDevice& GetStdOut(void);

        //! @brief Standard error device singleton.
        //! @return Standard error device reference.
        static OutputDevice& GetStdErr(void);

    public:
        //! @brief Screen information.
        class ScreenInfo : public Object
        {
        public:
            //! @brief ScreenInfo regular values.
            enum _RegularValues {
                UNKNOWN = -1,           //!< Unknown value constant for either width or height.
                DEFAULT_WIDTH = 80,     //!< Default width constant.
                DEFAULT_HEIGHT = 20     //!< Default height constant.
            };

        private:
            //! No default constructor.
            explicit ScreenInfo(void);

        public:
            //! @brief Constructor.
            explicit ScreenInfo(
                const int I_Width,      //!< Width of screen. Can be UNKNOWN.
                const int I_Height,     //!< Height of screen. Can be UNKNOWN.
                const bool B_TrueCls,   //!< True when an efficient CleanScreen() operation is implemented.
                const bool B_WrapLines  //!< True when the line automatically goes down when the cursor reached the right end of the screen.
                ) : Object(),
                    m_iWidth(I_Width), m_iHeight(I_Height),
                    m_bTrueCls(B_TrueCls), m_bWrapLines(B_WrapLines)
            {}
            //! @brief Copy constructor.
            ScreenInfo(
                const ScreenInfo& CLI_Info      //!< Dimension object to copy.
                ) : Object(),
                    m_iWidth(CLI_Info.GetWidth()), m_iHeight(CLI_Info.GetHeight()),
                    m_bTrueCls(CLI_Info.GetbTrueCls()), m_bWrapLines(CLI_Info.GetbWrapLines())
            {}
            //! @brief Destructor.
            virtual ~ScreenInfo(void) {}
        public:
            //! @brief Assignment operator.
            //! @return Same ScreenInfo instance reference.
            ScreenInfo& operator=(
                const ScreenInfo& CLI_ScreenInfo    //!< Screen information to copy.
                )
            {
                m_iWidth = CLI_ScreenInfo.m_iWidth;
                m_iHeight = CLI_ScreenInfo.m_iHeight;
                m_bTrueCls = CLI_ScreenInfo.m_bTrueCls;
                m_bWrapLines = CLI_ScreenInfo.m_bWrapLines;
                return *this;
            }
        public:
            //! @brief Screen width accessor.
            //! @return Screen width if known, UNKNOWN otherwise.
            const int GetWidth(void) const { return ((m_iWidth > 0) ? m_iWidth : UNKNOWN); }
            //! @brief Safe screen width accessor.
            //! @return Screen width if known, default value otherwise.
            const unsigned int GetSafeWidth(void) const { return ((m_iWidth > 0) ? m_iWidth : DEFAULT_WIDTH); }
            //! @brief Screen height accessor.
            //! @return Screen height if known, UNKNOWN otherwise.
            const int GetHeight(void) const { return ((m_iHeight > 0) ? m_iHeight : UNKNOWN); }
            //! @brief Safe screen height accessor.
            //! @return Screen height if known, default value otherwise.
            const unsigned int GetSafeHeight(void) const { return ((m_iHeight > 0) ? m_iHeight : DEFAULT_HEIGHT); }
            //! @brief True CleanScreen() characteristic accessor.
            //! @return True CleanScreen() characteristic.
            const bool GetbTrueCls(void) const { return m_bTrueCls; }
            //! @brief Line wrapping characteristic accessor.
            //! @return Line wrapping characteristic.
            const bool GetbWrapLines(void) const { return m_bWrapLines; }
        private:
            int m_iWidth;       //!< Page width.
            int m_iHeight;      //!< Page height.
            bool m_bTrueCls;    //!< True CleanScreen() implementation.
            bool m_bWrapLines;  //!< Line wrapping.
        };

    public:
        // Note: use of @param doxygen tag in order to avoid doxygen warnings for reimplementations in sub-classes.
        //! @brief Output handler.
        //! @param STR_Out Output string.
        virtual void PutString(const char* const STR_Out) const = 0;

        //! @brief Beep handler.
        virtual void Beep(void) const;

        //! @brief Clean screen.
        virtual void CleanScreen(void) const;

        //! @brief Screen info accessor.
        //! @return Screen info with possible ScreenInfo::UNKNOWN values.
        virtual const ScreenInfo GetScreenInfo(void) const;

        // Note: use of @param doxygen tag in order to avoid doxygen warnings for reimplementations in sub-classes.
        //! @brief Stack overflow protection.
        //! @param CLI_Device Other device that the callee device should check it would output characters to.
        //! @return true if the callee device would redirect characters to the given device for output.
        //!
        //! Determines whether the current device would output the given device in any way.
        //! Default implementation checks whether CLI_Device is the self device.
        virtual const bool WouldOutput(const OutputDevice& CLI_Device) const;

    private:
        //! Debug name. Useful for traces only.
        const tk::String m_strDebugName;

        //! Instance lock count.
        int m_iInstanceLock;

        //! Open state lock count.
        int m_iOpenLock;

        //! String encdoing.
        StringEncoder& m_cliStringEncoder;

    protected:
        //! Last error.
        //! @return N/A (doxygen warning)
        mutable ResourceString m_cliLastError;
    };


    //! @brief Generic input/output device.
    //! @see endl
    class IODevice : public OutputDevice
    {
    public:
        //! @brief Constructor.
        explicit IODevice(
            const char* const STR_DbgName,  //!< Debug name.
            const bool B_AutoDelete         //!< Auto-deletion flag.
            );

        //! @brief Destructor.
        virtual ~IODevice(void);

    private:
        //! @brief No default constructor.
        explicit IODevice(void);
        //! @brief No copy constructor.
        IODevice(const IODevice&);
        //! @brief No assignment operator.
        IODevice& operator=(const IODevice&);

    public:
        //! @brief Input key capture handler.
        //! @warning Blocking call.
        //! @return KEY code captured.
        virtual const KEY GetKey(void) const = 0;

        //! @brief Input location accessor.
        //! @return Input location resource string.
        virtual const ResourceString GetLocation(void) const;

        // Note: use of @param doxygen tag in order to avoid doxygen warnings for reimplementations in sub-classes.
        //! @brief Stack overflow protection.
        //! @param CLI_Device Other device that the callee device should check it would input characters from.
        //! @return true if the callee device would redirect input to the given device for reading.
        //!
        //! Determines whether the current device would input the given device in any ways.
        //! Default implementation checks whether CLI_Device is the self device.
        virtual const bool WouldInput(const IODevice& CLI_Device) const;

    public:
        //! @brief Null device singleton.
        //! @return Null device reference.
        static IODevice& GetNullDevice(void);

        //! @brief Standard input device singleton.
        //! @return Standard input device reference.
        static IODevice& GetStdIn(void);

    protected:
        //! @brief Common char translation.
        //! @return KEY code corresponding to the given common char,
        //!         FEED_MORE when the character has been analyzed as part of a sequence, and the end of the sequence should be given to determine the actual KEY.
        //! @note Routine explicitely renamed in order to cause compilation errors due to the addition of the FEED_MORE return code.
        const KEY Char2Key(
            const int I_Char    //!< Common char to translate.
            ) const;
    private:
        //! String decoding.
        StringDecoder& m_cliStringDecoder;
    };

CLI_NS_END(cli)

#endif // _CLI_IO_DEVICE_H_

