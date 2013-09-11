//  index_unit_test.cpp  ---------------------------------------------------------------//

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//--------------------------------------------------------------------------------------//
//                                                                                      //
//  These tests lightly exercise many portions of the interface. They do not attempt    //
//  to stress the many combinations of control paths possible in large scale use.       //
//                                                                                      //
//--------------------------------------------------------------------------------------//

#include <boost/config/warning_disable.hpp>

#include <iostream>
#include <iomanip>
#include <boost/btree/btree_index_set.hpp>
#include <boost/btree/btree_index_map.hpp>
#include <boost/btree/support/random_string.hpp>
#include <boost/detail/lightweight_main.hpp>
#include <boost/detail/lightweight_test.hpp>

#include <iomanip>
#include <utility>
#include <set>
#include <algorithm>
#include <cstring>

using namespace boost;
namespace fs = boost::filesystem;
using std::cout;
using std::endl;
using std::make_pair;

namespace
{
bool dump_dot(false);
int n_volume = 1000;

filesystem::path file_path("test.file");
filesystem::path idx1_path("test.1.idx");
filesystem::path idx2_path("test.2.idx");

struct stuff
{
  int x;
  int y;
  char unused[24];

  stuff() : x(-1), y(-2) {}
  stuff(int x_, int y_) : x(x_), y(y_) {}
  stuff(const stuff& rhs) : x(rhs.x), y(rhs.y) {}
  stuff& operator=(const stuff& rhs) {x = rhs.x; y = rhs.y; return *this; }
  stuff& assign(int x_, int y_) {x = x_; y = y_; return *this; }

