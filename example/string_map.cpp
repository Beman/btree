//  example/string_map.cpp

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

///$id code=
#include <boost/btree/btree_map.hpp>
#include <boost/btree/support/string_holder.hpp>
#include <iostream>
#include <boost/detail/lightweight_main.hpp> 

using std::cout;
using namespace boost::btree;

int cpp_main(int, char *[])
{
  typedef btree_map<string_holder<32>, string_holder<32> > BT;
  BT bt("string_map.btr", flags::truncate);

  bt.emplace("eat", "comer");
  bt.emplace("drink", "beber");
  bt.emplace("be merry", "ser feliz");
  bt.emplace("be exceptionally merry", "ser feliz excepcionalmente");

  for (BT::const_iterator itr = bt.begin(); itr != bt.end(); ++itr)
    cout << itr->first << ", " << itr->second << '\n';

  return 0;
}
///$endid
