//  endian_flip.hpp  -------------------------------------------------------------------//

//  Copyright Beman Dawes 2010

//  Distributed under the Boost Software License, Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_ENDIAN_FLIP_HPP
#define BOOST_ENDIAN_FLIP_HPP

#include <boost/integer.hpp>
namespace boost
{
namespace integer
{
  inline void endian_flip(int16_t& x)
  {
    char* rep = reinterpret_cast<char*>(&x);
    char tmp;
    tmp = *rep;
    *rep = *(rep+1);
    *(rep+1) = tmp;
  }

  inline void endian_flip(int32_t& x)
  {
    char* rep = reinterpret_cast<char*>(&x);
    char tmp;
    tmp = *rep;
    *rep = *(rep+3);
    *(rep+3) = tmp;
    tmp = *(rep+1);
    *(rep+1) = *(rep+2);
    *(rep+2) = tmp;
  }

  inline void endian_flip(int64_t& x)
  {
    char* rep = reinterpret_cast<char*>(&x);
    char tmp;
    tmp = *rep;
    *rep = *(rep+7);
    *(rep+7) = tmp;
    tmp = *(rep+1);
    *(rep+1) = *(rep+6);
    *(rep+6) = tmp;
    tmp = *(rep+2);
    *(rep+2) = *(rep+5);
    *(rep+5) = tmp;
    tmp = *(rep+3);
    *(rep+3) = *(rep+4);
    *(rep+4) = tmp;
  }

  inline void endian_flip(uint16_t& x)
  {
    char* rep = reinterpret_cast<char*>(&x);
    char tmp;
    tmp = *rep;
    *rep = *(rep+1);
    *(rep+1) = tmp;
  }

  inline void endian_flip(uint32_t& x)
  {
    char* rep = reinterpret_cast<char*>(&x);
    char tmp;
    tmp = *rep;
    *rep = *(rep+3);
    *(rep+3) = tmp;
    tmp = *(rep+1);
    *(rep+1) = *(rep+2);
    *(rep+2) = tmp;
  }

  inline void endian_flip(uint64_t& x)
  {
    char* rep = reinterpret_cast<char*>(&x);
    char tmp;
    tmp = *rep;
    *rep = *(rep+7);
    *(rep+7) = tmp;
    tmp = *(rep+1);
    *(rep+1) = *(rep+6);
    *(rep+6) = tmp;
    tmp = *(rep+2);
    *(rep+2) = *(rep+5);
    *(rep+5) = tmp;
    tmp = *(rep+3);
    *(rep+3) = *(rep+4);
    *(rep+4) = tmp;
  }
}  // namespace integer
}  // namespace boost

#endif // BOOST_ENDIAN_FLIP_HPP
