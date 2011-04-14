//  vbtree_unit_test.cpp  ------------------------------------------------------//

//  Copyright Beman Dawes 2006, 2010

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  See http://www.boost.org/libs/btree for documentation.

//--------------------------------------------------------------------------------------//
//                                                                                      //
//  These tests lightly exercise many portions of the interface. They do not attempt    //
//  to stress the many combinations of control paths possible in large scale use.       //
//  See stl_equivalence_test.cpp for large scale stress testing.                        //
//                                                                                      //
//--------------------------------------------------------------------------------------//

#include <boost/btree/vmap.hpp>
#include <boost/btree/vset.hpp>
#include <boost/btree/support/fixstr.hpp>
#include <boost/btree/support/strbuf.hpp>
#include <boost/detail/lightweight_main.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <boost/cstdint.hpp>
#include <boost/random.hpp>

#include <iostream>
#include <iomanip>
#include <utility>
#include <map>
#include <set>
#include <algorithm>

using namespace boost;
namespace fs = boost::filesystem;
using std::cout;
using std::endl;
using std::make_pair;

namespace
{
  struct fat
  {
    int x;
    char unused[28];

    fat(int x_) : x(x_) {}
    fat() : x(-1) {}
    fat(const fat& f) : x(f.x) {}
    fat& operator=(const fat& f) {x = f.x; return *this; }
    fat& operator=(int v) {x = v; return *this; }

    bool operator<(const fat& rhs) const {return x < rhs.x;}
    bool operator==(const fat& rhs) const {return x == rhs.x;}
    bool operator!=(const fat& rhs) const {return x != rhs.x;}
  };

  std::ostream& operator<<(std::ostream& os, const fat& f)
  {
    os << f.x;
    return os;
  }

}  // unnamed namespace

