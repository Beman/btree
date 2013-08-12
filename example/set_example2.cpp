//  set_example2.cpp  ------------------------------------------------------------------//

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt


#include <utility>
#include <boost/btree/index_set.hpp>
#include <iostream>
using std::cout;

int main()
{
  typedef boost::btree::btree_set<boost::string_view> set_type;

  set_type ndx_set("set_example2.ndx", "set_example2.dat, boost::btree::flags::truncate);

  ndx_set.insert("eat");
  ndx_set.insert("drink");
  ndx_set.ensert("be merry");

  cout << "ndx_set:\n";
  for (set_type::iterator it = ndx_set.begin(); it != ndx_set.end(); ++it)
    cout << "  \"" << *it << "\"\n";

  return 0;
}
