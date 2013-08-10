//  boost/btree/support/size_t_codec.hpp  ----------------------------------------------//

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_BTREE_SIZE_T_CODEC_HPP
#define BOOST_BTREE_SIZE_T_CODEC_HPP

#include <cstddef>
#include <climits>
#include <utility>  // for std::pair<>

namespace boost
{
namespace btree
{
namespace support
{

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                            coder/decoder for std::size_t                             //
//                                                                                      //
//  Encodes and decodes a size_t value as a variable length byte string. The            //
//  high-order bit of the last (i.e. low-order) byte is not set. The high-order bit of  //
//  all other bytes is set. Thus there are 7 significant bits per byte, so it takes 1   //
//  to (sizeof(size_t) * 8) / 7 + 1 bytes to hold a size_t value. That works out to 5   //
//  bytes for 32-bit size_t, and 10 bytes for 64-bit size_t.                            //
//                                                                                      //
//  A use case would be to encode the length of strings. Since typical strings are      //
//  short, the encode length is often only one or two bytes, and that is a significant  //
//  savings for applications like B-trees that are very sensitive to data sizes.        //
//                                                                                      //
//  The technique obviously generalizes to types other than size_t, but size_t is the   //
//  only current concern.                                                               //
//                                                                                      //
//  See size_t_codec_test() in index_unit_test.cpp for tests.                           //
//                                                                                      //
//--------------------------------------------------------------------------------------//

  BOOST_STATIC_ASSERT_MSG(sizeof(std::size_t) == 4 || sizeof(std::size_t) == 8,
    "Only 32 and 64-bit size_t currently supported");

  BOOST_STATIC_ASSERT_MSG(CHAR_BIT == 8, "Only CHAR_BIT == 8 currently supported");
  
  struct size_t_codec
  {
    static const std::size_t max_size = (sizeof(std::size_t)*8)/7+1;

    static std::size_t encoded_size(std::size_t x);
    //  Returns: The encoded size of x.

    static void  encode(std::size_t x, char* dest, std::size_t encoded_sz);
    //  Requires: encoded_sz == encoded_size(x)
    //  Effects: The encoded value of x is placed in the array beginning at dest.

    static std::pair<std::size_t, std::size_t> decode(const char* x);
    //  Requires: x is encoded using the encode function.
    //  Returns: A pair containing the value of the encoded byte string beginning at s
    //  and the size of that byte string.
  };

  inline std::size_t size_t_codec::encoded_size(std::size_t x)
  {
    // hopefully this code is very fast for the common use cases of smallish values
    if (x <= 0x7fu) return 1;
    if (x <= 0x3fffu) return 2;
    if (x <= 0x1fffffu) return 3;

    // this code will work for either 32 or 64-bit size_t
    x >>= 21;
    std::size_t r = 3;
    for (; x; ++r, x >>= 7) {}
    return r;
  }

  inline void size_t_codec::encode(std::size_t x, char* dest, std::size_t encoded_sz)
  {
    BOOST_ASSERT(encoded_sz == encoded_size(x));
    char* p = dest + encoded_sz - 1;
    *p = static_cast<unsigned char>(x & 0x7f);
    x >>= 7;
    while (x)
    {
      *--p = static_cast<unsigned char>((x & 0x7f) | 0x80);
      x >>= 7;
    };
  }

  inline std::pair<std::size_t, std::size_t>
    size_t_codec::decode(const char* x)
  {
    std::pair<std::size_t, std::size_t> tmp;
    tmp.first = 0;
    tmp.second = 0;
    while (*x & 0x80)
    {
      BOOST_ASSERT(tmp.second < max_size);
      tmp.first |= static_cast<std::size_t>(*x & 0x7f);
      tmp.first <<= 7;
      ++x;
      ++tmp.second;
    }
    tmp.first |= *x & 0x7f;
    ++tmp.second;
    return tmp;
  }

}  // namespace support
}  // namespace btree
}  // namespace boost

#endif  //BOOST_BTREE_SIZE_T_CODEC_HPP
