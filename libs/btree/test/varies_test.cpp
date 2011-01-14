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

#include <iostream>  // during development, headers need this for debugging
#include <boost/btree/map.hpp>
#include <boost/btree/set.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <boost/detail/lightweight_main.hpp>
#include <boost/cstdint.hpp>
#include <boost/type_traits.hpp>
#include <iomanip>
#include <utility>
#include <set>
#include <map>

using namespace boost;
namespace fs = boost::filesystem;
using std::cout;
using std::endl;
using std::make_pair;

namespace
{

//-------------------------------- types_test -----------------------------------//

void types_test()
{
  cout << "types_test..." << endl;

  typedef std::set<int>                 std_set;
  typedef std::map<int, long>           std_map;
  typedef btree::btree_set<int>         fixed_set;
  typedef btree::btree_map<int, long>   fixed_map;
  typedef btree::btree_set<char*>       varies_set;
  typedef btree::btree_map<char*, int*> varies_map;

  // should fail static_assert with:
  //   "Key and T must both be pointers or neither may be a pointer"
  // btree::btree_map<char*, int> bad_varies_map;

  BOOST_TEST((boost::is_same<std_set::iterator::reference, const int&>::value));    // set values always const
  BOOST_TEST((boost::is_same<std_set::const_iterator::reference, const int&>::value));

  BOOST_TEST((boost::is_same<std_map::iterator::reference, std::pair<const int, long>& >::value));
  BOOST_TEST((boost::is_same<std_map::const_iterator::reference, const std::pair<const int, long>& >::value));

  BOOST_TEST((boost::is_same<fixed_set::key_type, int>::value));
  BOOST_TEST((boost::is_same<fixed_set::value_type, int>::value));
  BOOST_TEST((boost::is_same<fixed_set::iterator::reference, const int&>::value));  // set values always const
  BOOST_TEST((boost::is_same<fixed_set::const_iterator::reference, const int&>::value));

  BOOST_TEST((boost::is_same<fixed_map::key_type, int>::value));
  BOOST_TEST((boost::is_same<fixed_map::mapped_type, long>::value));
  BOOST_TEST((boost::is_same<fixed_map::value_type, std::pair<const int, long> >::value));
  BOOST_TEST((boost::is_same<fixed_map::iterator::reference, std::pair<const int, long>& >::value));
  BOOST_TEST((boost::is_same<fixed_map::const_iterator::reference, const std::pair<const int, long>& >::value));

  BOOST_TEST((boost::is_same<varies_set::key_type, char*>::value));
  BOOST_TEST((boost::is_same<varies_set::value_type, char*>::value));
  BOOST_TEST((boost::is_same<varies_set::iterator::reference, const char*>::value));  // set values always const
  BOOST_TEST((boost::is_same<varies_set::const_iterator::reference, const char*>::value));
  
  BOOST_TEST((boost::is_same<varies_map::key_type, char*>::value));
  BOOST_TEST((boost::is_same<varies_map::mapped_type, int*>::value));
  BOOST_TEST((boost::is_same<varies_map::value_type, std::pair<const char*, int*> >::value));
  BOOST_TEST((boost::is_same<varies_map::iterator::reference, std::pair<const char*, int*> >::value));
  BOOST_TEST((boost::is_same<varies_map::const_iterator::reference, std::pair<const char*, const int*> >::value));

  cout << "  types_test complete" << endl;
}

//-------------------------------- btree_size_test -----------------------------------//

void btree_size_test()
{
  cout << "btree_size_test..." << endl;
  
  char c = 'a';
  boost::int16_t i16 = 12345;
  boost::int32_t i32 = 123456;
  char* cp = "harry the cat";

  BOOST_TEST_EQ(btree::size(c), 1U);
  BOOST_TEST_EQ(btree::size(i16), 2U);
  BOOST_TEST_EQ(btree::size(i32), 4U);
  BOOST_TEST_EQ(btree::size(cp), 14U);

  const char cc = 'a';
  const boost::int16_t ci16 = 12345;
  const boost::int32_t ci32 = 123456;
  const char* ccp = "harry the tough guy cat";

  BOOST_TEST_EQ(btree::size(cc), 1U);
  BOOST_TEST_EQ(btree::size(ci16), 2U);
  BOOST_TEST_EQ(btree::size(ci32), 4U);
  BOOST_TEST_EQ(btree::size(ccp), 24U);

  cout << "  btree_size_test complete" << endl;
}

//-------------------------------- btree_less_test -----------------------------------//

void btree_less_test()
{
  cout << "btree_less_test..." << endl;
  
  int i1 = 1;
  int i2 = 2;

  btree::less<int> int_fo;

  BOOST_TEST(int_fo(i1, i2));
  BOOST_TEST(!int_fo(i1, i1));
  BOOST_TEST(!int_fo(i2, i1));

  btree::less<char*> c_str_fo;
  
  char* c_str1 = "aaa";
  char* c_str2 = "abc";

  BOOST_TEST(c_str_fo(c_str1, c_str2));
  BOOST_TEST(!c_str_fo(c_str1, c_str1));
  BOOST_TEST(!c_str_fo(c_str2, c_str1));
  
  const char* c_str3 = "aa";
  const char* c_str4 = "aaa";

  BOOST_TEST(c_str_fo(c_str3, c_str4));
  BOOST_TEST(!c_str_fo(c_str3, c_str3));
  BOOST_TEST(!c_str_fo(c_str4, c_str3));

  cout << "  btree_less_test complete" << endl;
}

//-------------------------------- set_c_string_test -----------------------------------//

void set_c_string_test()
{
  cout << "set_c_string_test..." << endl;
  
  //fs::path p("btree_set_c_string.btree");
  //btree::btree_set<const char*> bt(p, btree::flags::truncate);

  //bt.insert("how");
  //bt.insert("now");
  //bt.insert("brown");
  //bt.insert("cow");
  //BOOST_TEST_EQ(bt.size(), 4U);
  //btree::btree_set<const char*>::const_iterator it = bt.begin();
  //BOOST_TEST(*it == std::string("brown")); 
  //BOOST_TEST(*++it == std::string("cow")); 
  //BOOST_TEST(*++it == std::string("how")); 
  //BOOST_TEST(*++it == std::string("now")); 
  //BOOST_TEST(++it == btree::btree_set<const char*>::const_iterator()); 

  cout << "  set_c_string_test complete" << endl;
}

}  // unnamed namespace

//------------------------------------ cpp_main ----------------------------------------//

int cpp_main(int, char*[])
{
  //btree::btree_set<const char*> bta("a", btree::flags::truncate);
  //BOOST_TEST(bta.f<btree::btree_set<const char*>::key_type>());
  //btree::btree_set<char> btb("b", btree::flags::truncate);
  //BOOST_TEST(!btb.f<btree::btree_set<char>::key_type>());


  // Should fail to link: unresolved external symbol: size
  //btree::btree_set<const int*> bt2("btree_set_int_star.btree", btree::flags::truncate);
  //int i;
  //bt2.insert(&i);

  types_test();
  btree_size_test();
  btree_less_test();
  set_c_string_test();

  cout << "all tests complete" << endl;

  return boost::report_errors();
}