  bool operator<(const stuff& rhs) const
    {return x < rhs.x || (x == rhs.x && y < rhs.y);}
  bool operator==(const stuff& rhs) const {return x == rhs.x && y == rhs.y;}
  bool operator!=(const stuff& rhs) const {return x != rhs.x || y != rhs.y;}
};

std::ostream& operator<<(std::ostream& os, const stuff& s)
{
  os << s.x << "," << s.y;
  return os;
}

struct stuff_reverse_order
{
  bool operator()(const stuff& lhs, const stuff& rhs) const
  {
    return rhs<lhs;
  }
};

//-------------------------------------- instantiate -----------------------------------//

void instantiate_test()
{
  cout << "  instantiate_test..." << endl;

  // set
  {
    btree::btree_index_set<int> x;
    BOOST_TEST(!x.is_open());
    BOOST_TEST(x.size() == 0U);
    BOOST_TEST(x.empty());
  }
 
  cout << "    instantiate_test complete" << endl;
}


//---------------------------------  open_all_new_test  --------------------------------//

void  open_all_new_test()
{
  cout << "  open_all_new_test..." << endl;

  {
    cout << "      default construct, then open..." << endl;
    btree::btree_index_set<int> idx;
    idx.open(idx1_path, file_path, btree::flags::truncate, -1,
      btree::less(), 128);
    BOOST_TEST(idx.is_open());
    BOOST_TEST_EQ(idx.file_path(), file_path);
    BOOST_TEST_EQ(idx.file_size(), 0u);
    BOOST_TEST_EQ(idx.path(), idx1_path);
    BOOST_TEST_EQ(idx.size(), 0u);
  }

  {
    cout << "      open via constructor..." << endl;
    btree::btree_index_set<int> idx(idx1_path, file_path,
      btree::flags::truncate, -1, btree::less(), 128);
    BOOST_TEST(idx.is_open());
    BOOST_TEST_EQ(idx.file_path(), file_path);
    BOOST_TEST_EQ(idx.file_size(), 0u);
    BOOST_TEST_EQ(idx.path(), idx1_path);
    BOOST_TEST_EQ(idx.size(), 0u);
  }

  cout << "    open_all_new_test complete" << endl;
}

//-------------------------------  push_back_insert_pos_test  --------------------------//
                                                                 
void  push_back_insert_pos_test()
{
  cout << "  push_back_insert_pos_test..." << endl;

  {
    btree::btree_index_set<stuff> idx(idx1_path, file_path, 
                                btree::flags::truncate, -1, btree::less(), 128);
    stuff x(2,2);
    btree::btree_index_set<stuff>::file_position pos = idx.push_back(x);
    BOOST_TEST_EQ(pos, 0u);
    BOOST_TEST_EQ(idx.file_size(), sizeof(stuff));
    idx.insert_file_position(pos);
    BOOST_TEST_EQ(idx.size(), 1u);

    x.assign(3,1);
    pos = idx.push_back(x);
    BOOST_TEST_EQ(pos, sizeof(stuff));
    BOOST_TEST_EQ(idx.file_size(), 2*sizeof(stuff));
    idx.insert_file_position(pos);
    BOOST_TEST_EQ(idx.size(), 2u);

    x.assign(1,3);
    pos = idx.push_back(x);
    BOOST_TEST_EQ(pos, 2*sizeof(stuff));
    BOOST_TEST_EQ(idx.file_size(), 3*sizeof(stuff));
    idx.insert_file_position(pos);
    BOOST_TEST_EQ(idx.size(), 3u);
  }
  BOOST_TEST_EQ(boost::filesystem::file_size(file_path), 3*sizeof(stuff));

  cout << "     push_back_insert_pos_test complete" << endl;
}

//-------------------------------  iterator_test  -------------------------------//

void  iterator_test()
{
  cout << "  iterator_test..." << endl;

  typedef btree::btree_index_set<stuff> index_type;
  index_type idx(idx1_path, file_path);

  index_type::iterator itr = idx.begin();
  index_type::iterator end = idx.end();

  //*end;

  BOOST_TEST(itr != end);
  stuff s = *itr;
  BOOST_TEST_EQ(s.x, 1);
  BOOST_TEST_EQ(s.y, 3);

  ++itr;
  BOOST_TEST(itr != end);
  s = *itr;
  BOOST_TEST_EQ(s.x, 2);
  BOOST_TEST_EQ(s.y, 2);

  ++itr;
  BOOST_TEST(itr != end);
  s = *itr;
  BOOST_TEST_EQ(s.x, 3);
  BOOST_TEST_EQ(s.y, 1);

  ++itr;
  BOOST_TEST(itr == end);

  cout << "     iterator_test complete" << endl;
}

//----------------------------------  lower_bound_test  --------------------------------//

//  insert() depends on find(), which depends on lower_bound(), so test lower_bound() now

void  lower_bound_test()
{
  cout << "  lower_bound_test..." << endl;

  typedef btree::btree_index_set<stuff> index_type;
  index_type idx(idx1_path, file_path);

  index_type::iterator itr1 = idx.begin();
  index_type::iterator itr2 = idx.begin(); ++itr2;
  index_type::iterator itr3 = idx.begin(); ++itr3; ++itr3;

  stuff s00 (0,0);
  stuff s13 (1,3);
  stuff s3 (2,2);
  stuff s31 (3,1);
  stuff s12 (1,2);
  stuff s14 (1,4);
  stuff s32 (3,2);

  BOOST_TEST(idx.lower_bound(s00) == itr1);
  BOOST_TEST(idx.lower_bound(s13) == itr1);
  BOOST_TEST(idx.lower_bound(s3) == itr2);
  BOOST_TEST(idx.lower_bound(s31) == itr3);
  BOOST_TEST(idx.lower_bound(s12) == itr1);
  BOOST_TEST(idx.lower_bound(s14) == itr2);
  BOOST_TEST(idx.lower_bound(s32) == idx.end());

  cout << "     lower_bound_test complete" << endl;
}

//------------------------------------  find_test  -------------------------------------//

//  insert() depends on find(), so test find() now

void  find_test()
{
  cout << "  find_test..." << endl;

  typedef btree::btree_index_set<stuff> index_type;
  index_type idx(idx1_path, file_path);

  index_type::iterator itr1 = idx.begin();
  index_type::iterator itr2 = idx.begin(); ++itr2;
  index_type::iterator itr3 = idx.begin(); ++itr3; ++itr3;

  stuff s00 (0,0);
  stuff s13 (1,3);
  stuff s3 (2,2);
  stuff s31 (3,1);
  stuff s12 (1,2);
  stuff s14 (1,4);
  stuff s32 (3,2);

  BOOST_TEST(idx.find(s00) == idx.end());
  BOOST_TEST(idx.find(s13) == itr1);
  BOOST_TEST(idx.find(s3) == itr2);
  BOOST_TEST(idx.find(s31) == itr3);
  BOOST_TEST(idx.find(s12) == idx.end());
  BOOST_TEST(idx.find(s14) == idx.end());
  BOOST_TEST(idx.find(s32) == idx.end());

  cout << "     find_test complete" << endl;
}

//---------------------------------  insert_test  --------------------------------------//

//  dependencies have been tested, so test insert() now

void  insert_test()
{
  cout << "  insert_test..." << endl;

  typedef btree::btree_index_set<stuff> index_type;
  index_type idx(idx1_path, file_path, btree::flags::read_write);

  stuff s00 (0,0);
  stuff s13 (1,3);
  stuff s3 (2,2);
  stuff s31 (3,1);
  stuff s12 (1,2);
  stuff s14 (1,4);
  stuff s32 (3,2);

  std::pair<index_type::const_iterator, bool> result;

  result = idx.insert(s00);
  BOOST_TEST(result.second);
  BOOST_TEST_EQ(*result.first, s00);
  result = idx.insert(s13);
  BOOST_TEST(!result.second);
  result = idx.insert(s3);
  BOOST_TEST(!result.second);
  result = idx.insert(s31);
  BOOST_TEST(!result.second);
  result = idx.insert(s12);
  BOOST_TEST(result.second);
  BOOST_TEST_EQ(*result.first, s12);
  result = idx.insert(s14);
  BOOST_TEST(result.second);
  BOOST_TEST_EQ(*result.first, s14);
  result = idx.insert(s32);
  BOOST_TEST(result.second);
  BOOST_TEST_EQ(*result.first, s32);

  BOOST_TEST(idx.size() == 7u);

  BOOST_TEST(idx.find(s00) != idx.end());
  BOOST_TEST_EQ(*idx.find(s00), s00);
  BOOST_TEST(idx.find(s13) != idx.end());
  BOOST_TEST_EQ(*idx.find(s13), s13);
  BOOST_TEST(idx.find(s3) != idx.end());
  BOOST_TEST_EQ(*idx.find(s3), s3);
  BOOST_TEST(idx.find(s31) != idx.end());
  BOOST_TEST_EQ(*idx.find(s31), s31);
  BOOST_TEST(idx.find(s12) != idx.end());
  BOOST_TEST_EQ(*idx.find(s12), s12);
  BOOST_TEST(idx.find(s14) != idx.end());
  BOOST_TEST_EQ(*idx.find(s14), s14);
  BOOST_TEST(idx.find(s32) != idx.end());
  BOOST_TEST_EQ(*idx.find(s32), s32);

  cout << "     insert_test complete" << endl;
}

////-------------------------------  open_new_index_test  --------------------------------//
//
//void  open_new_index_test()
//{
//  cout << "  open_new_index_test with existing flat file..." << endl;
//
//  cout << "    open_new_index_test with existing flat file complete" << endl;
//}

//---------------------------------  two_index_test  -----------------------------------//

void  two_index_test()
{
  cout << "  two_index_test..." << endl;

  {
    btree::btree_index_set<stuff> idx1(idx1_path, file_path, btree::flags::truncate, -1,
      btree::less(), 128);
    btree::btree_index_set<stuff, btree::default_traits, stuff_reverse_order>
      idx2(idx2_path, idx1.file(), btree::flags::truncate, -1,
           stuff_reverse_order(), 128);

    stuff x(2,2);
    btree::btree_index_set<stuff>::file_position pos = idx1.push_back(x);
    idx1.insert_file_position(pos);
    idx2.insert_file_position(pos);

    x.assign(1,3);
    pos = idx1.push_back(x);
    idx1.insert_file_position(pos);
    idx2.insert_file_position(pos);

    x.assign(3,1);
    pos = idx1.push_back(x);
    idx1.insert_file_position(pos);
    idx2.insert_file_position(pos);

    BOOST_TEST_EQ(idx1.size(), 3u);
    BOOST_TEST_EQ(idx2.size(), 3u);
  }
  BOOST_TEST_EQ(boost::filesystem::file_size(file_path), 3*sizeof(stuff));

  cout << "     two_index_test complete" << endl;
}

//-------------------------------  two_index_iterator_test  -------------------------------//

void  two_index_iterator_test()
{
  cout << "  two_index_iterator_test..." << endl;

  {
    cout << "       idx1..." << endl;

    typedef btree::btree_index_set<stuff> index_type;
    index_type idx(idx1_path, file_path);

    index_type::iterator itr = idx.begin();
    index_type::iterator end = idx.end();

    BOOST_TEST(itr != end);
    stuff s = *itr;
    BOOST_TEST_EQ(s.x, 1);
    BOOST_TEST_EQ(s.y, 3);

    ++itr;
    BOOST_TEST(itr != end);
    s = *itr;
    BOOST_TEST_EQ(s.x, 2);
    BOOST_TEST_EQ(s.y, 2);

    ++itr;
    BOOST_TEST(itr != end);
    s = *itr;
    BOOST_TEST_EQ(s.x, 3);
    BOOST_TEST_EQ(s.y, 1);

    ++itr;
    BOOST_TEST(itr == end);

    cout << "         idx1 complete" << endl;
  }

  {
    cout << "       idx2..." << endl;

    typedef btree::btree_index_set<stuff,
      btree::default_traits, stuff_reverse_order> index_type;
    index_type idx(idx2_path, file_path);

    index_type::iterator itr = idx.begin();
    index_type::iterator end = idx.end();

    BOOST_TEST(itr != end);
    stuff s = *itr;
    BOOST_TEST_EQ(s.x, 3);
    BOOST_TEST_EQ(s.y, 1);

    ++itr;
    BOOST_TEST(itr != end);
    s = *itr;
    BOOST_TEST_EQ(s.x, 2);
    BOOST_TEST_EQ(s.y, 2);

    ++itr;
    BOOST_TEST(itr != end);
    s = *itr;
    BOOST_TEST_EQ(s.x, 1);
    BOOST_TEST_EQ(s.y, 3);

    ++itr;
    BOOST_TEST(itr == end);

    cout << "         idx2 complete" << endl;
  }

  cout << "     two_index_iterator_test complete" << endl;
}

////-------------------------------------  c_string_test  ----------------------------------------//
//
//struct c_str_less
//{
//  bool operator()(const char* x, const char* y) const
//  {
//    return std::strcmp(x, y) < 0;
//  }
//};
//
//
//void  c_string_test()
//{
//  cout << "  c_string_test..." << endl;
//
//  typedef btree::btree_index_set<const char*, btree::default_traits, c_str_less> index;
//  index idx(idx1_path, file_path.string() + ".c_string", btree::flags::truncate);
//
//  const char* s1 = "aa";
//  const char* s2 = "ccc";
//  const char* s3 = "b";
//  const char* s4 = "";
//
//  std::pair<index::const_iterator, bool> result;
//
//  result = idx.insert(s1);
//  BOOST_TEST(result.second);
//  BOOST_TEST(std::strcmp(*result.first, s1) == 0);
//  result = idx.insert(s2);
//  BOOST_TEST(result.second);
//  BOOST_TEST(std::strcmp(*result.first, s2) == 0);
//  result = idx.insert(s3);
//  BOOST_TEST(result.second);
//  BOOST_TEST(std::strcmp(*result.first, s3) == 0);
//  result = idx.insert(s4);
//  BOOST_TEST(result.second);
//  BOOST_TEST(std::strcmp(*result.first, s4) == 0);
// 
//  BOOST_TEST(idx.size() == 4u);
//
//  index::iterator itr = idx.begin();
//  index::iterator end = idx.end();
//
//  BOOST_TEST(itr != end);
//  const char* s = *itr;
//  BOOST_TEST(std::strcmp(s, s4) == 0);
//
//  ++itr;
//  BOOST_TEST(itr != end);
//  s = *itr;
//  BOOST_TEST(std::strcmp(s, s1) == 0);
//
//  ++itr;
//  BOOST_TEST(itr != end);
//  s = *itr;
//  BOOST_TEST(std::strcmp(s, s3) == 0);
//
//  ++itr;
//  BOOST_TEST(itr != end);
//  s = *itr;
//  BOOST_TEST(std::strcmp(s, s2) == 0);
// 
//  ++itr;
//  BOOST_TEST(itr == end);
//
//  BOOST_TEST(idx.find(s1) != idx.end());
//  BOOST_TEST(std::strcmp(*idx.find(s1), s1) == 0);
//  BOOST_TEST(idx.find(s2) != idx.end());
//  BOOST_TEST(std::strcmp(*idx.find(s2), s2) == 0);
//  BOOST_TEST(idx.find(s3) != idx.end());
//  BOOST_TEST(std::strcmp(*idx.find(s3), s3) == 0);
//  BOOST_TEST(idx.find(s4) != idx.end());
//  BOOST_TEST(std::strcmp(*idx.find(s4), s4) == 0);
//
//  BOOST_TEST(idx.find("ccc") != idx.end());
//  BOOST_TEST(std::strcmp(*idx.find("ccc"), s2) == 0);
//
//  result = idx.insert(s1);
//  BOOST_TEST(!result.second);
//  result = idx.insert(s2);
//  BOOST_TEST(!result.second);
//  result = idx.insert(s3);
//  BOOST_TEST(!result.second);
//  result = idx.insert(s4);
//  BOOST_TEST(!result.second);
// 
//  BOOST_TEST(idx.size() == 4u);
//
//  cout << "     c_string_test complete" << endl;
//}

//--------------------------------- string_view_test  ----------------------------------//


void  string_view_test()
{
  cout << "  string_view_test..." << endl;

  typedef btree::btree_index_set<string_view> index;
  index idx(idx1_path, file_path.string() + ".string_view", btree::flags::truncate);

  const char* s1 = "aa";
  const char* s2 = "ccc";
  const char* s3 = "b";
  const char* s4 = "";

  string_view sv1(s1);
  string_view sv2(s2);
  string_view sv3(s3);
  string_view sv4(s4);

  std::pair<index::const_iterator, bool> result;

  result = idx.insert(sv1);
  BOOST_TEST(result.second);
  BOOST_TEST(*result.first == sv1);
  //cout << '"' << *result.first << '"' << endl;

  result = idx.insert(sv2);
  BOOST_TEST(result.second);
  BOOST_TEST(*result.first == sv2);
  //cout << '"' << *result.first << '"' << endl;

  result = idx.insert(sv3);
  BOOST_TEST(result.second);
  BOOST_TEST(*result.first == sv3);
  //cout << '"' << *result.first << '"' << endl;

  result = idx.insert(sv4);
  BOOST_TEST(result.second);
  BOOST_TEST(*result.first == sv4);
  //cout << '"' << *result.first << '"' << endl;
 
  BOOST_TEST(idx.size() == 4u);
 //cout << "*********************" << endl;

  index::iterator itr = idx.begin();
  index::iterator end = idx.end();

  BOOST_TEST(itr != end);
  string_view sv = *itr;
  BOOST_TEST(sv == sv4);
  //cout << '"' << sv << '"' << endl;

  ++itr;
  BOOST_TEST(itr != end);
  sv = *itr;
  BOOST_TEST(sv == sv1);
  //cout << '"' << sv << '"' << endl;

  ++itr;
  BOOST_TEST(itr != end);
  sv = *itr;
  BOOST_TEST(sv == sv3);
  //cout << '"' << sv << '"' << endl;

  ++itr;
  BOOST_TEST(itr != end);
  sv = *itr;
  BOOST_TEST(sv == sv2);
  //cout << '"' << sv << '"' << endl;
 
  ++itr;
  BOOST_TEST(itr == end);

  BOOST_TEST(idx.find(s1) != idx.end());
  BOOST_TEST_EQ(*idx.find(s1), s1);
  BOOST_TEST(idx.find(sv1) != idx.end());
  BOOST_TEST_EQ(*idx.find(sv1), sv1);
  BOOST_TEST(idx.find("aa") != idx.end());
  BOOST_TEST_EQ(*idx.find(sv1), "aa");

  BOOST_TEST(idx.find(s2) != idx.end());
  BOOST_TEST_EQ(*idx.find(s2), s2);
  BOOST_TEST(idx.find(s3) != idx.end());
  BOOST_TEST_EQ(*idx.find(s3), s3);
  BOOST_TEST(idx.find(s4) != idx.end());
  BOOST_TEST_EQ(*idx.find(s4), s4);

  result = idx.insert(s1);
  BOOST_TEST(!result.second);
  result = idx.insert(s2);
  BOOST_TEST(!result.second);
  result = idx.insert(s3);
  BOOST_TEST(!result.second);
  result = idx.insert(s4);
  BOOST_TEST(!result.second);
 
  BOOST_TEST(idx.size() == 4u);

  BOOST_TEST(idx.find("a") == idx.end());
  BOOST_TEST(idx.find("ccca") == idx.end());

  BOOST_TEST(idx.lower_bound("b") != idx.end());
  BOOST_TEST_EQ(*idx.lower_bound("b"), "b");
  BOOST_TEST(idx.lower_bound("ba") != idx.end());
  BOOST_TEST_EQ(*idx.lower_bound("ba"), "ccc");
  BOOST_TEST(idx.lower_bound("ccca") == idx.end());

  BOOST_TEST(idx.upper_bound("b") != idx.end());
  BOOST_TEST_EQ(*idx.upper_bound("b"), "ccc");
  BOOST_TEST(idx.upper_bound("ba") != idx.end());
  BOOST_TEST_EQ(*idx.upper_bound("ba"), "ccc");
  BOOST_TEST(idx.upper_bound("ccc") == idx.end());
  BOOST_TEST(idx.upper_bound("ccca") == idx.end());

  BOOST_TEST_EQ(idx.count("a"), 0u);
  BOOST_TEST_EQ(idx.count("b"), 1u);
  BOOST_TEST_EQ(idx.count("ba"), 0u);
  BOOST_TEST_EQ(idx.count("ccc"), 1u);
  BOOST_TEST_EQ(idx.count("ccca"), 0u);

  std::string big(1000000, '*');

  idx.insert(big);

  idx.file()->close();

  BOOST_TEST_EQ(fs::file_size(file_path.string() + ".string_view"),
    static_cast<uintmax_t>(4 + 6 + 1000003)); 


  cout << "     string_view_test complete" << endl;
}

//--------------------------------- string_view_volume_test  ----------------------------------//

void  string_view_volume_test()
{
  cout << "  string_view_volume_test..." << endl;
//n_volume = 10;
  boost::random_string rsg(0, 10, 'a', 'z');
  std::string s;
  
  typedef btree::btree_index_set<string_view> index;
  index idx(idx1_path, btree::flags::truncate, -1, btree::less(), 128);

  // insert
  rsg.seed(1);
  index::size_type insert_count = 0;
  for (int i = 1; i <= n_volume; ++i)
  {
    s = rsg();
//    cout << "insert \"" << s << "\"\n";
    auto result = idx.insert(s);
    if (result.second)
      ++insert_count;
  }
  BOOST_TEST_EQ(idx.size(), insert_count);
  cout << idx << endl;

  // iterate
  std::string prior;
  for (auto itr = idx.begin(); itr != idx.end(); ++itr)
  {
    s.assign(itr->data(), itr->size());
    //cout << "size() is " << s.size() << endl;
    //cout << '"' << s << '"' << endl;
    BOOST_TEST(itr == idx.begin() || s > prior);
    prior = s;
  }
  cout << endl;

  // find
  rsg.seed(1);
  for (int i = 1; i <= n_volume; ++i)
  {
    s = rsg();
    auto result = idx.find(s);
    BOOST_TEST(result != idx.end());
    BOOST_TEST(*result == s);
  }

  cout << "     string_view_volume_test complete" << endl;
}

//---------------------------- string_view_multiset_test  ------------------------------//

void  string_view_multiset_test()
{
  cout << "  string_view_multiset_test..." << endl;

  typedef btree::btree_index_multiset<string_view> index;
  index idx(idx1_path, file_path.string() + ".string_view", btree::flags::truncate);

  const char* s1 = "aa";
  const char* s2 = "ccc";
  const char* s3 = "b";
  const char* s4 = "";

  string_view sv1(s1);
  string_view sv2(s2);
  string_view sv3(s3);
  string_view sv4(s4);

  index::const_iterator result;

  result = idx.insert(sv1);
  BOOST_TEST(*result == sv1);

  result = idx.insert(sv2);
  BOOST_TEST(*result == sv2);

  result = idx.insert(sv3);
  BOOST_TEST(*result == sv3);

  result = idx.insert(sv4);
  BOOST_TEST(*result == sv4);
 
  BOOST_TEST(idx.size() == 4u);

  index::iterator itr = idx.begin();
  index::iterator end = idx.end();

  BOOST_TEST(itr != end);
  string_view sv = *itr;
  BOOST_TEST(sv == sv4);

  ++itr;
  BOOST_TEST(itr != end);
  sv = *itr;
  BOOST_TEST(sv == sv1);

  ++itr;
  BOOST_TEST(itr != end);
  sv = *itr;
  BOOST_TEST(sv == sv3);

  ++itr;
  BOOST_TEST(itr != end);
  sv = *itr;
  BOOST_TEST(sv == sv2);
 
  ++itr;
  BOOST_TEST(itr == end);

  BOOST_TEST(idx.find(s1) != idx.end());
  BOOST_TEST_EQ(*idx.find(s1), s1);
  BOOST_TEST(idx.find(sv1) != idx.end());
  BOOST_TEST_EQ(*idx.find(sv1), sv1);
  BOOST_TEST(idx.find("aa") != idx.end());
  BOOST_TEST_EQ(*idx.find(sv1), "aa");

  BOOST_TEST(idx.find(s2) != idx.end());
  BOOST_TEST_EQ(*idx.find(s2), s2);
  BOOST_TEST(idx.find(s3) != idx.end());
  BOOST_TEST_EQ(*idx.find(s3), s3);
  BOOST_TEST(idx.find(s4) != idx.end());
  BOOST_TEST_EQ(*idx.find(s4), s4);

  idx.insert(s4);
  idx.insert(s3);
  idx.insert(s2);
  idx.insert(s1);
 
  BOOST_TEST(idx.size() == 8u);

  BOOST_TEST_EQ(idx.erase(string_view("aa")), 2u);
  BOOST_TEST(idx.size() == 6u);

  BOOST_TEST(idx.find("a") == idx.end());
  BOOST_TEST(idx.find("aa") == idx.end());
  BOOST_TEST(idx.find("ccca") == idx.end());

  BOOST_TEST(idx.lower_bound("b") != idx.end());
  BOOST_TEST_EQ(*idx.lower_bound("b"), "b");
  BOOST_TEST(idx.lower_bound("ba") != idx.end());
  BOOST_TEST_EQ(*idx.lower_bound("ba"), "ccc");
  BOOST_TEST(idx.lower_bound("ccca") == idx.end());

  BOOST_TEST(idx.upper_bound("b") != idx.end());
  BOOST_TEST_EQ(*idx.upper_bound("b"), "ccc");
  BOOST_TEST(idx.upper_bound("ba") != idx.end());
  BOOST_TEST_EQ(*idx.upper_bound("ba"), "ccc");
  BOOST_TEST(idx.upper_bound("ccc") == idx.end());
  BOOST_TEST(idx.upper_bound("ccca") == idx.end());

  BOOST_TEST_EQ(idx.count(""), 2u);
  BOOST_TEST_EQ(idx.count("a"), 0u);
  BOOST_TEST_EQ(idx.count("b"), 2u);
  BOOST_TEST_EQ(idx.count("ba"), 0u);
  BOOST_TEST_EQ(idx.count("ccc"), 2u);
  BOOST_TEST_EQ(idx.count("ccca"), 0u);

  itr = idx.erase(idx.begin(), idx.find("b"));
  index::iterator itr2 = idx.find("b");
  BOOST_TEST(itr == itr2);
  BOOST_TEST_EQ(idx.size(), 4u);
  BOOST_TEST_EQ(idx.count("b"), 2u);
  BOOST_TEST_EQ(idx.count("ccc"), 2u);

  cout << "     string_view_multiset_test complete" << endl;
}

//-------------------------------  size_t_codec_test  ----------------------------------//

void  size_t_codec_test()
{
  cout << "  size_t_codec_test..." << endl;

  typedef btree::support::size_t_codec codec;

  cout << "    max_size is " << codec::max_size << endl;

  char buf[codec::max_size];

  cout << "    sizeof(buf) is " << sizeof(buf) << endl;

  std::pair<std::size_t, std::size_t> r_decode;  // value decoded, size of decoded value
  std::size_t encoded_sz;

  encoded_sz = codec::encoded_size(0);
  BOOST_TEST_EQ(encoded_sz, 1u);
  codec::encode(0, buf, encoded_sz);
  r_decode = codec::decode(buf);
  BOOST_TEST_EQ(r_decode.first, 0u);
  BOOST_TEST_EQ(r_decode.second, encoded_sz);

  encoded_sz = codec::encoded_size(1);
  BOOST_TEST_EQ(encoded_sz, 1u);
  codec::encode(1, buf, encoded_sz);
  r_decode = codec::decode(buf);
  BOOST_TEST_EQ(r_decode.first, 1u);
  BOOST_TEST_EQ(r_decode.second, encoded_sz);

  encoded_sz = codec::encoded_size(127);
  BOOST_TEST_EQ(encoded_sz, 1u);
  codec::encode(127, buf, encoded_sz);
  r_decode = codec::decode(buf);
  BOOST_TEST_EQ(r_decode.first, 127u);
  BOOST_TEST_EQ(r_decode.second, encoded_sz);

  encoded_sz = codec::encoded_size(128);
  BOOST_TEST_EQ(encoded_sz, 2u);
  codec::encode(128u, buf, encoded_sz);
  r_decode = codec::decode(buf);
  BOOST_TEST_EQ(r_decode.first, 128u);
  BOOST_TEST_EQ(r_decode.second, encoded_sz);

  encoded_sz = codec::encoded_size(16384u);
  BOOST_TEST_EQ(encoded_sz, 3u);
  codec::encode(16384u, buf, encoded_sz);
  r_decode = codec::decode(buf);
  BOOST_TEST_EQ(r_decode.first, 16384u);
  BOOST_TEST_EQ(r_decode.second, encoded_sz);

  for (std::size_t x = 0; x < 25000000u; ++x)
  {
    encoded_sz = codec::encoded_size(x);
    codec::encode(x, buf, encoded_sz);
    r_decode = codec::decode(buf);
    BOOST_TEST_EQ(r_decode.first, x);
    BOOST_TEST_EQ(r_decode.second, encoded_sz);
  }

  cout << "     size_t_codec_test complete" << endl;
}

//-----------------------------  heterogeneous_key_test  -------------------------------//
struct simple
{
  int x;
  int y;

