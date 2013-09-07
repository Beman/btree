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
  typedef btree_map<big_int56_t, big_int24_t> BT;  // note the sizes!
  BT bt("endian_map.btr", flags::truncate);

  bt.emplace(38759234875LL, 1);
  bt.emplace(82352345, 2);
  bt.emplace(1242423462, 3);

  cout << "sizeof(BT::value_type) is " << sizeof(BT::value_type) << '\n';

  for (BT::const_iterator itr = bt.begin();
    itr != bt.end(); ++itr)
    cout << itr->first << ", " << itr->second << '\n';

  return 0;
}
///$endid