namespace {

//-------------------------------------- instantiate -----------------------------------//

template <class BT>
void instantiate_test(BT& bt)
{ 
  BOOST_TEST(!bt.is_open());
  BOOST_TEST(bt.size() == 0U);
  BOOST_TEST(bt.empty());
}

void instantiate()
{
  cout << "  instantiate..." << endl;
  {
    btree::vbtree_map<fat, int> x;
    instantiate_test(x);
  }
  {
    btree::vbtree_multimap<fat, int> x;
    instantiate_test(x);
  }
  {
    btree::vbtree_set<int> x;
    instantiate_test(x);
  }
  {
    btree::vbtree_multiset<int> x;
    instantiate_test(x);
  }
  {
    btree::vbtree_map<fat, int, btree::default_big_endian_traits> x;
    instantiate_test(x);
  }
  {
    btree::vbtree_multimap<fat, int, btree::default_little_endian_traits> x;
    instantiate_test(x);
  }
  {
    btree::vbtree_set<int, btree::default_little_endian_traits> x;
    instantiate_test(x);
  }
  {
    btree::vbtree_multiset<int, btree::default_big_endian_traits> x;
    instantiate_test(x);
  }
  cout << "    instantiate complete" << endl;
}

//-------------------------------- types_test ------------------------------------------//

void types_test()
{
  cout << "  types_test..." << endl;

  //  test std::set and std::map to add insight into types

  typedef std::set<int>                  std_set;
  typedef btree::vbtree_set<int>         bt_set;

  BOOST_TEST((boost::is_same<std_set::key_type, int>::value));
  BOOST_TEST((boost::is_same< bt_set::key_type, int>::value));

  BOOST_TEST((boost::is_same<std_set::value_type, int>::value));
  BOOST_TEST((boost::is_same< bt_set::value_type, int>::value));

  // reference is "lvalue of T" where T is value_type
  BOOST_TEST((boost::is_same<std_set::reference, int&>::value));
  BOOST_TEST((boost::is_same< bt_set::reference, int&>::value));

  // const_reference is "const lvalue of T" where T is value_type
  BOOST_TEST((boost::is_same<std_set::const_reference, int const&>::value));
  BOOST_TEST((boost::is_same< bt_set::const_reference, int const&>::value));

  BOOST_TEST((boost::is_same<std_set::iterator::reference, int const&>::value));
  BOOST_TEST((boost::is_same< bt_set::iterator::reference, int const&>::value));
  
  BOOST_TEST((boost::is_same<std_set::const_iterator::reference, int const&>::value));
  BOOST_TEST((boost::is_same< bt_set::const_iterator::reference, int const&>::value));

  typedef std::map<int, long>            std_map;
  typedef btree::vbtree_map<int, long>   bt_map;

  BOOST_TEST((boost::is_same<std_map::key_type, int>::value));
  BOOST_TEST((boost::is_same< bt_map::key_type, int>::value));
  
  BOOST_TEST((boost::is_same<std_map::mapped_type, long>::value));
  BOOST_TEST((boost::is_same< bt_map::mapped_type, long>::value));

  BOOST_TEST((boost::is_same<std_map::value_type, std::pair<const int, long> >::value));
  BOOST_TEST((boost::is_same< bt_map::value_type, btree::vbtree_value<const int, const long> >::value));

  // reference is "lvalue of T" where T is value_type
  BOOST_TEST((boost::is_same<std_map::reference,
    std::pair<const int, long>& >::value));
  BOOST_TEST((boost::is_same< bt_map::reference,
    btree::vbtree_value<const int, const long>& >::value));

  // const_reference is "const lvalue of T" where T is value_type
  BOOST_TEST((boost::is_same<std_map::const_reference,
    const std::pair<const int, long>& >::value));
  BOOST_TEST((boost::is_same< bt_map::const_reference,
    const btree::vbtree_value<const int, const long>& >::value));

  BOOST_TEST((boost::is_same<std_map::iterator::reference,
    std::pair<const int, long>& >::value));
  BOOST_TEST((boost::is_same< bt_map::iterator::reference,
    const btree::vbtree_value<const int, const long>& >::value));

  BOOST_TEST((boost::is_same<std_map::const_iterator::reference,
    const std::pair<const int, long>& >::value));
  BOOST_TEST((boost::is_same< bt_map::const_iterator::reference,
    const btree::vbtree_value<const int, const long>& >::value));

  cout << "    types_test complete" << endl;
}

//-------------------------------------- construct_new ---------------------------------//

template <class BT>
void construct_new_test(BT& bt, const fs::path& p)
{ 
  BOOST_TEST(bt.is_open());
  BOOST_TEST_EQ(bt.size(), 0U);
  BOOST_TEST(bt.empty());
  BOOST_TEST(!bt.read_only());
  BOOST_TEST(bt.page_size() == btree::default_page_size);  // the default
  BOOST_TEST(bt.file_path() == p);

  BOOST_TEST(bt.begin() == bt.end());

  BT::key_type k;
  BOOST_TEST(bt.lower_bound(k) == bt.end());
  BOOST_TEST(bt.upper_bound(k) == bt.end());
  BOOST_TEST(bt.find(k) == bt.end());

  bt.close();
  BOOST_TEST(!bt.is_open());
}

void construct_new()
{
  cout << "  construct_new..." << endl;
  {
    fs::path p("btree_map.btree");
    btree::vbtree_map<fat, int> x(p, btree::flags::truncate);
    construct_new_test(x, p);
  }

  cout << "    construct_new complete" << endl;
}

//-----------------------------------  single_insert  ----------------------------------//

void  single_insert()
{
  cout << "  single_insert..." << endl;

  {
    fs::path p("btree_map.btree");
    fs::remove(p);

    typedef btree::vbtree_map<int, int> btree_type;

    btree_type x(p, btree::flags::read_write, 128);

    std::pair<btree_type::const_iterator, bool> result
      = x.insert(123, 456);

    BOOST_TEST_EQ(x.size(), 1);
    BOOST_TEST(result.second);
    BOOST_TEST_EQ(result.first->key(), 123);
    BOOST_TEST_EQ(result.first->mapped_value(), 456);
  }
 
  cout << "     single_insert complete" << endl;
}

//------------------------------------ open_existing -----------------------------------//

void open_existing()
{
  cout << "  open_existing..." << endl;
  fs::path p("btree_map.btree");

  {
    fs::remove(p);
    btree::vbtree_map<fat, int> bt(p, btree::flags::truncate, 128);

    fat key = 5;
    int mapped = 0x55;
    bt.insert(key, mapped);
    key = 4; mapped = 0x44;
    bt.insert(key, mapped);
    key = 6; mapped = 0x66;
    bt.insert(key, mapped);
  }

  btree::vbtree_map<fat, int> bt2(p);
  BOOST_TEST(bt2.is_open());
  BOOST_TEST(!bt2.empty());
  BOOST_TEST_EQ(bt2.size(), 3U);
  BOOST_TEST_EQ(bt2.page_size(), 128U);
  BOOST_TEST_EQ(bt2.header().element_count(), 3U);
  BOOST_TEST_EQ(bt2.header().page_size(), 128U);

  bt2.dump_dot(std::cout);

  // TODO: test each header value

  cout << "    open_existing complete" << endl;
}

//--------------------------------- small_variable_set ---------------------------------//

void small_variable_set()
{
  cout << "  small_variable_set..." << endl;

  fs::path p("btree_set.btree");
  fs::remove(p);
  btree::vbtree_set<btree::strbuf> bt(p, btree::flags::truncate, 128);

  btree::strbuf stuff;

  stuff = "now";
  bt.insert(stuff);
  stuff = "is";
  bt.insert(stuff);
  stuff = "the";
  bt.insert(stuff);
  stuff = "time";
  bt.insert(stuff);
  stuff = "when";
  bt.insert(stuff);
  stuff = "all";
  bt.insert(stuff);
  stuff = "good";
  bt.insert(stuff);
  stuff = "...";
  bt.insert(stuff);

  BOOST_TEST(bt.is_open());
  BOOST_TEST(!bt.empty());
  BOOST_TEST_EQ(bt.size(), 8U);
  BOOST_TEST_EQ(bt.page_size(), 128U);
  BOOST_TEST_EQ(bt.header().element_count(), 8U);
  BOOST_TEST_EQ(bt.header().page_size(), 128U);

  bt.dump_dot(std::cout);

  cout << "    small_variable_set complete" << endl;
}

////------------------------------- btree_less ---------------------------------------------//
//
//void btree_less()
//{
//  cout << "  btree_less..." << endl;
//
//  const char* list[] = {"bb", "b", "a", "aa"};
//
//  for (int i = 0; i < 4; ++i) 
//    cout << list[i] << '\n';
//  cout << endl;
//
//  std::sort(&list[0], &list[4], btree::less<const char*>());
//
//  for (int i = 0; i < 4; ++i) 
//    cout << list[i] << '\n';
//
//  int i1=4;
//  int i2=3;
//  int i3=1;
//  int i4=2;
//  const int* list_int[] = {&i1, &i2, &i3, &i4};
//
//  for (int i = 0; i < 4; ++i) 
//    cout << *list_int[i] << '\n';
//  cout << endl;
//
//  std::sort(&list_int[0], &list_int[4], btree::less<const int*>());
//
//  for (int i = 0; i < 4; ++i) 
//    cout << *list_int[i] << '\n';
//  cout << endl;
//
//  cout << "    btree_less complete" << endl;
//}
//
////------------------------------- compare_function_objects -----------------------------//
//
//template <class BT>
//void compare_function_objects_test(BT& bt)
//{
//}
//
//void compare_function_objects()
//{
//  cout << "  compare_function_objects..." << endl;
//  {
//    btree::vbtree_set<int> bt;
//
//    int i2 = 2;
//    int i1 = 1;
//     
//    BOOST_TEST(bt.key_comp()(&i1, &i2));
//    BOOST_TEST(!bt.key_comp()(&i1, &i1));
//    BOOST_TEST(!bt.key_comp()(&i2, &i1));
//    BOOST_TEST(bt.value_comp()(&i1, &i2));
//    BOOST_TEST(!bt.value_comp()(&i1, &i1));
//    BOOST_TEST(!bt.value_comp()(&i2, &i1));
//  }
//
//  {
//    btree::vbtree_map<int, long> bt;
//
//    int i2 = 2;
//    int i1 = 1;
//
//    BOOST_TEST(bt.key_comp()(&i1, &i2));
//    BOOST_TEST(!bt.key_comp()(&i1, &i1));
//    BOOST_TEST(!bt.key_comp()(&i2, &i1));
//
//    struct my_pair : public btree::vbtree_value<const int, const long>
//    {
//      int first;
//      int second;
//    };
//
//    my_pair pr2, pr1;
//    pr1.first = 1;
//    pr2.first = 2;
//
//    BOOST_TEST(bt.value_comp()(pr1, pr2));
////    BOOST_TEST(!bt.value_comp()(pr1, pr1));
////    BOOST_TEST(!bt.value_comp()(pr2, pr1));
//  }
//
//  //{
//  //  btree::vbtree_multimap x;
//  //}
//  //{
//  //  btree::vbtree_multiset x;
//  //}
//
//  cout << "    compare_function_objects complete" << endl;
//}

////------------------------------------ alignment ---------------------------------------//
//
//void alignment()
//{
//  cout << "  alignment..." << endl;
//  cout << "    alignment complete" << endl;
//}

//------------------------------------- insert -----------------------------------------//

template <class BTree>
void insert_tests(BTree& bt)
{
  cout << "  testing \"" << bt.file_path().string() << "\" ..." << endl;
  cout << '\n' << bt.manager() << '\n';

  BOOST_TEST(bt.size() == 0U);
  BOOST_TEST(bt.empty());
  BOOST_TEST(!bt.read_only());
  BOOST_TEST(bt.page_size() == 128);
  BOOST_TEST(bt.begin() == bt.end());

  typename BTree::const_iterator empty_iterator, begin, end, cur;
  begin = bt.begin();
  end = bt.end();
  BOOST_TEST(begin == end);
  BOOST_TEST(bt.find(fat(0)) == bt.end());

  typename BTree::key_type key(0x0C);
  typename BTree::mapped_type mapped_value(0xCCCCCCCC);

  std::pair<typename BTree::const_iterator, bool> result;

  result = bt.insert(key, mapped_value);
  BOOST_TEST(result.second);
  BOOST_TEST(result.first->key() == key);
  BOOST_TEST_EQ(result.first->mapped_value(), mapped_value);
  BOOST_TEST(bt.size() == 1U);
  BOOST_TEST(!bt.empty());
  BOOST_TEST(bt.begin() != bt.end());
  cur = bt.find(key);
  BOOST_TEST(cur != bt.end());
  BOOST_TEST(cur->key() == key);
  BOOST_TEST(cur->mapped_value() == mapped_value);
  BOOST_TEST(bt.find(fat(0)) == bt.end());
  BOOST_TEST(bt.find(fat(1000)) == bt.end());

  key = 0x0A;
  mapped_value = 0xAAAAAAAA;
  result = bt.insert(key, mapped_value);
  BOOST_TEST(result.second);
  BOOST_TEST_EQ(result.first->key().x, key.x);
  BOOST_TEST_EQ(result.first->mapped_value(), mapped_value);
  BOOST_TEST_EQ(bt.find(0x0A)->key().x, 0x0A);
  BOOST_TEST_EQ(bt.find(0x0C)->key().x, 0x0C);

  key = 0x0E;
  mapped_value = 0xEEEEEEEE;
  result = bt.insert(key, mapped_value);
  BOOST_TEST(result.second);
  BOOST_TEST_EQ(result.first->key().x, key.x);
  BOOST_TEST_EQ(result.first->mapped_value(), mapped_value);
  BOOST_TEST_EQ(bt.find(0x0E)->key().x, 0x0E);
  BOOST_TEST_EQ(bt.find(0x0A)->key().x, 0x0A);
  BOOST_TEST_EQ(bt.find(0x0C)->key().x, 0x0C);

  key = 0x0B;
  mapped_value = 0xBBBBBBBB;
  result = bt.insert(key, mapped_value);
  BOOST_TEST(result.second);
  BOOST_TEST_EQ(result.first->key().x, key.x);
  BOOST_TEST_EQ(result.first->mapped_value(), mapped_value);
  BOOST_TEST_EQ(bt.find(0x0B)->key().x, 0x0B);
  BOOST_TEST_EQ(bt.find(0x0E)->key().x, 0x0E);
  BOOST_TEST_EQ(bt.find(0x0A)->key().x, 0x0A);
  BOOST_TEST_EQ(bt.find(0x0C)->key().x, 0x0C);

  key = 0x0D;
  mapped_value = 0xDDDDDDDD;
  result = bt.insert(key, mapped_value);
  BOOST_TEST(result.second);
  BOOST_TEST_EQ(result.first->key().x, key.x);
  BOOST_TEST_EQ(result.first->mapped_value(), mapped_value);
  BOOST_TEST_EQ(bt.find(0x0D)->key().x, 0x0D);
  BOOST_TEST_EQ(bt.find(0x0B)->key().x, 0x0B);
  BOOST_TEST_EQ(bt.find(0x0E)->key().x, 0x0E);
  BOOST_TEST_EQ(bt.find(0x0A)->key().x, 0x0A);
  BOOST_TEST_EQ(bt.find(0x0C)->key().x, 0x0C);
  bt.flush();

  BOOST_TEST_EQ(bt.size(), 5U);

  BOOST_TEST_EQ(bt.find(0x0A)->key().x, 0x0A);
  BOOST_TEST_EQ(bt.find(0x0B)->key().x, 0x0B);
  BOOST_TEST_EQ(bt.find(0x0C)->key().x, 0x0C);
  BOOST_TEST_EQ(bt.find(0x0D)->key().x, 0x0D);
  BOOST_TEST_EQ(bt.find(0x0E)->key().x, 0x0E);

  cur = begin = end = empty_iterator;
  //cout << '\n' << bt.manager() << '\n';
  //cout << "\nroot is page " << bt.header().root_page_id() << ", size() " << bt.size() << '\n'; 
  //bt.dump_dot(std::cout);

  cur = bt.begin();
  BOOST_TEST_EQ(cur->key().x, 0x0A);
  ++cur;
  BOOST_TEST_EQ(cur->key().x, 0x0B);
  ++cur;
  BOOST_TEST_EQ(cur->key().x, 0x0C);
  ++cur;
  BOOST_TEST_EQ(cur->key().x, 0x0D);
  ++cur;
  BOOST_TEST_EQ(cur->key().x, 0x0E);
  ++cur;
  BOOST_TEST(cur == bt.end());
  --cur;
  BOOST_TEST_EQ(cur->key().x, 0x0E);
  --cur;
  BOOST_TEST_EQ(cur->key().x, 0x0D);
  --cur;
  BOOST_TEST_EQ(cur->key().x, 0x0C);
  --cur;
  BOOST_TEST_EQ(cur->key().x, 0x0B);
  --cur;
  BOOST_TEST_EQ(cur->key().x, 0x0A);
  BOOST_TEST(cur == bt.begin());

  BOOST_TEST_EQ(bt.last()->key().x, 0x0E);

  // erase tests

  cout << "\n  erase all elements" << endl;
  cur = bt.find(0x0C);
  BOOST_TEST(cur != bt.end());
  cout << "    erase " << cur->key() << endl;
  cur = bt.erase(cur);

  BOOST_TEST_EQ(cur->key().x, 0x0D);
  BOOST_TEST_EQ(bt.size(), 4U);

  cur = bt.find(0x0B);
  BOOST_TEST(cur != bt.end());
  cout << "    erase " << cur->key() << endl;
  cur = bt.erase(cur);

  BOOST_TEST_EQ(cur->key().x, 0x0D);
  BOOST_TEST_EQ(bt.size(), 3U);

  cur = bt.find(0x0E);
  BOOST_TEST(cur != bt.end());

  //cout << "root is page " << bt.header().root_page_id() << '\n'; 
  //bt.dump_dot(std::cout);

  cout << "    erase " << cur->key() << endl;
  cur = bt.erase(cur);

  BOOST_TEST(cur == bt.end());
  BOOST_TEST_EQ(bt.size(), 2U);
  BOOST_TEST_EQ(bt.header().root_page_id(), 2U);
  BOOST_TEST_EQ(bt.header().root_level(), 1);

  cur = bt.find(0x0A);
  BOOST_TEST(cur != bt.end());
  cout << "    erase " << cur->key() << endl;
  cur = bt.erase(cur);

  BOOST_TEST(cur != bt.end());
  BOOST_TEST_EQ(cur->key().x, 0x0D);
  BOOST_TEST(bt.begin() == cur);
  BOOST_TEST_EQ(bt.size(), 1U);
  BOOST_TEST_EQ(bt.header().root_page_id(), 4U);
  BOOST_TEST_EQ(bt.header().root_level(), 0);

  cur = bt.find(0x0D);

  //cout << "root is page " << bt.header().root_page_id() << '\n'; 
  //bt.dump_dot(std::cout);

  BOOST_TEST(cur != bt.end());

  cout << "    erase " << cur->key() << endl;
  cur = bt.erase(cur);

  BOOST_TEST(cur == bt.end());
  BOOST_TEST(bt.begin() == bt.end());
  BOOST_TEST_EQ(bt.size(), 0U);
  BOOST_TEST_EQ(bt.header().root_page_id(), 4U);
  BOOST_TEST_EQ(bt.header().root_level(), 0);
  
  //cout << "root is page " << bt.header().root_page_id() << '\n'; 
  //bt.dump_dot(std::cout);

  cout << "\n  add enough elements to force branch page splits" << endl;
  for (int i = 1; i <= 21; ++i )
  {
// std::cout << "\n inserting " << i << std::endl;
    key = i;
    mapped_value = i * 100;
    bt.insert(key, mapped_value);
//std::cout << "root is page " << bt.header().root_page_id() << '\n'; 
  //bt.dump_dot(std::cout);
  }
  BOOST_TEST_EQ(bt.size(), 21U);
  
  //cout << "root is page " << bt.header().root_page_id() << '\n'; 
  //bt.dump_dot(std::cout);

  cout << "\n  erase every other element" << endl;
  for (int i = 1; i <= 21; i += 2 )
  {
    BOOST_TEST_EQ(bt.erase(i), 1U);
    BOOST_TEST_EQ(bt.erase(i), 0U);
  }
  BOOST_TEST_EQ(bt.size(), 10U);

  //cout << "root is page " << bt.header().root_page_id() << '\n'; 
  //bt.dump_dot(std::cout);

  cout << "\n  erase remaining elements and attempt to erase nonexistant elements" << endl;
  for (int i = 1; i <= 31; ++i )  // many of these won't exist
  {
    //cout << "\n  erase " << i << endl;
    typename BTree::size_type ct
      = bt.erase(i);
    //cout << "     erase count " << ct << ", size() " << bt.size() << endl;
    //cout << "root is page " << bt.header().root_page_id() << '\n'; 
    //bt.dump_dot(std::cout);
  }
  BOOST_TEST_EQ(bt.size(), 0U);

  bt.flush();
  cout << '\n' << bt << '\n';

  cout << "  testing \"" << bt.file_path().string() << "\" complete" << endl;
}

void insert()
{
  cout << "  insert..." << endl;

  //  these tests use a value type that is large relative to the page size, thus stressing
  //  the code by causing a lot of page splits 

  {
    fs::path map_path("btree_map.btree");
    btree::vbtree_map<fat, int> map(map_path, btree::flags::truncate, 128);
    map.max_cache_pages(0);  // maximum stress
    insert_tests(map);
  }

  cout << "    insert complete" << endl;
}

//---------------------------------- find_and_bounds -----------------------------------//

typedef btree::vbtree_set<int> fb_set_type;
typedef btree::vbtree_multiset<int> fb_multiset_type;
typedef btree::vbtree_map<fat, int> fb_map_type;
typedef btree::vbtree_multimap<fat, int> fb_multimap_type;

void do_fb_insert(fb_set_type& bt, int i)
{
  bt.insert(i);
}
void do_fb_insert(fb_multiset_type& bt, int i)
{
  bt.insert(i);
}
void do_fb_insert(fb_map_type& bt, int i)
{
  fat k(i);
  bt.insert(k, i*100);
}
void do_fb_insert(fb_multimap_type& bt, int i)
{
  fat k(i);
  bt.insert(k, i*100);
}

template <class BTree>
void find_and_bounds_tests(BTree& bt)
{
  cout << "    testing \"" << bt.file_path().string() << "\" ..." << endl;

  for (int i = 1; i < 18; i += 2)
  {
    do_fb_insert(bt, i);
    //std::cout << "   size is " << bt.size() << std::endl;
    //bt.dump_dot(std::cout);
  }

  BOOST_TEST_EQ(bt.size(), 9U);

  if (!(bt.header().flags() & btree::flags::unique))
  {
    //cout << "root is page " << bt.header().root_page_id() << '\n'; 
    //bt.dump_dot(std::cout);
    //cout << "insert 3" << endl;
    do_fb_insert(bt, 3);
    //cout << "root is page " << bt.header().root_page_id() << '\n'; 
    //bt.dump_dot(std::cout);
    //cout << "insert 7" << endl;
    do_fb_insert(bt, 7);
    //cout << "root is page " << bt.header().root_page_id() << '\n'; 
    //bt.dump_dot(std::cout);
    //cout << "insert 7" << endl;
    do_fb_insert(bt, 7);
    for (int i = 0; i < 10; ++i)
    {
      //cout << "root is page " << bt.header().root_page_id() << '\n'; 
      //bt.dump_dot(std::cout);

      //cout << "insert 15" << endl;
      do_fb_insert(bt, 15);
    }
    //cout << "root is page " << bt.header().root_page_id() << '\n'; 
    //bt.dump_dot(std::cout);


    BOOST_TEST_EQ(bt.size(), 22U);
  }

  //cout << "root is page " << bt.header().root_page_id() << '\n'; 
  //bt.dump_dot(std::cout);

  //             i =   0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18
  const int  lwr[] = { 1, 1, 3, 3, 5, 5, 7, 7, 9, 9, 11, 11, 13, 13, 15, 15, 17, 17, -1};
  const int  upr[] = { 1, 3, 3, 5, 5, 7, 7, 9, 9,11, 11, 13, 13, 15, 15, 17, 17, -1, -1};
  const int  fnd[] = {-1, 1,-1, 3,-1, 5,-1, 7,-1, 9, -1, 11, -1, 13, -1, 15, -1, 17, -1};
  const unsigned
             cnt[] = { 0, 1, 0, 2, 0, 1, 0, 3, 0, 1,  0,  1,  0,  1,  0, 11,  0,  1,  0};

  for (int i = 0; i <= 18; ++i)
  {
    //std::cout << "lower_bound " << i << " is " <<
    //  (bt.lower_bound(i) == bt.end() ? 99999 : bt.key(*bt.lower_bound(i))) << std::endl;


    BOOST_TEST((bt.lower_bound(i) != bt.end() && bt.key(*bt.lower_bound(i)) == lwr[i])
      || (bt.lower_bound(i) == bt.end() && lwr[i] == -1));

    BOOST_TEST((bt.upper_bound(i) != bt.end() && bt.key(*bt.upper_bound(i)) == upr[i])
      || (bt.upper_bound(i) == bt.end() && upr[i] == -1));

    BOOST_TEST((bt.find(i) != bt.end() && bt.key(*bt.find(i)) == fnd[i])
      || (bt.find(i) == bt.end() && fnd[i] == -1));

    if (bt.header().flags() & btree::flags::unique)
      BOOST_TEST(bt.count(i) == (cnt[i] ? 1 : 0));
    else
      BOOST_TEST(bt.count(i) == cnt[i]);

//    cout << "      i = " << i << ", bt.count(i) = " << bt.count(i) <<endl;
  }

  cout << "    testing \"" << bt.file_path().string() << "\" complete" << endl;
}

void find_and_bounds()
{
  cout << "  find_and_bounds..." << endl;

  {
    fb_set_type set("find_and_bounds_set.btr", btree::flags::truncate, 128);
    BOOST_TEST(set.header().flags() & btree::flags::unique);
    BOOST_TEST(set.header().flags() & btree::flags::key_only);
    set.max_cache_pages(0);  // maximum stress
    find_and_bounds_tests(set);
  }

  {
    fb_multiset_type multiset("find_and_bounds_multiset.btr",
      btree::flags::truncate, 128);
    BOOST_TEST(!(multiset.header().flags() & btree::flags::unique));
    BOOST_TEST(multiset.header().flags() & btree::flags::key_only);
    multiset.max_cache_pages(0);  // maximum stress
    find_and_bounds_tests(multiset);
  }

  //  these tests use a value type that is large relative to the page size, thus stressing
  //  the code by causing a lot of page splits 

  {
    fb_map_type map("find_and_bounds_map.btr",
      btree::flags::truncate, 128);
    BOOST_TEST(map.header().flags() & btree::flags::unique);
    BOOST_TEST(!(map.header().flags() & btree::flags::key_only));
    map.max_cache_pages(0);  // maximum stress
    find_and_bounds_tests(map);
  }

  {
    fb_multimap_type multimap("find_and_bounds_multimap.btr",
      btree::flags::truncate, 128);
    BOOST_TEST(!(multimap.header().flags() & btree::flags::unique));
    BOOST_TEST(!(multimap.header().flags() & btree::flags::key_only));
    multimap.max_cache_pages(0);  // maximum stress
    find_and_bounds_tests(multimap);
  }

  cout << "    find_and_bounds complete" << endl;
}

//---------------------------------- insert_non_unique -----------------------------------//

template <class BTree>
void insert_non_unique_tests(BTree& bt)
{
  BOOST_TEST(!(bt.header().flags() & btree::flags::unique));
  
  typename BTree::const_iterator result;

  const int n = 12;
  cout << "  testing with " << n << " equal elements ..." << endl;

  fat k;

  for (int i = 1; i <= n; ++i)
  {
    result = bt.insert(k = 3, i);
    BOOST_TEST_EQ(bt.size(), static_cast<unsigned>(i));
    BOOST_TEST_EQ(result->key().x, 3);
    BOOST_TEST_EQ(result->mapped_value(), i);

    //cout << "root is page " << bt.header().root_page_id() << '\n'; 
    //bt.dump_dot(std::cout);
   
    int j = 0;
    for (typename BTree::const_iterator_range range = bt.equal_range(3);
         range.first != range.second; ++range.first)
    {
      //cout << range.first->key().x << ", " << range.first->mapped_value() << endl;
      ++j;
      BOOST_TEST_EQ(range.first->key().x, 3);
      BOOST_TEST_EQ(range.first->mapped_value(), j);
    }
    BOOST_TEST_EQ(j, i);
  }

  cout << "  testing \"" << bt.file_path().string() << "\" complete" << endl;
}

void insert_non_unique()
{
  cout << "  insert_non_unique..." << endl;

  //  these tests use a value type that is large relative to the page size, thus stressing
  //  the code by causing a lot of page splits 

  {
    fs::path map_path("non_unique.btr");
    btree::vbtree_multimap<fat, int> multimap(map_path,
      btree::flags::truncate, 128);
    multimap.max_cache_pages(0);  // maximum stress
    insert_non_unique_tests(multimap);
  }

  cout << "    insert_non_unique complete" << endl;
}

//--------------------------------  update_test  --------------------------------------//

void update_test()
{
  cout << "  update_test..." << endl;

  typedef btree::vbtree_map<fat, int> bt_type;
  bt_type bt("update.btr", btree::flags::truncate, 128);

  boost::mt19937 rng;
  boost::uniform_int<> million(1,1000000);
  boost::variate_generator<boost::mt19937&, boost::uniform_int<> >
           random_value(rng, million);

  for (int n = 1; n < 1000; ++n)
  {
    bt.insert(fat(random_value()), n);
  }

  for (bt_type::iterator itr = bt.begin(); itr != bt.end(); ++itr)
  {
//    cout << "    " << itr->key() << "," << itr->mapped_value() << '\n';
    int n = itr->mapped_value();
    bt_type::iterator new_itr =  bt.update(itr, -n);
    BOOST_TEST(itr == new_itr);
    BOOST_TEST(new_itr->mapped_value() == -n);
  }

  for (bt_type::iterator itr = bt.begin(); itr != bt.end(); ++itr)
  {
//    cout << "    " << itr->key() << "," << itr->mapped_value() << '\n';
    BOOST_TEST(itr->mapped_value() <= 0);
  }

  cout << "     update_test complete" << endl;
}

////-------------------------------------- erase -----------------------------------------//
//
//void erase()
//{
//  cout << "  erase..." << endl;
//  cout << "    erase complete" << endl;
//}
//
////------------------------------------ iteration ---------------------------------------//
//
//void iteration()
//{
//  cout << "  iteration..." << endl;
//  cout << "    iteration complete" << endl;
//}
//
////-------------------------------------  multi -----------------------------------------//
//
//void  multi()
//{
//  cout << "   multi..." << endl;
//  cout << "     multi complete" << endl;
//}
//
////--------------------------- parent_pointer_to_split_page -----------------------------//
//
//void parent_pointer_to_split_page()
//{
//  cout << "  parent_pointer_to_split_page..." << endl;
//  cout << "    parent_pointer_to_split_page complete" << endl;
//}
//
////----------------------------- parent_pointer_lifetime --------------------------------//
//
//void parent_pointer_lifetime()
//{
//  cout << "  parent_pointer_lifetime..." << endl;
//  cout << "    parent_pointer_lifetime complete" << endl;
//}

//-------------------------------- pack_optimization -----------------------------------//

void pack_optimization()
{
  cout << "  pack_optimization..." << endl;

  typedef std::pair<int, int> value_type;
  const int page_sz = 128;
  const int overhead = 12;
  const int per_page = (page_sz - overhead) / sizeof(value_type);
  const int n = per_page * 2;  // sufficient to distinguish if pack optimization works

  btree::vbtree_map<int, int> np("not_packed.btr", btree::flags::truncate, page_sz);
  for (int i=n; i > 0; --i)
    np.insert(i, 0xffffff00+i);

  cout << "\nroot is page " << np.header().root_page_id() << '\n'; 
  np.dump_dot(std::cout);
  
  btree::vbtree_map<int, int> p("packed.btr", btree::flags::truncate, page_sz);
  for (int i=1; i <= n; ++i)
    p.insert(i, 0xffffff00+i);

  cout << "\nroot is page " << p.header().root_page_id() << '\n'; 
  p.dump_dot(std::cout);

  BOOST_TEST_EQ(np.size(), p.size());
  BOOST_TEST(p.header().page_count() < np.header().page_count());

  cout << "    pack_optimization complete" << endl;
}

////-------------------------------------  fixstr ----------------------------------------//
//
//void  fixstr()
//{
//  cout << "  fixstr..." << endl;
//
//  typedef boost::detail::fixstr<15>       str_t;
//  typedef btree::vbtree_map<str_t, str_t>  map_t;
//
//  map_t bt("fixstr.btr", btree::flags::truncate);
//
//  bt.insert(make_pair(str_t("Tyler"), str_t("jet black")));
//  bt.insert(make_pair(str_t("Harry"), str_t("black & white")));
//
//  map_t::const_iterator it = bt.begin();
//  BOOST_TEST(it->first == "Harry");
//  BOOST_TEST(it->second == "black & white");
//  ++it;
//  BOOST_TEST(it->first == "Tyler");
//  BOOST_TEST(it->second == "jet black");
//  ++it;
//  BOOST_TEST(it == bt.end());
//
//  cout << "     fixstr complete" << endl;
//}

//-------------------------------------  _test  ----------------------------------------//

void  _test()
{
  cout << "  _test..." << endl;

  cout << "     _test complete" << endl;
}

}  // unnamed namespace

//------------------------------------ cpp_main ----------------------------------------//

int cpp_main(int, char*[])
{
  instantiate();
  types_test();
  //btree_less();
  //compare_function_objects();
  construct_new();
  single_insert();
  open_existing();
  small_variable_set();
  //alignment();
  insert();
  insert_non_unique();
  find_and_bounds();
  //erase();
  update_test();
  //iteration();
  //multi();
  //parent_pointer_to_split_page();
  //parent_pointer_lifetime();
  pack_optimization();
  //fixstr();
  

  //{
  //  cout << "btree_multimap tests..." << endl;
  //  fs::path multimap_path("btree_multimap.btree");
  //  btree::vbtree_multimap<int,long> multimap(multimap_path, btree::flags::truncate);
  //}

  //{
  //  cout << "btree_set tests..." << endl;
  //  fs::path set_path("btree_set.btree");
  //  btree::vbtree_set<int,long> set(set_path, btree::flags::truncate);
  //}

  //{
  //  cout << "btree_multiset tests..." << endl;
  //  fs::path multiset_path("btree_multiset.btree");
  //  btree::vbtree_multiset<int,long> multiset(multiset_path, btree::flags::truncate);
  //}

  cout << "all tests complete" << endl;

  return boost::report_errors();
}


