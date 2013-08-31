//  example/string_set

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

///$id code=
#include <boost/btree/btree_set.hpp>
#include <boost/btree/support/string_box.hpp>     // fixed length string
#include <iostream>
#include <boost/detail/lightweight_main.hpp> 

using std::cout;
using namespace boost::btree;

int cpp_main(int, char *[])
{
  typedef btree_set<string_box<16> > BT;    // note maximum length
  BT bt("string_set.btr", flags::truncate);

  bt.insert("eat");
  bt.insert("drink");
  bt.insert("be merry");
  bt.insert("be exceptionally merry");      // will truncate

  for (BT::iterator it = bt.begin(); it != bt.end(); ++it)
    cout << *it << '\n';

  return 0;    // required
}
///$endid
