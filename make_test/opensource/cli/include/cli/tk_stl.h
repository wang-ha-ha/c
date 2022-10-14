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
//! @brief STL toolkit definition.

#ifndef _CLI_TK_STL_H_
#define _CLI_TK_STL_H_

#include <ctype.h> // toupper, tolower
#include <string.h> // memset

#include <string>
#include <deque>
#include <map>

#include "cli/object.h"


CLI_NS_BEGIN(cli)

    //! @brief CLI classes toolkit.
    CLI_NS_BEGIN(tk)

        //! @brief Basic string object.
        class String : public cli::Object
        {
        public:
            //! @brief Concatenation.
            //! @return Result string.
            static const tk::String Concat(
                    const unsigned int UI_MaxLength,    //!< Maximum length.
                    const char* const STR_1,            //!< First string.
                    const char* const STR_2             //!< Second string.
                    )
            {
                tk::String str_Result(UI_MaxLength);
                str_Result.Append(STR_1);
                str_Result.Append(STR_2);
                return str_Result;
            }

            //! @brief Concatenation.
            //! @return Result string.
            static const tk::String Concat(
                    const unsigned int UI_MaxLength,    //!< Maximum length.
                    const char* const STR_1,            //!< First string.
                    const char* const STR_2,            //!< Second string.
                    const char* const STR_3             //!< Third string.
                    )
            {
                tk::String str_Result(Concat(UI_MaxLength, STR_1, STR_2));
                str_Result.Append(STR_3);
                return str_Result;
            }

            //! @brief Concatenation.
            //! @return Result string.
            static const tk::String Concat(
                    const unsigned int UI_MaxLength,    //!< Maximum length.
                    const char* const STR_1,            //!< First string.
                    const char* const STR_2,            //!< Second string.
                    const char* const STR_3,            //!< Third string.
                    const char* const STR_4             //!< Fourth string.
                    )
            {
                tk::String str_Result(Concat(UI_MaxLength, STR_1, STR_2, STR_3));
                str_Result.Append(STR_4);
                return str_Result;
            }

            //! @brief Concatenation.
            //! @return Result string.
            static const tk::String Concat(
                    const unsigned int UI_MaxLength,    //!< Maximum length.
                    const char* const STR_1,            //!< First string.
                    const char* const STR_2,            //!< Second string.
                    const char* const STR_3,            //!< Third string.
                    const char* const STR_4,            //!< Fourth string.
                    const char* const STR_5             //!< Fifth string.
                    )
            {
                tk::String str_Result(Concat(UI_MaxLength, STR_1, STR_2, STR_3, STR_4));
                str_Result.Append(STR_5);
                return str_Result;
            }

        private:
            //! @brief No default constructor.
            explicit String(void);

        public:
            //! @brief Constructor.
            explicit String(
                    const unsigned int UI_MaxLen    //!< Maximum string length.
                    )
              : Object(), m_stlString()
            {
                UnusedParameter(UI_MaxLen); // [contrib: Oleg Smolsky, 2010, based on CLI 2.5]
            }

            //! @brief Initial value constructor.
            //! @warning Maximum length inherits the initial string value.
            //!         Objects initialized by this constructor might be constants in general.
            explicit String(
                    const unsigned int UI_MaxLen,   //!< Maximum string length.
                    const char* const STR_String    //!< Initial value.
                    )
              : Object(), m_stlString(STR_String)
            {
                UnusedParameter(UI_MaxLen); // [contrib: Oleg Smolsky, 2010, based on CLI 2.5]
            }

            //! @brief Copy constructor.
            String(
                    const String& TK_String //!< Source object.
                    )
              : Object(), m_stlString((const char* const) TK_String)
            {
            }

            //! @brief Destructor.
            virtual ~String(void)
            {
            }

        public:
            //! @brief String length accessor.
            //! @return String length.
            const unsigned int GetLength(void) const
            {
                return (unsigned int) m_stlString.size();
            }

            //! @brief Checks whether the string is empty or not.
            //! @return true for an empty string, false otherwise.
            const bool IsEmpty(void) const
            {
                return (GetLength() <= 0);
            }

            //! @brief Conversion operator.
            operator const char* const(void) const
            {
                return m_stlString.c_str();
            }

            //! @brief Single character accessor.
            //! @return Character at the given position. Null character when the position is out of bounds.
            const char GetChar(
                const unsigned int UI_Pos               //!< Character position from the beginning of the string.
                ) const
            {
                char c_Char = '\0';
                if (UI_Pos < m_stlString.size())
                {
                    c_Char = m_stlString[UI_Pos];
                }
                return c_Char;
            }

            //! @brief Sub-string computation.
            //! @return Computed sub-string.
            const tk::String SubString(
                    const unsigned int UI_FirstCharacter,   //!< First character position.
                    const int I_SubStringLength             //!< Sub-string length.
                                                            //!< negative values mean 0.
                    ) const
            {
                tk::String str_SubString(0);

                // Determine copy length.
                unsigned int ui_CopyLen = I_SubStringLength;
                if (GetLength() - UI_FirstCharacter < ui_CopyLen)
                    ui_CopyLen = GetLength() - UI_FirstCharacter;

                // Copy.
                if ((UI_FirstCharacter < GetLength()) && (ui_CopyLen > 0))
                {
                    str_SubString.Append(m_stlString.substr(UI_FirstCharacter, ui_CopyLen).c_str());
                }

                return str_SubString;
            }

            //! @brief Upper string transformation.
            //! @return Upper string computed.
            const String ToUpper(void) const
            {
                tk::String str_Upper(0);

                const unsigned int ui_StrLen = GetLength() + 1;
                if (char* const pc_Upper = new char[ui_StrLen])
                {
                    memset(pc_Upper, '\0', ui_StrLen);
                    if (const char* const pc_String = (const char*) *this) {
                        for (unsigned int ui=0; ui<ui_StrLen; ui++)
                        {
                            pc_Upper[ui] = toupper(pc_String[ui]);
                        }
                    }
                    str_Upper.Append(pc_Upper);
                    delete [] pc_Upper;
                }

                return str_Upper;
            }

            //! @brief Lower string transformation.
            //! @return Lower string computed.
            const String ToLower(void) const
            {
                tk::String str_Lower(0);

                const unsigned int ui_StrLen = GetLength() + 1;
                if (char* const pc_Lower = new char[ui_StrLen])
                {
                    memset(pc_Lower, '\0', ui_StrLen);
                    if (const char* const pc_String = (const char*) *this)
                    {
                        for (unsigned int ui=0; ui<ui_StrLen; ui++)
                        {
                            pc_Lower[ui] = tolower(pc_String[ui]);
                        }
                    }
                    str_Lower.Append(pc_Lower);
                    delete [] pc_Lower;
                }

                return str_Lower;
            }

        public:
            //! @brief String resetting.
            //! @return true when success, false otherwise.
            const bool Reset(void)
            {
                m_stlString.erase();
                return true;
            }

            //! @brief String setting.
            //! @return true when success, false otherwise.
            const bool Set(
                    const char* const STR_String    //!< String to set.
                    )
            {
                m_stlString = STR_String;
                return true;
            }

            //! @brief String appending.
            //! @return true when success, false otherwise.
            const bool Append(
                    const char* const STR_String    //!< String to append.
                    )
            {
                m_stlString += STR_String;
                return true;
            }

            //! @brief String appending.
            //! @return true when success, false otherwise.
            const bool Append(
                    const char C_Character          //!< Character to append.
                    )
            {
                m_stlString += C_Character;
                return true;
            }

            //! @brief Assignment operator.
            //! @return The string just assigned itself.
            //! @warning Prefer Set() method since this method does not indicate failures.
            tk::String& operator=(
                    const tk::String& STR_String    //!< Source string used for assignment.
                    )
            {
                m_stlString = STR_String.m_stlString;
                return *this;
            }

        private:
            //! String buffer.
            std::string m_stlString;
        };


        //! @brief Basic queue object.
        template <class T> class Queue : public cli::Object
        {
        private:
            //! @brief No default constructor.
            explicit Queue(void);

        public:
            //! @brief Main constructor.
            explicit Queue(
                    const unsigned int UI_MaxCount  //!< Maximum item count.
                    )
              : Object(), m_stlQueue()
            {
                UnusedParameter(UI_MaxCount); // [contrib: Oleg Smolsky, 2010, based on CLI 2.5]
            }

            //! @brief Copy constructor.
            Queue(
                    const Queue<T>& TK_Queue    //!< Source queue object.
                    )
              : Object(), m_stlQueue(TK_Queue.m_stlQueue)
            {
            }

            //! @brief Destructor.
            virtual ~Queue(void)
            {
            }

        private:
            //! @brief No assignment operator.
            Queue& operator=(const Queue&);

        public:
            //! @brief Determines whether the queue is empty.
            //! @return true when the queue is empty, false otherwise.
            const bool IsEmpty(void) const
            {
                return m_stlQueue.empty();
            }

            //! @brief Item count.
            //! @return Number of items in the queue.
            const unsigned int GetCount(void) const
            {
                return (unsigned int) m_stlQueue.size();
            }

        public:
            //! @brief Resets the queue.
            //! @return true when success, false otherwise.
            //! @author [contrib: Oleg Smolsky, 2010, based on CLI 2.5]
            const bool Reset(void)
            {
                m_stlQueue.clear();
                return true;
            }

        public:
            //! @brief Iterator object.
            class Iterator : public std::deque<T>::iterator
            {
            private:
                //! @brief Default constructor.
                explicit Iterator(void) : std::deque<T>::iterator() {}

            public:
                //! @brief Copy constructor from an STL iterator.
                //! @note This is convenient for implicitely converting STL objects to CLI objects.
                template <class U>
                Iterator(
                        const U& it     //!< STL iterator used for initialization.
                        )
                  : std::deque<T>::iterator(it) {}
            };

            //! @brief Iterator retrieval.
            //! @return Iterator instance.
            Iterator GetIterator(void) const
            {
                return const_cast<std::deque<T>&>(m_stlQueue).begin();
            }

            //! @brief Checks the element at the given position is valid.
            //! @return true if the iterator is at a valid place, false otherwise.
            const bool IsValid(
                    const Iterator& it      //!< Iterator to check.
                    ) const
            {
                return (it != m_stlQueue.end());
            }

            //! @brief Iterates backward the iterator.
            //! @return true if the iterator has moved to a valid place, false otherwise.
            const bool MovePrevious(
                    Iterator& it            //!< Iterator to move backward.
                    ) const
            {
                if (it != m_stlQueue.begin())
                {
                    it --;
                    return true;
                }
                else
                {
                    return false;
                }
            }

            //! @brief Iterates forward the iterator.
            //! @return true if the iterator has moved to a valid place, false otherwise.
            const bool MoveNext(
                    Iterator& it            //!< Iterator to move forward.
                    ) const
            {
                it ++;
                return IsValid(it);
            }

            //! @brief Read-only item retrieval.
            //! @return Read-only item retrieved.
            const T& GetAt(
                    const Iterator& it      //!< Current iterator.
                    ) const
            {
                return *it;
            }

            //! @brief Modifiable item retrieval.
            //! @return Modifiable item retrieved.
            T& GetAt(
                    const Iterator& it      //!< Current iterator.
                    )
            {
                return *it;
            }

            //! @brief Item removal.
            //! @param it Position. Set to next item.
            //! @return The removed element.
            const T Remove(
                    Iterator& it            //!< Current iterator.
                    )
            {
                /*
		const T t_Element = *it;
                it = m_stlQueue.erase(it);
                return t_Element;
		*/ //by sunyang
		const T t_Element = *it;
		int i_Num = 0;
		for(Iterator it_1 = this->GetIterator();
				this->IsValid(it_1);
				this->MoveNext(it_1))
		{
			if(it_1 == it)
			{
				break;
			}

			i_Num ++;
		}

		it = m_stlQueue.erase(m_stlQueue.begin() + i_Num);
		return t_Element;
            }

        public:
            //! @brief Add a new element at the head of the queue.
            //! @return true if the element has been added, false otherwise.
            const bool AddHead(
                    const T& T_Element          //!< New element.
                    )
            {
                m_stlQueue.push_front(T_Element);
                return true;
            }

            //! @brief Add a new element at the tail of the queue.
            //! @return true if the element has been added, false otherwise.
            const bool AddTail(
                    const T& T_Element          //!< New element.
                    )
            {
                m_stlQueue.push_back(T_Element);
                return true;
            }

            //! @brief First item accessor of the read-only queue.
            //! @warning Do not call on an empty queue.
            //! @return Read-only head element.
            const T& GetHead(void) const
            {
                return m_stlQueue.front();
            }

            //! @brief First item accessor of the modifiable queue.
            //! @warning Do not call on an empty queue.
            //! @return Modifiable head element.
            T& GetHead(void)
            {
                return const_cast<T&>(
                    const_cast<const Queue<T>*>(this)->GetHead()
                );
            }

            //! @brief Last item accessor of the read-only queue.
            //! @warning Do not call on an empty queue.
            //! @return Read-only tail element.
            const T& GetTail(void) const
            {
                return m_stlQueue.back();
            }

            //! @brief Last item accessor of the modifiable queue.
            //! @warning Do not call on an empty queue.
            //! @return Modifiable tail element.
            T& GetTail(void)
            {
                return const_cast<T&>(
                    const_cast<const Queue<T>*>(this)->GetTail()
                );
            }

            //! @brief Add a new element at the head of the queue.
            //! @warning Do not call on an empty queue.
            //! @return Element removed.
            const T RemoveHead(void)
            {
                const T t_Element = m_stlQueue.front();
                m_stlQueue.pop_front();
                return t_Element;
            }

            //! @brief Add a new element at the tail of the queue.
            //! @warning Do not call on an empty queue.
            //! @return Element removed.
            const T RemoveTail(void)
            {
                const T t_Element = m_stlQueue.back();
                m_stlQueue.pop_back();
                return t_Element;
            }

        public:
            //! @brief Sort the list according to the given comparison function.
            //! @return true when success, false otherwise.
            const bool Sort(
                    const int (*cmp)(const T&, const T&)    //!< Comparison function.
                                                                //!< Return positive value when then second argument should follow first one.
                    )
            {
                bool b_Result = true;
                if (GetCount() < 2)
                {
                    b_Result = true;
                }
                else
                {
                    // Get a reference element.
                    T t_Ref = RemoveTail();

                    // Dispatch elements in q_1 and q_2.
                    Queue<T> q_1(0), q_2(0);
                    while (! IsEmpty())
                    {
                        T t_Sort = RemoveTail();
                        if (cmp(t_Sort, t_Ref) > 0)
                        {
                            q_1.AddTail(t_Sort);
                        }
                        else
                        {
                            q_2.AddTail(t_Sort);
                        }
                    }

                    // Sort each list q_1 and q_2
                    if (q_1.Sort(cmp) && q_2.Sort(cmp))
                    {
                        // Eventually restore q_1...
                        b_Result = true;
                        for (   Iterator it_1 = q_1.GetIterator();
                                q_1.IsValid(it_1);
                                q_1.MoveNext(it_1))
                        {
                            if (! AddTail(q_1.GetAt(it_1)))
                            {
                                b_Result = false;
                            }
                        }
                        // ... then the reference element...
                        if (! AddTail(t_Ref))
                        {
                            b_Result = false;
                        }
                        // ... and then q_2.
                        for (   Iterator it_2 = q_2.GetIterator();
                                q_2.IsValid(it_2);
                                q_2.MoveNext(it_2))
                        {
                            if (! AddTail(q_2.GetAt(it_2)))
                            {
                                b_Result = false;
                            }
                        }
                    }
                }
                return b_Result;
            }

        private:
            //! Internal queue buffer.
            //! @return N/A (doxygen warning)
            std::deque<T> m_stlQueue;
        };

        //! @brief Basic map object.
        template <class K, class T> class Map : public cli::Object
        {
        private:
            //! @brief No default constructor.
            explicit Map(void);

        public:
            //! @brief Main constructor.
            explicit Map(
                    const unsigned int UI_MaxCount  //!< Maximum item count.
                    )
              : Object(), m_stlMap()
            {
            }

            //! @brief Copy constructor.
            Map(
                    const Map<K,T>& TK_Map      //!< Source map object.
                    )
              : Object(), m_stlMap(TK_Map.m_stlMap)
            {
            }

            //! @brief Destructor.
            virtual ~Map(void)
            {
            }

        private:
            //! @brief No assignment operator.
            Map& operator=(const Map&);

        public:
            //! @brief Determines whether the map is empty.
            //! @return true when the map is empty, false otherwise.
            const bool IsEmpty(void) const
            {
                return m_stlMap.empty();
            }

            //! @brief Item count.
            //! @return The number of items in the map.
            const unsigned int GetCount(void) const
            {
                return (unsigned int) m_stlMap.size(); // cast to avoid warnings
            }

        public:
            //! @brief Resets the map.
            //! @return true when success, false otherwise.
            //! @author [contrib: Oleg Smolsky, 2010, based on CLI 2.5]
            const bool Reset(void)
            {
                m_stlMap.clear();
                return true;
            }

        public:
            //! @brief Set a new item.
            //! @return true if the element has been set, false otherwise.
            const bool SetAt(
                    const K& K_Key,     //!< Key of the element set.
                    const T& T_Value    //!< Value of the element set.
                    )
            {
                if (m_stlMap.count(K_Key) > 0)
                {
                    m_stlMap.erase(K_Key);
                }
                m_stlMap.insert(std::make_pair(K_Key, T_Value));
                return true;
            }

            //! @brief Unset an item.
            //! @return true if the element has been unset correctly, or if the element was not set, false otherwise.
            const bool Unset(
                    const K& K_Key      //!< Key of the element unset.
                    )
            {
                m_stlMap.erase(K_Key);
                return true;
            }

            //! @brief Checks whether an element is set for this key.
            //! @return true if the key is set, false otherwise.
            const bool IsSet(
                    const K& K_Key      //!< Key to check.
                    ) const
            {
                return (m_stlMap.count(K_Key) > 0);
            }

            //! @brief Element accessor.
            //! @return NULL if no element is set for this key.
            const T* const GetAt(
                    const K& K_Key      //!< Key of the element accessed.
                    ) const
            {
                typename std::map<K,T>::const_iterator it = m_stlMap.find(K_Key);
                if (it != m_stlMap.end())
                {
                    return & it->second;
                }
                else
                {
                    return NULL;
                }
            }

        public:
            //! @brief Iterator object.
            class Iterator : public std::map<K,T>::iterator
            {
            private:
                //! @brief Default constructor.
                explicit Iterator(void) : std::map<K,T>::iterator() {}

            public:
                //! @brief Copy constructor.
                //! @note   We cannot use implicit conversions from STL objects to CLI objects here
                //!         because conflicting calls to Map::GetAt().
                Iterator(
                        const Iterator& it      //!< Element to be copied.
                        )
                  : std::map<K,T>::iterator(it) {}

            private:
                //! @brief Assignment operator.
                //! @note This operator ensures conversions from STL objects to CLI objects.
                template <class U>
                Iterator& operator=(const U& any) {
                    std::map<K,T>::iterator::operator=(any);
                    return *this;
                }

            private:
                // In order to allow access to privet members for iteration.
                friend class Map;
            };


            //! @brief Iterator retrieval.
            //! @return Iterator instance.
            Iterator GetIterator(void) const
            {
                Iterator it;
                it = const_cast<std::map<K,T>&>(m_stlMap).begin();
                return it;
            }

            //! @brief Checks the element at the given position is valid.
            //! @return true when the iterator is at a valid place, false otherwise.
            const bool IsValid(
                    const Iterator& it      //!< Iterator to check.
                    ) const
            {
                return (it != m_stlMap.end());
            }

            //! @brief Iterates the iterator.
            //! @return true if the iterator has moved to a valid place, false otherwise.
            const bool MoveNext(
                    Iterator& it            //!< Iterator to move forward.
                    ) const
            {
                it ++;
                return IsValid(it);
            }

            //! @brief Key retrieval.
            //! @return Key of the element pointed by the iterator.
            const K& GetKey(
                    const Iterator& it      //!< Current iterator.
                    ) const
            {
                return it->first;
            }

            //! @brief Read-only item retrieval.
            //! @return Read-only value of the element pointed by the iterator.
            const T& GetAt(
                    const Iterator& it      //!< Current iterator.
                    ) const
            {
                return it->second;
            }

            //! @brief Modifiable item retrieval.
            //! @return Modifiable value of the element pointed by the iterator.
            T& GetAt(
                    const Iterator& it      //!< Current iterator.
                    )
            {
                return it->second;
            }

            //! @brief Item removal.
            //! @param it Position. Set to next item.
            //! @return The remove element.
            const T Remove(
                    Iterator& it            //!< Current iterator.
                    )
            {
                const T t_Element = GetAt(it);
                /*it =*/ m_stlMap.erase(it);
                return t_Element;
            }

        private:
            //! Internal pair list.
            std::map<K,T> m_stlMap;
        };

    CLI_NS_END(tk)

CLI_NS_END(cli)

#endif // _CLI_TK_STL_H_