  simple(){}
  simple(int x_, int y_) : x(x_), y(y_) {}

  bool operator<(const simple& rhs) const  {return x < rhs.x;}
};

bool operator<(const simple& lhs, int rhs) {return lhs.x < rhs;}
bool operator<(int lhs, const simple& rhs) {return lhs < rhs.x;}

void  heterogeneous_key_test()
{
  cout << "  heterogeneous_key_test..." << endl;

  typedef btree::btree_index_set<simple> set;
  set idx(idx1_path, file_path, btree::flags::truncate);

  idx.insert(simple(2, 0));
  idx.insert(simple(1, 0));
  idx.insert(simple(3, 0));

  BOOST_TEST_EQ(idx.size(), 3u);

  set::const_iterator result;
  simple key(2,0);
  result = idx.lower_bound(key);
  BOOST_TEST(result != idx.end());
  BOOST_TEST_EQ(result->x, 2);

  result = idx.lower_bound(2);
  BOOST_TEST(result != idx.end());
  BOOST_TEST_EQ(result->x, 2);

  BOOST_TEST(idx.find(key) == idx.find(2));
  BOOST_TEST(idx.count(key) == idx.count(2));
  BOOST_TEST(idx.lower_bound(key) == idx.lower_bound(2));
  BOOST_TEST(idx.upper_bound(key) == idx.upper_bound(2));
  set::const_iterator_range r1, r2;
  r1 = idx.equal_range(key);
  r2 = idx.equal_range(2);
  BOOST_TEST(r1.first == r2.first);
  BOOST_TEST(r1.second == r2.second);

  cout << "     heterogeneous_key_test complete" << endl;
}

//-------------------------------------  map_test  ----------------------------------------//

void  map_test()
{
  cout << "  map_test..." << endl;

  {
    cout << "      btree_index_map<int, long>..." << endl;
    typedef btree::btree_index_map<int, long> index_type;
    index_type idx 
      ("map_test.ndx", "map_test.dat", btree::flags::truncate, -1, btree::less(), 128);

    index_type::const_iterator find_itr;
    idx.insert(make_pair<int, long>(1,2));
    BOOST_TEST_EQ(idx.size(), 1u);
    find_itr = idx.find(1);
    BOOST_TEST(find_itr != idx.end());
    BOOST_TEST_EQ(find_itr->first, 1);
    BOOST_TEST_EQ(find_itr->second, 2);
    BOOST_TEST_EQ(idx.count(1), 1u);

    idx.insert(make_pair<int, long>(1,4));
    BOOST_TEST_EQ(idx.size(), 1u);
    BOOST_TEST_EQ(idx.count(1), 1u);
    idx.insert(make_pair<int, long>(0,5));
    BOOST_TEST_EQ(idx.size(), 2u);
    BOOST_TEST_EQ(idx.count(1), 1u);
    BOOST_TEST_EQ(idx.count(0), 1u);

    idx.emplace(2, 9);
    BOOST_TEST_EQ(idx.size(), 3u);
    find_itr = idx.find(2);
    BOOST_TEST(find_itr != idx.end());
    BOOST_TEST_EQ(find_itr->first, 2);
    BOOST_TEST_EQ(find_itr->second, 9);

    for (index_type::const_iterator itr = idx.begin();
      itr != idx.end(); ++itr)
      cout << itr->first << "," << itr->second << std::endl;
  }

  {
    cout << "      btree_index_multimap<int, long>..." << endl;
    typedef btree::btree_index_multimap<int, long> index_type;
    index_type idx 
      ("multimap_test.ndx", "multimap_test.dat",
       btree::flags::truncate, -1, btree::less(), 128);

    idx.insert(make_pair<int, long>(1,2));
    BOOST_TEST_EQ(idx.count(1), 1u);
    BOOST_TEST_EQ(idx.size(), 1u);
    idx.insert(make_pair<int, long>(1,4));
    BOOST_TEST_EQ(idx.size(), 2u);
    BOOST_TEST_EQ(idx.count(1), 2u);
    idx.insert(make_pair<int, long>(0,5));
    BOOST_TEST_EQ(idx.size(), 3u);
    BOOST_TEST_EQ(idx.count(1), 2u);
    BOOST_TEST_EQ(idx.count(0), 1u);

    idx.emplace(2, 9);
    BOOST_TEST_EQ(idx.size(), 4u);
    index_type::const_iterator find_itr = idx.find(2);
    BOOST_TEST(find_itr != idx.end());
    BOOST_TEST_EQ(find_itr->first, 2);
    BOOST_TEST_EQ(find_itr->second, 9);

    idx.emplace(1, 0);
    BOOST_TEST_EQ(idx.size(), 5u);
    BOOST_TEST_EQ(idx.count(1), 3u);

    for (index_type::const_iterator itr = idx.begin();
      itr != idx.end(); ++itr)
      cout << itr->first << "," << itr->second << std::endl;
  }

  {
    cout << "      btree_index_map<string_view, string_view>..." << endl;
    typedef btree::btree_index_map<string_view, string_view> index_type;
    index_type idx 
      ("map_test_sv.ndx", "map_test_sv.dat",
       btree::flags::truncate, -1, btree::less(), 128);

    idx.insert(make_pair(string_view("abc"), string_view("xyz")));
    BOOST_TEST_EQ(idx.size(), 1u);
    idx.insert(make_pair(string_view("abc"), string_view("12345")));
    BOOST_TEST_EQ(idx.size(), 1u);
    idx.insert(make_pair(string_view("ab"), string_view("1")));
    BOOST_TEST_EQ(idx.size(), 2u);
    BOOST_TEST_EQ(idx.size(), 2u);

    idx.emplace(string_view("wxyz"), string_view("zyxw"));
    BOOST_TEST_EQ(idx.size(), 3u);
    index_type::const_iterator find_itr = idx.find("wxyz");
    BOOST_TEST(find_itr != idx.end());
    BOOST_TEST_EQ(find_itr->first, "wxyz");
    BOOST_TEST_EQ(find_itr->second, "zyxw");

    for (index_type::const_iterator itr = idx.begin();
      itr != idx.end(); ++itr)
      cout << itr->first << "," << itr->second << std::endl;
  }

  {
    cout << "      btree_index_multimap<string_view, string_view>..." << endl;
    typedef btree::btree_index_multimap<string_view, string_view> index_type;
    index_type idx 
      ("multimap_test_sv.ndx", "multimap_test_sv.dat",
       btree::flags::truncate, -1, btree::less(), 128);

    idx.insert(make_pair(string_view("abc"), string_view("xyz")));
    BOOST_TEST_EQ(idx.size(), 1u);
    idx.insert(make_pair(string_view("abc"), string_view("12345")));
    BOOST_TEST_EQ(idx.size(), 2u);
    idx.insert(make_pair(string_view("ab"), string_view("1")));
    BOOST_TEST_EQ(idx.size(), 3u);

    idx.emplace(string_view("wxyz"), string_view("zyxw"));
    BOOST_TEST_EQ(idx.size(), 4u);
    index_type::const_iterator find_itr = idx.find("wxyz");
    BOOST_TEST(find_itr != idx.end());
    BOOST_TEST_EQ(find_itr->first, "wxyz");
    BOOST_TEST_EQ(find_itr->second, "zyxw");

    idx.emplace(string_view("wxyz"), string_view("foobar"));
    BOOST_TEST_EQ(idx.size(), 5u);
    BOOST_TEST_EQ(idx.count("wxyz"), 2u);

    for (index_type::const_iterator itr = idx.begin();
      itr != idx.end(); ++itr)
      cout << itr->first << "," << itr->second << std::endl;
  }

  cout << "     map_test complete" << endl;
}


//-------------------------------------  _test  ----------------------------------------//

void  _test()
{
  cout << "  _test..." << endl;

  cout << "     _test complete" << endl;
}

}  // unnamed namespace

