//  example/int_set_read_first_try.cpp

//  Copyright Beman Dawes 2011

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

///$id code=
#include <boost/btree/btree_set.hpp>
#include <iostream>

using std::cout;
using namespace boost::btree;

int main()
{
  typedef btree_set<int> BT;
  BT  bt("int_set.btr");

  for (BT::iterator itr = bt.begin(); itr != bt.end(); ++itr)
    cout << *itr << '\n';

  cout << "lower_bound(3) is " << *bt.lower_bound(3) << '\n';
  cout << "upper_bound(3) is " << *bt.upper_bound(3) << '\n';
}
///$endid
