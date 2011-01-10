//  varies_test.cpp  -------------------------------------------------------------------//

//  Copyright Beman Dawes 2011

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  See http://www.boost.org/libs/btree for documentation.

//--------------------------------------------------------------------------------------//
//                                                                                      //
//  These tests lightly exercise variable length keys and/or data.                      //
//                                                                                      //
//--------------------------------------------------------------------------------------//

#include <boost/btree/map.hpp>
#include <boost/btree/set.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <boost/detail/lightweight_main.hpp>
#include <iostream>
#include <iomanip>
#include <utility>

using namespace boost;
namespace fs = boost::filesystem;
using std::cout;
using std::endl;
using std::make_pair;

namespace
{
  //struct fat
  //{
  //  int x;
  //  char unused[28];

  //  fat(int x_) : x(x_) {}
  //  fat() : x(-1) {}
  //  fat(const fat& f) : x(f.x) {}
  //  fat& operator=(const fat& f) {x = f.x; return *this; }

  //  bool operator<(const fat& rhs) const {return x < rhs.x;}
  //  bool operator==(const fat& rhs) const {return x == rhs.x;}
  //  bool operator!=(const fat& rhs) const {return x != rhs.x;}
  //};

//-------------------------------- set_c_string_test -----------------------------------//


void set_c_string_test()
{
  cout << "  set_c_string_test..." << endl;
  
  fs::path p("btree_set_c_string.btree");
  btree::btree_set<const char*> bt(p, btree::flags::truncate);

  bt.insert("how");
  bt.insert("now");
  bt.insert("brown");
  bt.insert("cow");
  BOOST_TEST_EQ(bt.size(), 4U);
  btree::btree_set<const char*>::const_iterator it = bt.begin();
  BOOST_TEST(*it == std::string("brown")); 
  BOOST_TEST(*++it == std::string("cow")); 
  BOOST_TEST(*++it == std::string("how")); 
  BOOST_TEST(*++it == std::string("now")); 
  BOOST_TEST(++it == btree::btree_set<const char*>::const_iterator()); 

  cout << "    set_c_string_test complete" << endl;
}

}  // unnamed namespace

//------------------------------------ cpp_main ----------------------------------------//

int cpp_main(int, char*[])
{
  set_c_string_test();

  cout << "all tests complete" << endl;

  return boost::report_errors();
}


