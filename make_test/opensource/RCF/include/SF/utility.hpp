
//******************************************************************************
// RCF - Remote Call Framework
//
// Copyright (c) 2005 - 2018, Delta V Software. All rights reserved.
// http://www.deltavsoft.com
//
// RCF is distributed under dual licenses - closed source or GPL.
// Consult your particular license for conditions of use.
//
// If you have not purchased a commercial license, you are using RCF 
// under GPL terms.
//
// Version: 3.0
// Contact: support <at> deltavsoft.com 
//
//******************************************************************************

#ifndef INCLUDE_SF_UTILITY_HPP
#define INCLUDE_SF_UTILITY_HPP

#include <utility>

#include <SF/Archive.hpp>

namespace SF {

    // std::pair
    template<typename T, typename U>
    inline void serialize_vc6(Archive &ar, std::pair<T,U> &t, const unsigned int)
    {
        ar & t.first & t.second;
    }

} // namespace SF

#endif // ! INCLUDE_SF_UTILITY_HPP
