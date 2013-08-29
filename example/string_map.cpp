//  example/string_map.cpp

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

#include <boost/btree/btree_map.hpp>
#include <boost/btree/support/string_box.hpp>
#include <iostream>
#include <boost/detail/lightweight_main.hpp> 

using std::cout;
using namespace boost::btree;

int cpp_main(int, char *[])
{
  typedef btree_map<string_box<16>, string_box<16> > BT;
  BT bt("string_map.btr", flags::truncate);


  for (BT::iterator itr = bt.begin(); itr != bt.end(); ++itr)
    cout << itr->first << ", " << itr->second << '\n';
}
