
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

#include <SF/bitset.hpp>

#include <SF/Archive.hpp>
#include <SF/vector.hpp>
#include <RCF/MemStream.hpp>
#include <RCF/Tools.hpp>

namespace SF {

    void serializeBitset(SF::Archive & ar, I_BitsetWrapper & bits)
    {
        if (ar.isWrite())
        {
            std::uint32_t bitCount = static_cast<std::uint32_t>(bits.size());
            std::uint32_t charCount = (bitCount+7)/8;
            std::vector<char> buffer;
            buffer.reserve(charCount);
            for (std::uint32_t i=0; i<charCount; ++i)
            {
                char ch = 0;

                std::uint32_t bitsForThisChar = RCF_MIN( 
                    std::uint32_t(8), 
                    std::uint32_t(bitCount - 8*i));

                for (std::uint32_t j=0; j<bitsForThisChar; ++j)
                {
                    bool bit = bits.getBit(8*i + j);
                    ch = ch | static_cast<char>( bit << j );
                }
                buffer.push_back(ch);
            }

            ar & bitCount & buffer;
        }
        else if (ar.isRead())
        {
            std::uint32_t bitCount = 0;
            std::vector<char> buffer;
            ar & bitCount & buffer;

            // Sanity check on bit count and buffer size.
            bool ok =       
                (bitCount == 0 && buffer.size() == 0)
                ||  (8*buffer.size() - 7 <= bitCount &&  bitCount <= 8*buffer.size());

            if (!ok)
            {
                RCF_THROW( RCF::Exception( RCF::RcfError_DeserializeVectorBool, bitCount, buffer.size()));
            }

            bits.resize(bitCount);
            for (std::size_t i=0; i<buffer.size(); ++i)
            {
                char ch = buffer[i];

                std::uint32_t bitsForThisChar = RCF_MIN(
                    std::uint32_t(8), 
                    std::uint32_t(bitCount - 8*i));

                for (std::uint32_t j=0; j<bitsForThisChar; ++j)
                {
                    bool bit = ch & (1<<j) ? true : false;
                    bits.setBit(8*i + j, bit);
                }
            }
        }

    }

} // namespace SF
