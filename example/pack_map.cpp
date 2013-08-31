//  example/pack_map.cpp  

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

///$id code=
#include <boost/btree/btree_map.hpp>
#include <iostream>
#include <boost/detail/lightweight_main.hpp>
#include <boost/assert.hpp>

using std::cout;
using namespace boost::btree;

int cpp_main(int, char *[])
{
  typedef btree_map<int, int> BT;

  BT old_bt("int_map.btr", flags::read_only);
  BT new_bt(old_bt.begin(), old_bt.end(), "packed_int_map.btr", flags::truncate);

  BOOST_ASSERT(old_bt == new_bt);

  return 0;
}
///$endid
