//  example/set_read_file.cpp  ---------------------------------------------------------//

//  Copyright Beman Dawes 2011

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

#include <boost/btree/btree_set.hpp>
#include <iostream>
#include <boost/detail/lightweight_main.hpp> 

using std::cout;
using namespace boost::btree;

int cpp_main(int, char *[])
{
  btree_set<int> set("set1.btr");

  for (btree_set<int>::iterator it = set.begin();
    it != set.end(); ++it)
    cout << *it << '\n';

  cout << "lower_bound(3) is " << *set.lower_bound(3) << '\n';
  cout << "upper_bound(3) is " << *set.upper_bound(3) << '\n';

  return 0;
}
