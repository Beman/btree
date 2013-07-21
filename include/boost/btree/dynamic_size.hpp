//  boost/btree/dynamic_size.hpp  ------------------------------------------------------//

//  Copyright Beman Dawes 2010, 2011

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  See http://www.boost.org/libs/btree for documentation.

#ifndef BOOST_BTREE_DYNAMIC_SIZE_HPP
#define BOOST_BTREE_DYNAMIC_SIZE_HPP

#include <boost/type_traits/integral_constant.hpp>

namespace boost
{ 
  namespace btree
  {
    //  The value type of T shall be trivially copyable type; see std 3.9 [basic.types]

    //  Must be overloaded for any type T whose dynamic size differs from sizeof(T)
    //  See http://www.gotw.ca/publications/mill17.htm,
    //  Why Not Specialize: The Dimov/Abrahams Example

    //  Q. Why free-functions rather than member-functions.
    //  A. Free-functions are easier to write than the wrapper classes that would be
    //     required to adapt third-party classes that don't happend to have the
    //     required member-functions.

    //  Q. Implementing these requires reinterpret_cast. Isn't that dangerous?
    //  A. No, because of the trivially copyable type requirement. See the examples
    //     in the C++ standard, 3.9 [basic.types], paragraphs 2 and 3. 

    template <class T>
    struct has_dynamic_size : public boost::false_type{};

    template <class T>
    struct knows_own_dynamic_size : public boost::false_type{};

    template <class T>
    inline const char* dynamic_data(const T& data);

    template <class T>
    inline std::size_t dynamic_size(const T&)
      {return sizeof(T);}

    template <class T>
    inline void dynamic_assign(T& x, const char* ptr, std::size_t sz);

    template <class T>
    inline void dynamic_assign(T& x, const char* ptr);  // knows own size
  }
}

#endif  // BOOST_BTREE_DYNAMIC_SIZE_HPP
