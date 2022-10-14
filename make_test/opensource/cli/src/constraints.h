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
//! @brief Object consistency insurance.

#ifndef _CLI_CONSTRAINTS_H_
#define _CLI_CONSTRAINTS_H_

#include "cli/namespace.h"


CLI_NS_BEGIN(cli)

    // General.

    //! Maximum length of resource strings, for each language.
    #ifndef CLI_MAX_RESOURCE_LENGTH
    static const unsigned int MAX_RESOURCE_LENGTH = 256;
    #else
    static const unsigned int MAX_RESOURCE_LENGTH = CLI_MAX_RESOURCE_LENGTH;
    #endif

    //! Maximum file path length.
    #ifndef CLI_MAX_FILE_PATH_LENGTH
    static const unsigned int MAX_FILE_PATH_LENGTH = 256;
    #else
    static const unsigned int MAX_FILE_PATH_LENGTH = CLI_MAX_FILE_PATH_LENGTH;
    #endif

    // CLI structures.

    //! Maximum number of menus per cli.
    #ifndef CLI_MAX_MENU_PER_CLI
    static const unsigned int MAX_MENU_PER_CLI = 50;
    #else
    static const unsigned int MAX_MENU_PER_CLI = CLI_MAX_MENU_PER_CLI;
    #endif

    //! Maximum number of words per syntax node.
    #ifndef CLI_MAX_WORDS_PER_NODE
    static const unsigned int MAX_WORDS_PER_NODE = 256;
    #else
    static const unsigned int MAX_WORDS_PER_NODE = CLI_MAX_WORDS_PER_NODE;
    #endif

    //! Maximum number of comment patterns.
    #ifndef CLI_MAX_COMMENT_PATTERNS_PER_CLI
    static const unsigned int MAX_COMMENT_PATTERNS_PER_CLI = 256;
    #else
    static const unsigned int MAX_COMMENT_PATTERNS_PER_CLI = CLI_MAX_COMMENT_PATTERNS_PER_CLI;
    #endif

    // Command lines.

    //! Maximum string length for command lines.
    #ifndef CLI_MAX_CMD_LINE_LENGTH
    static const unsigned int MAX_CMD_LINE_LENGTH = 2048;
    #else
    static const unsigned int MAX_CMD_LINE_LENGTH = CLI_MAX_CMD_LINE_LENGTH;
    #endif

    //! Maximum number of words in a command line.
    #ifndef CLI_MAX_CMD_LINE_WORD_COUNT
    static const unsigned int MAX_CMD_LINE_WORD_COUNT = 256;
    #else
    static const unsigned int MAX_CMD_LINE_WORD_COUNT = CLI_MAX_CMD_LINE_WORD_COUNT;
    #endif

    //! Maximum length for each word in a command line. Used for comment patterns as well.
    #ifndef CLI_MAX_WORD_LENGTH
    static const unsigned int MAX_WORD_LENGTH = 256;
    #else
    static const unsigned int MAX_WORD_LENGTH = CLI_MAX_WORD_LENGTH;
    #endif

    // Traces / debug.

    //! Maximum number of devices that the traces system can stack.
    #ifndef CLI_MAX_TRACE_DEVICE_COUNT
    static const unsigned int MAX_TRACE_DEVICE_COUNT = 128;
    #else
    static const unsigned int MAX_TRACE_DEVICE_COUNT = CLI_MAX_TRACE_DEVICE_COUNT;
    #endif

    //! Maximum number of trace classes that the traces system manages.
    #ifndef CLI_MAX_TRACE_CLASS_COUNT
    static const unsigned int MAX_TRACE_CLASS_COUNT = 1024;
    #else
    static const unsigned int MAX_TRACE_CLASS_COUNT = CLI_MAX_TRACE_CLASS_COUNT;
    #endif

    //! Maximum length of a trace class name.
    #ifndef CLI_MAX_TRACE_CLASS_NAME_LENGTH
    static const unsigned int MAX_TRACE_CLASS_NAME_LENGTH = MAX_WORD_LENGTH;
    #else
    static const unsigned int MAX_TRACE_CLASS_NAME_LENGTH = CLI_MAX_TRACE_CLASS_NAME_LENGTH;
    #endif

    //! Maximum length of a device name (for debug purpose).
    #ifndef CLI_MAX_DEVICE_NAME_LENGTH
    static const unsigned int MAX_DEVICE_NAME_LENGTH = 256;
    #else
    static const unsigned int MAX_DEVICE_NAME_LENGTH = CLI_MAX_DEVICE_NAME_LENGTH;
    #endif

    // Misc

    //! Maximum number of cli instances that the cli registry manages.
    #ifndef CLI_MAX_CLI_REGISTRY_COUNT
    static const unsigned int MAX_CLI_REGISTRY_COUNT = 10;
    #else
    static const unsigned int MAX_CLI_REGISTRY_COUNT = MAX_CLI_CLI_REGISTRY_COUNT;
    #endif

    //! Maximum number of input devices that an IOMux instance manages.
    #ifndef CLI_MAX_IO_MUX_INPUTS
    static const unsigned int MAX_IO_MUX_INPUTS = 100;
    #else
    static const unsigned int MAX_IO_MUX_INPUTS = CLI_MAX_IO_MUX_INPUTS;
    #endif

    //! Maximum number of execution contexts.
    #ifndef CLI_MAX_EXECUTION_CONTEXTS
    static const unsigned int MAX_EXECUTION_CONTEXTS = 10;
    #else
    static const unsigned int MAX_EXECUTION_CONTEXTS = CLI_MAX_EXECUTION_CONTEXTS;
    #endif

CLI_NS_END(cli)

#endif // _CLI_CONSTRAINTS_H_
