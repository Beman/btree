//  example/string_set_first_try.cpp  --------------------------------------------------//

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

///$id code=
#include <boost/btree/btree_set.hpp>
#include <string>
#include <iostream>
#include <boost/detail/lightweight_main.hpp> 

using std::cout;
using namespace boost::btree;

int cpp_main(int, char *[])
{
  typedef btree_set<std::string> BT;
  BT bt("string_set.btr", flags::truncate);

  bt.insert("eat");
  bt.insert("drink");
  bt.insert("be merry");

  for (BT::iterator itr = bt.begin(); itr != bt.end(); ++itr)
    cout << *itr << '\n';

  return 0;    // required
}
///$endid
