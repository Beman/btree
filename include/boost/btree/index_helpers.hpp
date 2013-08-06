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
//                                string_view_support                                   //
//--------------------------------------------------------------------------------------//

  //  Boost has an early version of string_view, named string_ref. So typedef the name.

  typedef string_ref string_view;

namespace btree
{

//--------------------------------------------------------------------------------------//
//                                   index traits                                       //
//--------------------------------------------------------------------------------------//

//--------------  primary template; handles all fixed length data types  ---------------//

  template <class T>
  class default_index_traits
  {
  public:
    typedef const T&  reference;

    typedef endian::big_uint48un_t  index_key;  // position in the flat file

    static std::size_t flat_size(const T&)    {return sizeof(T);}

    static void build_flat_element(const T& x, char* dest, std::size_t sz)
    { 
      BOOST_ASSERT(dest);
      BOOST_ASSERT_MSG(sz == sizeof(T),
        "btree index: size error; did you mean to uses a varaible-size type?");
      std::memcpy(dest, reinterpret_cast<const char*>(&x), sz);
    }

    static reference  make_reference(const char* x)
    {
      BOOST_ASSERT(x);
      return *reinterpret_cast<const T*>(x);
    }
    
  };

  //----------  const char* specialization; null-terminated C-style strings  -----------//

  template <>
  class default_index_traits<const char*>
  {
  public:
    typedef const char*  reference;

    typedef endian::big_uint48un_t  index_key;  // position in the flat file

    static std::size_t flat_size(const char* x)          {BOOST_ASSERT(x);
                                                          return std::strlen(x) + 1;}

    static void build_flat_element(const char* x, char* dest, std::size_t)
    {
      BOOST_ASSERT(x);
      BOOST_ASSERT(dest);
      std::strcpy(dest, x);
    }

    static reference make_reference(const char* x)       {BOOST_ASSERT(x);
                                                          return x;}
  };

  //-----------------  string_view specialization; C++ style strings  ------------------//

  template <>
  class default_index_traits<boost::string_view>
  {
  public:
    typedef boost::string_view         reference;

    typedef endian::big_uint48un_t     index_key;  // position in the flat file

    static std::size_t flat_size(const boost::string_view& x)  {return x.size() + 1;}

    static void build_flat_element(const boost::string_view& x, char* dest,
      std::size_t sz)
    {
      BOOST_ASSERT(dest);
      BOOST_ASSERT(sz == x.size() + 1);
      std::memcpy(dest, x.data(), x.size());
      *(dest+x.size()) = '\0';
    }

    static reference make_reference(const char* x)
      {return boost::string_view(x);}
  };

}  // namespace btree
}  // namespace boost

#endif // BOOST_BTREE_INDEX_HELPERS_HPP
