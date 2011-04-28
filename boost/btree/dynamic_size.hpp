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

    //  Must be overloaded for any type T whose dynamic size differs from sizeof(T)
    //  See http://www.gotw.ca/publications/mill17.htm,
    //  Why Not Specialize: The Dimov/Abrahams Example

    template <class T>
    inline std::size_t dynamic_size(const T&) { return sizeof(T); }

    template <class T>
    struct has_dynamic_size : public false_type{};
  }
}

#endif BOOST_BTREE_DYNAMIC_SIZE_HPP
