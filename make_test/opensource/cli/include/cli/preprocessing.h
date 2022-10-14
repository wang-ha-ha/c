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
//! @brief Pre-processing constants listing.
//! @warning This file is not supposed to be included.
//!
//! This file is not supposed to be included.
//! It just intends to list all configuration preprocessing constants for doxygen documentation.
#error Do not include preprocessing.h

#ifndef _CLI_PREPROCESSING_H_
#define _CLI_PREPROCESSING_H_


    //! @brief Namespace disabling.
    //!
    //! Set this constant to disable cli and cli::tk namespaces.
    //!
    //! Define this constant if your compiler does not support namespaces.
    //! Classes will be used instead.
    #define CLI_NO_NAMESPACE                <set to disable namespaces>

    //! @brief Debug preprocessing constant.
    //!
    //! Set this constant to activate debug utilities.
    #define _DEBUG                          <set to enable debug utilities>

    //! @brief CLI assertion macro.
    #define CLI_ASSERT(a)                   <set to override default implementation>

    //! @brief STL implementation disabling.
    //!
    //! Define this constant if you wish to use inner rather than the STL TK implementation.
    #define CLI_NO_STL                      <use inner instead of STL for tk objects>

    //! @brief Regular expressions disabling.
    //!
    //! Defining this constant will disable regular expressions.
    #define CLI_NO_REGEX                    <set to disable regular expressions>

    //! @brief Size of telnet input buffer.
    //! @note Useless when STL toolkit implementation is used.
    #define CLI_TELNET_INPUT_BUFFER_SIZE    <set to adjust telnet input buffer>

    //! @brief Windows networking enabling.
    //!
    //! Define this constant if you wish to use Windows Socket API.
    //! BSD socket API otherwise.
    #define CLI_WIN_NETWORK                 <set to enable Windows networking>

// Constraints
    //! @brief Maximum number of Cli objects in registry.
    //! @note Useless with STL implementation.
    //! @see constraints.h
    #define CLI_MAX_CLI_REGISTRY_COUNT      <adjust constraint>

    //! @brief Maximum number of menus per Cli object.
    //! @note Useless with STL implementation.
    //! @see constraints.h
    #define CLI_MAX_MENU_PER_CLI            <adjust constraint>

    //! @brief Maximum length of command lines.
    //! @note Useless with STL implementation.
    //! @see constraints.h
    #define CLI_MAX_CMD_LINE_LENGTH         <adjust constraint>

    //! @brief Maximum number of words per command line.
    //! @note Useless with STL implementation.
    //! @see constraints.h
    #define CLI_MAX_CMD_LINE_WORD_COUNT     <adjust constraint>

    //! @brief Maximum length of each word.
    //! @note Useless with STL implementation.
    //! @see constraints.h
    #define CLI_MAX_WORD_LENGTH             <adjust constraint>

    //! @brief Maximum device name length.
    //! @note Useless with STL implementation.
    //! @note Useless in release mode.
    //! @see constraints.h
    #define CLI_MAX_DEVICE_NAME_LENGTH      <adjust constraint>

    //! @brief Maximum file path length.
    //! @note Useless with STL implementation.
    //! @see constraints.h
    #define CLI_MAX_FILE_PATH_LENGTH        <adjust constraint>

    //! @brief Maximum input devices per IO Multiplexer device.
    //! @note Useless with STL implementation.
    //! @see constraints.h
    #define CLI_MAX_IO_MUX_INPUTS           <adjust constraint>

    //! @brief Maximum number of words per syntax node.
    //! @note Useless with STL implementation.
    //! @see constraints.h
    #define CLI_MAX_WORDS_PER_NODE          <adjust constraint>

    //! @brief Maximum resource string length.
    //! @note Useless with STL implementation.
    //! @see constraints.h
    #define CLI_MAX_RESOURCE_LENGTH         <adjust constraint>

    //! @brief Maximum number of trace classes managed by the trace system.
    //! @note Useless with STL implementation.
    //! @see constraints.h
    #define CLI_MAX_TRACE_CLASS_COUNT       <adjust constraint>

    //! @brief Maximum length of trace class names.
    //! @note Useless with STL implementation.
    //! @see constraints.h
    #define CLI_MAX_TRACE_CLASS_NAME_LENGTH <adjust constraint>

#endif // _CLI_PREPROCESSING_H_
