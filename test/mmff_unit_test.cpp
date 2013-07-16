//  mmff_unit_test.cpp  ----------------------------------------------------------------//

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//--------------------------------------------------------------------------------------//
//                                                                                      //
//            memory-mapped flat-file unit test; mmff_unit_test for short               //
//                                                                                      //
//--------------------------------------------------------------------------------------//

#define _SCL_SECURE_NO_WARNINGS  // turn off Microsoft foolishness; their own library
                                 // generates warnings otherwise

#include <boost/btree/mmff.hpp>
#include <boost/detail/lightweight_main.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <cstring>

#include <iostream>

using std::cout; using std::endl;
using namespace boost::btree;

namespace
{
  boost::filesystem::path p("mmff_test.flat");

//-------------------------------  default_construct_test  -----------------------------//

void  default_construct_test()
{
  cout << "  default_construct_test..." << endl;

   extendible_mapped_file  xmf;

   BOOST_TEST(!xmf.is_open());

  cout << "     default_construct_test complete" << endl;
}

//-----------------------------------  open_test  --------------------------------------//

void  open_test()
{
  cout << "  open_test..." << endl;

  {
    extendible_mapped_file  xmf;
    xmf.open(p, flags::truncate, 100);

    BOOST_TEST(xmf.is_open());
    BOOST_TEST_EQ(xmf.reopen_flags(), flags::read_write);
    BOOST_TEST_EQ(xmf.mode(), boost::iostreams::mapped_file::readwrite);
    BOOST_TEST_EQ(xmf.reserve(), 100);
    BOOST_TEST_EQ(xmf.file_size(), 0);
    BOOST_TEST_EQ(xmf.mapped_size(), 100);
    BOOST_TEST_EQ(xmf.path(), p);
    BOOST_TEST(xmf.data<char>());
    BOOST_TEST(xmf.const_data<char>() == xmf.data<char>());
    BOOST_TEST_EQ(boost::filesystem::file_size(p), 100);
  }
  BOOST_TEST_EQ(boost::filesystem::file_size(p), 0);

  cout << "     open_test complete" << endl;
}

//--------------------------------  ctor_open_test  ------------------------------------//

void  ctor_open_test()
{
  cout << "  ctor_open_test..." << endl;

  {
    extendible_mapped_file  xmf(p, flags::truncate, 100);

    BOOST_TEST(xmf.is_open());
    BOOST_TEST_EQ(xmf.reopen_flags(), flags::read_write);
    BOOST_TEST_EQ(xmf.mode(), boost::iostreams::mapped_file::readwrite);
    BOOST_TEST_EQ(xmf.reserve(), 100);
    BOOST_TEST_EQ(xmf.file_size(), 0);
    BOOST_TEST_EQ(xmf.mapped_size(), 100);
    BOOST_TEST_EQ(xmf.path(), p);
    BOOST_TEST(xmf.data<char>());
    BOOST_TEST(xmf.const_data<char>() == xmf.data<char>());
    BOOST_TEST_EQ(boost::filesystem::file_size(p), 100);
  }
  BOOST_TEST_EQ(boost::filesystem::file_size(p), 0);

  cout << "     ctor_open_test complete" << endl;
}

//--------------------------------  create_test  ---------------------------------------//

void  create_test()
{
  cout << "  create_test..." << endl;

  {
    extendible_mapped_file  xmf(p, flags::truncate, 100);

    BOOST_TEST(xmf.is_open());
    BOOST_TEST_EQ(xmf.reopen_flags(), flags::read_write);
    BOOST_TEST_EQ(xmf.mode(), boost::iostreams::mapped_file::readwrite);
    BOOST_TEST_EQ(xmf.reserve(), 100);
    BOOST_TEST_EQ(xmf.file_size(), 0);
    BOOST_TEST_EQ(xmf.mapped_size(), 100);
    BOOST_TEST_EQ(xmf.path(), p);
    BOOST_TEST(xmf.data<char>());
    BOOST_TEST(xmf.const_data<char>() == xmf.data<char>());
    BOOST_TEST_EQ(boost::filesystem::file_size(p), 100);

    const char* dat = "1234567890";
    xmf.increment_file_size(10);
    std::memcpy(xmf.data<char>(), dat, 10);
    BOOST_TEST_EQ(xmf.file_size(), 10);
    BOOST_TEST_EQ(xmf.reserve(), 100);      // should not have changed
    BOOST_TEST_EQ(xmf.mapped_size(), 100);  // ditto

    const char* dat2 = "abcdef";
    xmf.increment_file_size(6);
    std::memcpy(xmf.data<char>()+10, dat2, 6);
    BOOST_TEST_EQ(xmf.file_size(), 16);
    BOOST_TEST_EQ(xmf.reserve(), 100);      // should not have changed
    BOOST_TEST_EQ(xmf.mapped_size(), 100);  // ditto
  }
  BOOST_TEST_EQ(boost::filesystem::file_size(p), 16);

  cout << "     create_test complete" << endl;
}

//--------------------------------  open_existing_test  --------------------------------//

void  open_existing_test()
{
  cout << "  open_existing_test..." << endl;

  {
    extendible_mapped_file  xmf(p, flags::read_only, 20);

    BOOST_TEST(xmf.is_open());
    BOOST_TEST_EQ(xmf.reopen_flags(), flags::read_only);
    BOOST_TEST_EQ(xmf.mode(), boost::iostreams::mapped_file::readonly);
    BOOST_TEST_EQ(xmf.reserve(), 20);
    BOOST_TEST_EQ(xmf.file_size(), 16);
    BOOST_TEST_EQ(xmf.mapped_size(), xmf.file_size() + xmf.reserve());
    BOOST_TEST_EQ(xmf.path(), p);
    BOOST_TEST(!xmf.data<char>());
    BOOST_TEST(xmf.const_data<char>());
    BOOST_TEST_EQ(boost::filesystem::file_size(p), xmf.file_size() + xmf.reserve());
    BOOST_TEST(std::memcmp(xmf.const_data<char>(), "1234567890abcdef", 16) == 0);
  }
  BOOST_TEST_EQ(boost::filesystem::file_size(p), 16);

  cout << "     open_existing_test complete" << endl;
}

//----------------------  extend_existing_across_reserve_test  -------------------------//

void  extend_existing_across_reserve_test()
{
  cout << "  extend_existing_across_reserve_test..." << endl;

  {
    extendible_mapped_file  xmf(p, flags::read_write, 4);

    BOOST_TEST(xmf.is_open());
    BOOST_TEST_EQ(xmf.reopen_flags(), flags::read_write);
    BOOST_TEST_EQ(xmf.mode(), boost::iostreams::mapped_file::readwrite);
    BOOST_TEST_EQ(xmf.reserve(), 4);
    BOOST_TEST_EQ(xmf.file_size(), 16);
    BOOST_TEST_EQ(xmf.mapped_size(), xmf.file_size() + xmf.reserve());
    BOOST_TEST_EQ(xmf.path(), p);
    BOOST_TEST(xmf.data<char>());
    BOOST_TEST(xmf.const_data<char>() == xmf.data<char>());
    BOOST_TEST_EQ(boost::filesystem::file_size(p), xmf.file_size() + xmf.reserve());
    xmf.increment_file_size(5);
    BOOST_TEST_EQ(xmf.file_size(), 21);
    std::memcpy(xmf.data<char>() + 16, "vwxyz", 5);
  }
  BOOST_TEST_EQ(boost::filesystem::file_size(p), 21);

  cout << "     extend_existing_across_reserve_test complete" << endl;
}

//--------------------------------  push_back_test  ------------------------------------//

void  push_back_test()
{
  cout << "  push_back_test, with reserve of exactly the size pushed..." << endl;

  {
    extendible_mapped_file  xmf(p, flags::read_write, 6); 

    BOOST_TEST(xmf.is_open());
    BOOST_TEST_EQ(xmf.reopen_flags(), flags::read_write);
    BOOST_TEST_EQ(xmf.mode(), boost::iostreams::mapped_file::readwrite);
    BOOST_TEST_EQ(xmf.reserve(), 6);
    BOOST_TEST_EQ(xmf.file_size(), 21);
    BOOST_TEST_EQ(xmf.mapped_size(), xmf.file_size() + xmf.reserve());
    BOOST_TEST_EQ(xmf.path(), p);
    BOOST_TEST(xmf.data<char>());
    BOOST_TEST(xmf.const_data<char>() == xmf.data<char>());
    BOOST_TEST_EQ(boost::filesystem::file_size(p), xmf.file_size() + xmf.reserve());
    const char bingo[] = "bingo!";
    BOOST_TEST_EQ((xmf.push_back(bingo[0], 6u)), 21U);
    BOOST_TEST_EQ(xmf.file_size(), 27);
  }
  BOOST_TEST_EQ(boost::filesystem::file_size(p), 27);

  cout << "     push_back_test complete" << endl;
}

//--------------------------------  final_value_test  ----------------------------------//

void  final_value_test(const std::string& expected_value)
{
  cout << "  final_value_test..." << endl;

  extendible_mapped_file  xmf(p, flags::read_only);
  std::string actual_value(xmf.const_data<char>(), xmf.file_size());
  BOOST_TEST_EQ(actual_value, expected_value);

  cout << "     final_value_test complete" << endl;
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

  cout << command_args << '\n' << endl;
  cout << "begin testing" << endl;

  default_construct_test();
  open_test();
  ctor_open_test();
  create_test();
  open_existing_test();
  extend_existing_across_reserve_test();
  push_back_test();

  // must be last test, expected value must be adjusted if file contents changed
  final_value_test("1234567890abcdefvwxyzbingo!");

  cout << "all tests complete" << endl;

  return boost::report_errors();
}
