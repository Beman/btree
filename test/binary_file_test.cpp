//  Boost binary_file_test.cpp  --------------------------------------------------------//

//  Copyright Beman Dawes 2006, 2010

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  See library home page http://www.boost.org/libs/filesystem

//--------------------------------------------------------------------------------------//

#include <boost/config/warning_disable.hpp>

#include <iostream>
#include <boost/btree/detail/binary_file.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/detail/lightweight_main.hpp>
#include <boost/detail/lightweight_test.hpp> 

namespace fs = boost::filesystem;
namespace bt = boost::btree;

#include <iomanip>
#include <string>
#include <cstring>

#ifdef BOOST_WINDOWS_API
# include <winerror.h>
# define BOOST_BAD_SEEK ERROR_NEGATIVE_SEEK
#else
# include <errno.h>
# define BOOST_BAD_SEEK EINVAL
#endif

namespace
{
  void open_flag_tests()
  {
    std::cout << "open flag tests..." << std::endl;

    fs::path p("test.txt");
    fs::remove(p);
    BOOST_TEST(!fs::exists(p));

    boost::system::error_code ec;

    {
      std::cout << "  oflag::in test, file doesn't exist..." << std::endl;   
      bt::binary_file f(p, bt::oflag::in, ec);
      BOOST_TEST(ec);
    }
    {
      std::cout << "  oflag::out test, file doesn't exist..." << std::endl;   
      bt::binary_file f(p, bt::oflag::out, ec);
      BOOST_TEST(!ec);
      BOOST_TEST(fs::exists(p));
      BOOST_TEST_EQ(fs::file_size(p), 0U);
      f.write("foo", 3, ec);
      BOOST_TEST(!ec);
      BOOST_TEST_EQ(fs::file_size(p), 3U);
    }
    {
      std::cout << "  oflag::in test, file exists..." << std::endl;   
      BOOST_TEST(fs::exists(p));
      bt::binary_file f(p, bt::oflag::in);
      std::cout << "    handle() returns " << f.handle() << std::endl;
      BOOST_TEST_EQ(fs::file_size(p), 3U);
    }
    {
      std::cout << "  oflag::in test | bt::oflag::out, file exists..." << std::endl;   
      BOOST_TEST(fs::exists(p));
      bt::binary_file f(p, bt::oflag::in | bt::oflag::out);
      BOOST_TEST_EQ(fs::file_size(p), 3U);
    }
    {
      std::cout << "  oflag::in test bt::oflag::in | bt::oflag::out | bt::oflag::truncate\n"
      ", file exists..." << std::endl;   
      BOOST_TEST(fs::exists(p));
      bt::binary_file f(p, bt::oflag::in | bt::oflag::out | bt::oflag::truncate);
      BOOST_TEST_EQ(fs::file_size(p), 0U);
    }

    fs::remove(p);
    std::cout << "  completed open flag tests" << std::endl;
  }
}

//  cpp_main  --------------------------------------------------------------------------//

int cpp_main(int argc, char * argv[])
{
  bt::binary_file::offset_type gap(32);

  if (argc > 1) gap =
    static_cast<bt::binary_file::offset_type>(std::atol(argv[1])) * 1024;

  open_flag_tests();

  char buf[128] = "0123456789abcdef";

  std::string filename("file_with_gap");
  bt::binary_file f(filename, bt::oflag::in |bt::oflag::out | bt::oflag::truncate);
  BOOST_TEST(f.path() == filename);
  BOOST_TEST(fs::exists(filename));


  boost::system::error_code ec;
  f.seek(-1, bt::seekdir::begin, ec);
  BOOST_TEST(ec);
  BOOST_TEST_EQ(ec.value(), BOOST_BAD_SEEK);

  bool error_thrown(false);
  try
  {
    f.seek(-1, bt::seekdir::begin);
  }
  catch (const fs::filesystem_error & ex)
  {
    error_thrown = true;
    BOOST_TEST_EQ(ec.value(), BOOST_BAD_SEEK);
    BOOST_TEST_EQ(ex.path1().string(), filename);
  }
  BOOST_TEST(error_thrown);

  char beginning[] = "beginning";
  f.write(beginning, 10);
  BOOST_TEST(fs::file_size(filename) == 10);
  BOOST_TEST(f.seek(0, bt::seekdir::end) == 10);
  BOOST_TEST(f.seek(gap, bt::seekdir::current) == gap + 10);

  char ending[] = "ending";
  f.write(ending, 7);
  BOOST_TEST(f.seek(0, bt::seekdir::current) == gap + 17);
  BOOST_TEST(f.seek(0, bt::seekdir::end) == gap + 17);

  int i = 12345;
  f.write(i);
  BOOST_TEST(f.seek(0, bt::seekdir::current) == gap + 17 + int(sizeof(i)));
  BOOST_TEST(f.seek(0, bt::seekdir::end) == gap + 17 + int(sizeof(i)));
  
  BOOST_TEST(f.seek(0, bt::seekdir::begin) == 0);
  
  BOOST_TEST(f.read(buf, 10));
  BOOST_TEST(std::strcmp(buf, beginning) == 0);
  BOOST_TEST(std::memcmp(&buf[10], "abcdef", 6) == 0);

  BOOST_TEST(f.seek(gap + 10, bt::seekdir::begin) == gap + 10);
  BOOST_TEST(f.read(buf, 7));
  BOOST_TEST(std::strcmp(buf, ending) == 0);

  int j;
  f.read(j);
  BOOST_TEST_EQ(i, j);

  BOOST_TEST(!f.read(buf, 1));

  BOOST_TEST(f.is_open());
  f.close();
  BOOST_TEST(!f.is_open());

  BOOST_TEST_EQ(fs::file_size(filename),
    static_cast<boost::uintmax_t>(gap + 17 + sizeof(i)));

  return boost::report_errors();
}
