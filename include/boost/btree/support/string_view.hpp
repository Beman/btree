//  boost/btree/index_helpers.hpp  -----------------------------------------------------//

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_BTREE_STRING_VIEW_HPP
#define BOOST_BTREE_STRING_VIEW_HPP
#include <boost/utility/string_ref.hpp>

namespace boost
{

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                                   string_view                                        //
//                                                                                      //
//  Thanks to Marshall Clow, Boost has an early version of string_view, dating from     //
//  when it was named string_ref. So the only thing needed is a typedef.                //
//                                                                                      //
//  See http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3685.html              //
//                                                                                      //
//--------------------------------------------------------------------------------------//

  //  . So typedef the name.

  typedef string_ref string_view;

}  // namespace boost

#endif // BOOST_BTREE_STRING_VIEW_HPP
