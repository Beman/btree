//  boost/btree/index_helpers.hpp  -----------------------------------------------------//

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_BTREE_INDEX_HELPERS_HPP
#define BOOST_BTREE_INDEX_HELPERS_HPP

#include <boost/config/warning_disable.hpp>

#include <boost/btree/detail/config.hpp>
#include <boost/btree/helpers.hpp>
#include <boost/utility/string_ref.hpp>
#include <boost/assert.hpp>
#include <cstring>

namespace boost
{

//--------------------------------------------------------------------------------------//
//                                 string_view_support                                  //
//--------------------------------------------------------------------------------------//

  //  Boost has an early version of string_view, named string_ref. So typedef the name.

  typedef string_ref string_view;

namespace btree
{
//--------------------------------------------------------------------------------------//
//                                 flat file adapters                                   //
//--------------------------------------------------------------------------------------//

//--------------  primary template; handles all fixed length data types  ---------------//

  template <class T>
  class flat_adapter
  {
  public:
    typedef const T&  type;

    static std::size_t flat_size(type x)             {return sizeof(T);}

    static void copy_to_flat(type src, char* dest)   {BOOST_ASSERT(dest);
                                                      std::memcpy(dest, &src, sizeof(T));}

    static type make_from_flat(const char* src)
    {
      BOOST_ASSERT(src);
      T tmp;
      std::memcpy(&T, src, sizeof(T));
      return tmp;}
  };

  //----------  const char* specialization; null-terminated C-style strings  -----------//

  template <>
  class flat_adapter<const char*>
  {
  public:
    typedef const char*  type;

    static std::size_t flat_size(const char* s)              {BOOST_ASSERT(s);
                                                              return std::strlen(s) + 1;}

    static void copy_to_flat(const char* src, char* dest)    {BOOST_ASSERT(src);
                                                              BOOST_ASSERT(dest);
                                                              std::strcpy(dest, src);}

    static type make_from_flat(const char* src)              {BOOST_ASSERT(src);
                                                              return src;}
  };

  //-----------------  string_view specialization; C++ style strings  ------------------//

  template <>
  class flat_adapter<boost::string_view>
  {
  public:
    typedef const boost::string_view&  type;

    static std::size_t flat_size(type s)                     {return s.size() + 1;}

    static void copy_to_flat(type src, char* dest)           {BOOST_ASSERT(dest);
                                                              std::strcpy(dest, src);}

    static type make_from_flat(const char* src)              {BOOST_ASSERT(src);
                                                              return src;}
  };

}  // namespace btree
}  // namespace boost

#endif // BOOST_BTREE_INDEX_HELPERS_HPP
