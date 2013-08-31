//  example/endian_map.cpp 

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

///$id code=
#include <boost/btree/btree_map.hpp>
#include <boost/endian/types.hpp>
#include <iostream>
#include <boost/detail/lightweight_main.hpp> 

using std::cout;
using namespace boost::btree;
using namespace boost::endian;

int cpp_main(int, char *[])
{
  typedef btree_map<big_int56un_t, big_int24un_t> BT;
  BT bt("endian_map.btr", flags::truncate);

  bt.emplace(big_int56un_t(38759234875LL), big_int24un_t(1));
  bt.emplace(big_int56un_t(82352345), big_int24un_t(2));
  bt.emplace(big_int56un_t(1242423462), big_int24un_t(3));

  cout << "sizeof(BT::value_type) is " << sizeof(BT::value_type) << '\n';

  for (BT::const_iterator itr = bt.begin();   // note well: const_iterator
    itr != bt.end(); ++itr)
    cout << itr->first << ", " << itr->second << '\n';

  return 0;
}
///$endid
