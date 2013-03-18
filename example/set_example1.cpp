//  set_example1.cpp  ------------------------------------------------------------------//

//  Copyright Beman Dawes 2011

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

#include <iostream>
using std::cout;

//#include <set>
//typedef std::set<int> set_type;
//set_type s;

#include <boost/btree/set.hpp>


int main()
{
  typedef boost::btree::btree_set<int> set_type;
  set_type s("set.btr", boost::btree::flags::truncate);

  s.insert(5); s.insert(3); s.insert(1);

  for (set_type::iterator it = s.begin(); it != s.end(); ++it)
    cout << *it << '\n';

  cout << "lower_bound(3) is " << *s.lower_bound(3) << '\n';
  cout << "lower_bound(4) is " << *s.lower_bound(4) << '\n';
  cout << "upper_bound(3) is " << *s.upper_bound(3) << '\n';
  cout << "upper_bound(4) is " << *s.upper_bound(4) << '\n';

  return 0;
}
