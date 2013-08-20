//  example/set2_1st_try.cpp  ----------------------------------------------------------//

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

#include <boost/btree/btree_set.hpp>
#include <string>
#include <iostream>
using std::cout;
using namespace boost::btree;

int main()
{
  btree_set<std::string> set("set2.btr", flags::truncate);

  set.insert("eat");
  set.insert("drink");
  set.insert("be merry");

  for (btree_set<int>::iterator it = set.begin();
    it != set.end(); ++it)
    cout << *it << '\n';

  return 0;
}
