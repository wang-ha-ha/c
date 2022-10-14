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
//! @brief Param and ParamT classes definition.

#ifndef _CLI_PARAM_H_
#define _CLI_PARAM_H_

#include "cli/namespace.h"
#include "cli/syntax_node.h"
#include "cli/tk.h"


CLI_NS_BEGIN(cli)

    // Forward declarations.
    class Help;


    //! @brief Base parameter class.
    //!
    //! Base class for any kind of parameter elements.
    //! Not final class.
    class Param : public SyntaxNode
    {
    protected:
        //! @brief Constructor.
        explicit Param(
            const char* const STR_Keyword,  //!< Keyword.
                                            //!< Does not mean much for a parameter.
                                            //!< Something like a description of the type of parameter.
            const Help& CLI_Help            //!< Corresponding help.
            );

    public:
        //! @brief Destructor.
        virtual ~Param(void);

    private:
        //! @brief No default constructor.
        explicit Param(void);
        //! @brief No copy constructor.
        Param(const Param&);
        //! @brief No assignment operator.
        Param& operator=(const Param&);

    public:
        // Inherit doxygen comments from cli::Element documentation.
        virtual const tk::String GetKeyword(void) const;

        // Inherit doxygen comments from cli::Element documentation.
        virtual const bool FindElements(Element::List& CLI_ExactList, Element::List& CLI_NearList, const char* const STR_Keyword) const;

        // Note: use of @param doxygen tag in order to avoid doxygen warnings for reimplementations in sub-classes.
        //! @brief Value setting.
        //! @param STR_Value New value.
        //! @return true if the value has been set correctly, false otherwise.
        //!
        //! To be overloaded by derived classes.
        virtual const bool SetstrValue(const char* const STR_Value) const = 0;

        //! @brief Value access in its string form.
        //! @return Parameter value in its string form.
        const tk::String GetstrValue(void) const;

        //! @brief Parameter cloning handler.
        //! @return A newly created parameter object of the correct type.
        //!
        //! To be overloaded by derived classes.
        virtual const Param* const Clone(void) const = 0;

        //! @brief Cloned parameter access.
        //! @return Cloned parameter instance if any. NULL otherwise.
        const Param* const GetCloned(void) const;

        // Note: use of @param doxygen tag in order to avoid doxygen warnings for reimplementations in sub-classes.
        //! @brief Value copy handler.
        //! @param CLI_Param Parameter to copy the value from.
        //! @return The Param instance itself.
        virtual const Param& CopyValue(const Param& CLI_Param) const = 0;

    protected:
        //! @brief Value setting from derived class.
        //! @return true for success, false otherwise.
        const bool SetValue(
            const char* const STR_Value     //!< New value.
            ) const;

        //! @brief Clone initialization.
        //! @return CLI_CloneParam reference.
        const Param* const InitClone(
            Param& CLI_CloneParam           //!< Clone parameter to initialize.
            ) const;

        //! @brief Cloned parameter reference setting.
        //! @return true for success, false otherwise.
        const bool SetCloned(
            const Param& CLI_Cloned         //!< Clone parameter reference.
            );

    private:
        //! Value in its string form.
        mutable tk::String m_strValue;

        //! Cloned parameter reference.
        const Param* m_pcliCloned;
    };


    //! @brief Template parameter class.
    template <class T> class ParamT : public Param
    {
    public:
        //! @brief Constructor.
        explicit ParamT<T>(
                const char* const STR_Keyword,  //!< Keyword.
                const T& T_Default,             //!< Default value.
                const Help& CLI_Help            //!< Corresponding help.
                )
          : Param(STR_Keyword, CLI_Help),
            m_tValue(T_Default)
        {
        }

        //! @brief Destructor.
        virtual ~ParamT<T>(void)
        {
        }

    private:
        //! @brief No default constructor.
        explicit ParamT<T>(void);
        //! @brief No copy constructor.
        ParamT<T>(const ParamT<T>&);
        //! @brief No assignment operator.
        ParamT<T>& operator=(const ParamT<T>&);

    public:
        //! @brief Implicit cast operator.
        operator const T(void) const
        {
            return m_tValue;
        }

        // Inherit doxygen comments from cli::Param documentation.
        virtual const Param& CopyValue(const Param& CLI_Param) const
        {
            if (const ParamT<T>* const pcli_Param = dynamic_cast<const ParamT<T>*>(& CLI_Param))
            {
                SetValue(CLI_Param.GetKeyword(), *pcli_Param);
            }
            return *this;
        }

    protected:
        //! @brief Value setting for derived class.
        void SetValue(
            const char* const STR_Value,    //!< New value in its string form.
            const T& T_Value                //!< New value in its typed form.
            ) const
        {
            Param::SetValue(STR_Value);
            m_tValue = T_Value;
        }

    private:
        //! Controlled value.
        mutable T m_tValue;
    };

CLI_NS_END(cli)

#endif // _CLI_PARAM_H_

