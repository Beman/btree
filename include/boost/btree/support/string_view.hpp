//  boost/btree/support/string_view.hpp  -----------------------------------------------//

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_BTREE_STRING_VIEW_HPP
#define BOOST_BTREE_STRING_VIEW_HPP

#include <boost/type_traits/integral_constant.hpp>
#include <boost/btree/dynamic_size.hpp>
#include <string>
#include <cstring>  // for strlen
#include <cstdint>

namespace boost
{
namespace btree
{
namespace support
{

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                         placeholder for N3685 string_view                            //
//                                                                                      //
//--------------------------------------------------------------------------------------//
/*

  B-trees persist via external I/O. Ditto the flat files a B-tree may index reference.
  This means that the data they contain must be trivially copyable, must not contain
  pointers or references, B-tree also cannot efficiently contain variable length data or
  data that is large in relation to efficient disk node sizes.
  
  std::string does not meet these requirements. Instead, we can use a proxy that does
  meet the requirements. The C++ standard library is about to add a Technical
  Specification (TS) for such a proxy:
  
  N3685, string_view: a non-owning reference to a string, revision 4,
  http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3685.html,
  
  Until implementations become available, this header supplies a simple string_view
  that supplies enough features for proof-of-concept testing.

*/

  class string_view
  {
    const char*  m_ptr;
    size_type    m_size;
  public:
    typedef char value_type;
    typedef const char* pointer;
    typedef pointer iterator;
    typedef pointer const_iterator;
    typedef std::size_t size_type;

    string_view() : m_ptr(0), m_size(0) {}
    string_view(const std::string& str) : m_ptr(str.data()), m_size(str.size()) {}
    string_view(const char* ptr) : m_ptr(ptr), m_size(std::strlen(ptr) {}
    string_view(const char* ptr, size_type sz) : m_ptr(ptr), m_size(sz) {}

    string_view& assign(const char* ptr, std::size_t sz)  {m_ptr = ptr; m_size = sz;}

    const_iterator begin() const {return m_ptr;}
    const_iterator end() const {return m_ptr + m_size;}

    size_type size() const {return m_size;}
    size_type length() const {return m_size;}
    bool empty() const{return m_size == 0;}

    const char& operator[](size_type pos) const {BOOST_ASSERT(!empty());
                                                 return *(m_ptr + pos);}
    const char* data() const {return m_ptr;}
  };

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                                dynamic size support                                  //
//                                                                                      //
//--------------------------------------------------------------------------------------//

  template <>
  struct has_dynamic_size<string_view>
    : public boost::true_type{};

  inline
  const char* dynamic_data(const string_view& sv)
    {return sv.const_data();}


  inline
  std::size_t dynamic_size(const string_view& sv)
    {return sv.size();}

  inline
  void dynamic_assign(string_view& sv, const char* ptr, std::size_t sz)
    {sv.assign(ptr, sz);}

}  // namespace support
}  // namespace btree
}  // namespace boost

#endif  BOOST_BTREE_STRING_VIEW_HPP