//------------------------------------ cpp_main ----------------------------------------//

int cpp_main(int argc, char* argv[])
{
  std::string command_args;

  for (int a = 0; a < argc; ++a)
  {
    command_args += argv[a];
    if (a != argc-1)
      command_args += ' ';
  }

  cout << command_args << '\n';;

  for (; argc > 1; ++argv, --argc) 
  {
    if ( std::strcmp( argv[1]+1, "d" )==0 )
      dump_dot = true;
    else
    {
      cout << "Error - unknown option: " << argv[1] << "\n\n";
      argc = -1;
      break;
    }
  }

  if (argc < 1) 
  {
    cout << "Usage: index_unit_test [Options]\n"
//      " The argument n specifies the number of test cases to run\n"
      " Options:\n"
//      "   path     Specifies the test file path; default test.btree\n"
      "   -d       Dump tree using Graphviz dot format; default is no dump\n"
      ;
    return 1;
  }

  instantiate_test();
  open_all_new_test();
  //open_new_index_test();
  push_back_insert_pos_test();
  iterator_test();
  lower_bound_test();
  find_test();
  insert_test();
  heterogeneous_key_test();
  two_index_test();
  two_index_iterator_test();
  //c_string_test();
  map_test();
  size_t_codec_test();  // do this before string_view, as string_view depends on it
  string_view_test();
  string_view_volume_test();
  string_view_multiset_test();
  cout << "all tests complete" << endl;

  return boost::report_errors();
}


