//  boost/btree/detail/fixstr.hpp  -----------------------------------------------------//

//  Copyright Beman Dawes 2010

//  Distributed under the Boost Software License, Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                            fixed maximum-length string holder                        //
//                                                                                      //
//   The interface for this class needs to be revised in light of the basic_string_view //
//   Technical Specification (TS).                                                      //
//                                                                                      //
//                                                                                      //
//--------------------------------------------------------------------------------------//

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable : 4996)   // equivalent to -D_SCL_SECURE_NO_WARNINGS
#endif

#include <string>
#include <cstring>
#include <boost/assert.hpp>

#ifndef BOOST_BTREE_FIXSTR_HPP
#define BOOST_BTREE_FIXSTR_HPP

namespace boost
{
namespace btree
{

  template <unsigned MaxSize>
  class fixstr
  {
    char rep[MaxSize+1];  // invariant: '\0' terminated and filled
  public:
    fixstr()                                 { std::memset(rep, 0, MaxSize+1); }
    fixstr(const char* s)                    { std::strncpy(rep, s, MaxSize);
                                               rep[MaxSize] = '\0'; }
    fixstr(const std::string& s)             { std::strncpy(rep, s.c_str(), MaxSize);
                                               rep[MaxSize] = '\0'; }
                                             
    fixstr& operator=(const char* s)         { std::strncpy(rep, s, MaxSize);
                                               rep[MaxSize] = '\0';
                                               return *this; } 
    fixstr& operator=(const std::string& s)  { std::strncpy(rep, s.c_str(), MaxSize);
                                               rep[MaxSize] = '\0';
                                               return *this; } 
    fixstr& operator+=(const char* s)        { std::size_t sz = size();
                                               std::strncpy(rep+sz, s, MaxSize-sz);
                                               rep[MaxSize] = '\0';
                                               return *this; } 
    fixstr& operator+=(const std::string& s) { std::size_t sz = size();
                                               std::strncpy(rep+sz, s.c_str(), MaxSize-sz);
                                               rep[MaxSize] = '\0';
                                               return *this; } 
                                             
    std::size_t         size() const         { return std::strlen(rep); }
    bool                empty() const        { return rep[0] == '\0'; }
    static std::size_t  max_size()           { return MaxSize; }

    void                clear()              { for (char* itr = &rep[0]; *itr; ++itr)
                                                 *itr = '\0'; }

    char     operator[](std::size_t n) const { BOOST_ASSERT(n <= MaxSize); return rep[n]; }
    char&    operator[](std::size_t n)       { BOOST_ASSERT(n <= MaxSize); return rep[n]; }
                                             
    const char*         c_str() const        { return rep; }
    std::string         string() const       { return std::string(rep); }

    bool operator==(const fixstr& fxs) const { return std::strcmp(rep, fxs.rep) == 0; }
    bool operator!=(const fixstr& fxs) const { return std::strcmp(rep, fxs.rep) != 0; }
    bool operator< (const fixstr& fxs) const { return std::strcmp(rep, fxs.rep) <  0; }
    bool operator<=(const fixstr& fxs) const { return std::strcmp(rep, fxs.rep) <= 0; }
    bool operator> (const fixstr& fxs) const { return std::strcmp(rep, fxs.rep) >  0; }
    bool operator>=(const fixstr& fxs) const { return std::strcmp(rep, fxs.rep) >= 0; }
  };

template <unsigned MaxSize>
std::ostream& operator<<(std::ostream& os, const fixstr<MaxSize>& x)
  {
    os << x.c_str();
    return os;
  }
}
}

#ifdef _MSC_VER
#  pragma warning(pop) 
#endif

#endif  // BOOST_BTREE_FIXSTR_HPP
