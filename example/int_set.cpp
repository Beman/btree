//  example/int_set.cpp 

//  Copyright Beman Dawes 2011

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

#include <boost/btree/btree_bt.hpp>
#include <iostream>

using std::cout;
using namespace boost::btree;

int main()
{
  btree_set<int> set("int_set.btr", flags::truncate);

  bt.insert(5);
  bt.insert(3);
  bt.insert(1);

  for (BT::iterator itr = bt.begin(); itr != bt.end(); ++itr)
    cout << *itr << '\n';

  cout << "lower_bound(3) is " << *bt.lower_bound(3) << '\n';
  cout << "upper_bound(3) is " << *bt.upper_bound(3) << '\n';
}
