//  example/string_index_map.cpp

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

///$id code=
#include <boost/btree/btree_index_map.hpp>
#include <boost/btree/support/string_view.hpp>
#include <iostream>
#include <boost/detail/lightweight_main.hpp> 

using std::cout;
using namespace boost::btree;

int cpp_main(int, char *[])
{
  typedef btree_index_map<boost::string_view, boost::string_view> BT;
  BT bt("string_index_map", flags::truncate);

  bt.emplace("eat", "comer");
  bt.emplace("drink", "beber");
  bt.emplace("be merry", "ser feliz");
  bt.emplace("be exceptionally merry", "ser feliz excepcionalmente");

  for (BT::const_iterator itr = bt.begin(); itr != bt.end(); ++itr)
    cout << itr->first << ", " << itr->second << '\n';

  return 0;
}
///$endid
