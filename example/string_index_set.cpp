//  example/string_index_set.cpp

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

///$id code=
#include <boost/btree/btree_index_set.hpp>
#include <boost/btree/support/string_view.hpp>
#include <iostream>
#include <boost/detail/lightweight_main.hpp>

using std::cout;
using namespace boost::btree;

int cpp_main(int, char *[])
{
  typedef btree_index_set<boost::string_view> BT;
  BT bt("string_index_set", flags::truncate);   // creates .ndx and .dat files

  bt.insert("eat");
  bt.insert("drink");
  bt.insert("be merry");
  bt.insert("be exceptionally merry");

  for (BT::iterator itr = bt.begin(); itr != bt.end(); ++itr)
    cout << *itr << '\n';

  return 0;    // required
}
/// $endid
