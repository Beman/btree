//  example/string_index_set.cpp

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  $id code=
#include <boost/btree/btree_index_set.hpp>
#include <boost/btree/support/string_view.hpp>
#include <iostream>
#include <boost/detail/lightweight_main.hpp>

using std::cout;
using namespace boost::btree;

int cpp_main(int, char *[])
{
  typedef btree_index_set<string_view> BT;
  BT bt("string_index_set", flags::truncate);

  set.insert("eat");
  set.insert("drink");
  set.insert("be merry");

  for (BT::iterator it = set.begin(); it != set.end(); ++it)
    cout << *it << '\n';
}
//  $endid

