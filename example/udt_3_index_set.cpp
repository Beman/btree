//  example/udt_3_index_set.cpp

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
  typedef btree_index_set<UDT> IDX1;
  typedef btree_index_set<UDT, default_traits, id_ordering> IDX2;
  typedef btree_index_set<UDT, default_traits, spanish_ordering> IDX3;

  IDX1 idx1("udt_3_index_set.idx1", "udt_3_index_set.dat", flags::truncate);
  IDX2 idx2("udt_3_index_set.idx2", idx1.file(), flags::truncate, -1,
    id_ordering());
  IDX3 idx3("udt_3_index_set.idx3", idx1.file(), flags::truncate, -1,
    spanish_ordering());

  IDX1::file_position pos;

  pos = idx1.push_back(UDT(1, "eat", "comer"));
  idx1.insert_file_position(pos);
  idx2.insert_file_position(pos);
  idx3.insert_file_position(pos);

  pos = idx1.push_back(UDT(2, "drink", "beber"));
  idx1.insert_file_position(pos);
  idx2.insert_file_position(pos);
  idx3.insert_file_position(pos);

  pos = idx1.push_back(UDT(3, "be merry", "ser feliz"));
  idx1.insert_file_position(pos);
  idx2.insert_file_position(pos);
  idx3.insert_file_position(pos);

  cout << "\ninx1 - English ordering:\n\n";
  for (IDX1::iterator itr = idx1.begin(); itr != idx1.end(); ++itr)
    cout << "    " << *itr << '\n';

  cout << "\ninx2 - ID ordering:\n\n";
  for (IDX2::iterator itr = idx2.begin(); itr != idx2.end(); ++itr)
    cout << "    " << *itr << '\n';

  cout << "\ninx3 - Spanish ordering:\n\n";
  for (IDX3::iterator itr = idx3.begin(); itr != idx3.end(); ++itr)
    cout << "    " << *itr << '\n';

  return 0;
}
///$endid
