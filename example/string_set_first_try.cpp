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
  typedef btree_set<std::string> set_type;
  set_type set("string_set.btr", flags::truncate);

  set.insert("eat");
  set.insert("drink");
  set.insert("be merry");

  for (set_type::iterator it = set.begin(); it != set.end(); ++it)
    cout << *it << '\n';

  return 0;    // required
}
///$endid
