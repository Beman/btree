//  example/int_map.cpp 

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

///$id code=
#include <boost/btree/btree_map.hpp>
#include <iostream>
#include <boost/detail/lightweight_main.hpp> 

using std::cout;
using namespace boost::btree;

int cpp_main(int, char *[])
{
  typedef btree_map<int, int> BT;
  BT bt("int_map.btr", flags::truncate);

  bt.emplace(2, 222);
  bt.emplace(1, 111);
  bt.emplace(3, 333);

  for (BT::const_iterator itr = bt.begin();   // note well: const_iterator
       itr != bt.end(); ++itr)
    cout << itr->first << ", " << itr->second << '\n';

  return 0;
}
///$endid
