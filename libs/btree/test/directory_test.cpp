 //  btree_directory_test.cpp  ---------------------------------------------------------//

//  Copyright Beman Dawes 2010

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  See library home page at http://www.boost.org/libs/btree

//--------------------------------------------------------------------------------------//


//--------------------------------------------------------------------------------------// 

#include <boost/btree/directory.hpp>
#include <boost/btree/set.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <boost/detail/main.hpp>
#include <boost/filesystem/operations.hpp>
namespace fs = boost::filesystem;
#include <utility>
#include <iostream>
#include <iomanip>

using std::cout;
using std::endl;

namespace
{
  fs::path test_dir(fs::current_path() / "temp_btree_test_dir");

  boost::btree::directory root;

  typedef boost::btree::btree_set<int> data_btree;
  data_btree data_1_1;
  data_btree data_1_2;
}

//  open_and_close_level_1_data  -------------------------------------------------------//

void open_and_close_level_1_data()
{
  cout << "open_and_close_level_1_data..." << endl;

  root.open(test_dir / "test_btree_directory.btr", boost::btree::flags::truncate, 128);
  BOOST_TEST(root.is_open());
  data_1_1.open(root, "data_1_1", boost::btree::flags::truncate);
  BOOST_TEST(data_1_1.is_open());
  data_1_2.open(root, "data_1_2", boost::btree::flags::truncate);
  BOOST_TEST(data_1_2.is_open());

  data_1_1.close();
  BOOST_TEST(root.is_open());
  BOOST_TEST(!data_1_1.is_open());
  BOOST_TEST(data_1_2.is_open());

  root.close();
  BOOST_TEST(!root.is_open());
  BOOST_TEST(!data_1_1.is_open());
  BOOST_TEST(!data_1_2.is_open());

  cout << "  open_and_close_level_1_data complete" << endl;
}

int cpp_main(int, char *[])
{

  open_and_close_level_1_data();

  return ::boost::report_errors();
}
