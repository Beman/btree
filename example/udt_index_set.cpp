//  example/udt_index_set.cpp

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

///$id code=
#include <boost/btree/btree_index_set.hpp>
#include "udt.hpp"
#include <boost/btree/support/string_view.hpp>
#include <iostream>
#include <boost/detail/lightweight_main.hpp> 

using std::cout;
using namespace boost::btree;

int cpp_main(int, char *[])
{
  typedef btree_index_set<UDT> BT;
  BT bt("UDT_index_set", flags::truncate);

  bt.insert(UDT(1, "eat", "comer"));
  bt.insert(UDT(2, "drink", "beber"));
  bt.insert(UDT(3, "be merry", "ser feliz"));

  for (BT::iterator itr = bt.begin(); itr != bt.end(); ++itr)
    cout << *itr << '\n';

  return 0;
}
///$endid
