//  boost/btree/index.hpp  -------------------------------------------------------------//

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                       index for a memory-mapped flat-file                            //
//                                                                                      //
//--------------------------------------------------------------------------------------//

/*

  Proof-of-concept; not intended for actual use.

*/

#ifndef BOOST_BTREE_INDEX_HPP
#define BOOST_BTREE_INDEX_HPP

#include <boost/btree/mmff.hpp>
#include <boost/btree/helpers.hpp>
#include <boost/assert.hpp>

namespace boost
{
namespace btree
{
  template <class Key, class Traits = default_traits,
            class Comp = btree::less<Key> >
  class btree_index
  {
  public:
  };
  


}  // namespace btree
}  // namespace boost

#endif  BOOST_BTREE_INDEX_HPP
