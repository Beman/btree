//  example/hetero_set.cpp

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

///$id code=
#include <boost/btree/btree_set.hpp>
#include <iostream>
#include <boost/detail/lightweight_main.hpp> 

using std::cout;
using namespace boost::btree;

namespace
{
  struct UDT
  {
    int x;
    int y;

    UDT() {}
    UDT(int x_, int y_) : x(x_), y(y_) {}
    bool operator<(const UDT& rhs) const { return x < rhs.x; }
    bool operator<(int rhs) const { return x < rhs; }              // note rhs type
  };
  bool operator<(int lhs, const UDT& rhs) { return lhs < rhs.x; }  // note lhs type
}


int cpp_main(int, char *[])
{
  typedef btree_set<UDT> BT;
  BT bt("hetero_set.btr", flags::truncate);

  bt.insert(UDT(2, 222));
  bt.insert(UDT(1, 111));
  bt.insert(UDT(3, 333));

  for (BT::iterator itr = bt.begin(); itr != bt.end(); ++itr)
    cout << itr->x << ',' << itr->y << '\n';

  BT::iterator itr = bt.find(2);  // note find(2) argument type
  cout << "find(2) found " << itr->x << ',' << itr->y << '\n';

  return 0;
}
///$endid
