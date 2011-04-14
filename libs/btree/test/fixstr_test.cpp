//  fixstr_test.cpp  -------------------------------------------------------------------//

//  Copyright Beman Dawes 2010

//  Distributed under the Boost Software License, Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt

#include <boost/config/warning_disable.hpp>

#include <boost/btree/support/fixstr.hpp>
#include <boost/detail/lightweight_main.hpp>
#include <boost/detail/lightweight_test.hpp> 

#include <iostream>

namespace
{
  void default_construct()
  {
    boost::btree::fixstr<5> x;
    
    BOOST_TEST_EQ(x.size(), 0U);
    BOOST_TEST_EQ(x.max_size(), 5U);
    BOOST_TEST(x.empty());
    BOOST_TEST_EQ(std::strlen(x.c_str()), 0U);
    BOOST_TEST_EQ(std::memcmp(x.c_str(),"\0\0\0\0\0\0", 6), 0 );
  }

  void construct_from_c_str()
  {
    boost::btree::fixstr<5> x0("");
    BOOST_TEST_EQ(x0.size(), 0U);
    BOOST_TEST(x0.empty());
    BOOST_TEST_EQ(std::strlen(x0.c_str()), 0U);
    BOOST_TEST_EQ(std::memcmp(x0.c_str(),"\0\0\0\0\0\0", 6), 0 );
    
    boost::btree::fixstr<5> x3("abc");
    BOOST_TEST_EQ(x3.size(), 3U);
    BOOST_TEST(!x3.empty());
    BOOST_TEST_EQ(std::strlen(x3.c_str()), 3U);
    BOOST_TEST_EQ(std::memcmp(x3.c_str(),"abc\0\0\0", 6), 0 );
    
    boost::btree::fixstr<5> x5("abcdef");
    BOOST_TEST_EQ(x5.size(), 5U);
    BOOST_TEST(!x5.empty());
    BOOST_TEST_EQ(std::strlen(x5.c_str()), 5U);
    BOOST_TEST_EQ(std::memcmp(x5.c_str(),"abcde\0", 6), 0 );
  }

  //  size(), empty(), and c_str() have now been well tested, so test contents only
  //  from here on.

  void construct_from_string()
  {
    boost::btree::fixstr<5> x0(std::string(""));
    BOOST_TEST_EQ(std::memcmp(x0.c_str(),"\0\0\0\0\0\0", 6), 0 );
    
    boost::btree::fixstr<5> x3(std::string("abc"));
    BOOST_TEST_EQ(std::memcmp(x3.c_str(),"abc\0\0\0", 6), 0 );
    
    boost::btree::fixstr<5> x5(std::string("abcdef"));
    BOOST_TEST_EQ(std::memcmp(x5.c_str(),"abcde\0", 6), 0 );
  }

  void copy_construct()
  {
    boost::btree::fixstr<5> y0("");
    boost::btree::fixstr<5> x0(y0);
    BOOST_TEST_EQ(std::memcmp(x0.c_str(),"\0\0\0\0\0\0", 6), 0 );
    
    boost::btree::fixstr<5> y3("abc");
    boost::btree::fixstr<5> x3(y3);
    BOOST_TEST_EQ(std::memcmp(x3.c_str(),"abc\0\0\0", 6), 0 );
    
    boost::btree::fixstr<5> y5("abcdef");
    boost::btree::fixstr<5> x5(y5);
    BOOST_TEST_EQ(std::memcmp(x5.c_str(),"abcde\0", 6), 0 );
  }

  void copy_assign()
  {
    boost::btree::fixstr<5> y0("");
    boost::btree::fixstr<5> x0 = y0;
    BOOST_TEST_EQ(std::memcmp(x0.c_str(),"\0\0\0\0\0\0", 6), 0 );
    
    boost::btree::fixstr<5> y3("abc");
    boost::btree::fixstr<5> x3 = y3;
    BOOST_TEST_EQ(std::memcmp(x3.c_str(),"abc\0\0\0", 6), 0 );
    
    boost::btree::fixstr<5> y5("abcdef");
    boost::btree::fixstr<5> x5 = y5;
    BOOST_TEST_EQ(std::memcmp(x5.c_str(),"abcde\0", 6), 0 );
  }

  void copy_from_c_str()
  {
    boost::btree::fixstr<5> x0("xxx");
    x0 = "";
    BOOST_TEST_EQ(std::memcmp(x0.c_str(),"\0\0\0\0\0\0", 6), 0 );
    
    boost::btree::fixstr<5> x3("xxxxx");
    x3 = "abc";
    BOOST_TEST_EQ(std::memcmp(x3.c_str(),"abc\0\0\0", 6), 0 );
    
    boost::btree::fixstr<5> x5("xxx");
    x5 = "abcdef";
    BOOST_TEST_EQ(std::memcmp(x5.c_str(),"abcde\0", 6), 0 );
  }

  void copy_from_string()
  {
    boost::btree::fixstr<5> x0("xxx");
    x0 = std::string("");
    BOOST_TEST_EQ(std::memcmp(x0.c_str(),"\0\0\0\0\0\0", 6), 0 );
    
    boost::btree::fixstr<5> x3("xxxxx");
    x3 = std::string("abc");
    BOOST_TEST_EQ(std::memcmp(x3.c_str(),"abc\0\0\0", 6), 0 );
    
    boost::btree::fixstr<5> x5("xxx");
    x5 = std::string("abcdef");
    BOOST_TEST_EQ(std::memcmp(x5.c_str(),"abcde\0", 6), 0 );
  }

  void append_from_c_str()
  {
    boost::btree::fixstr<5> x0("ab");
    x0 += "";
    BOOST_TEST_EQ(std::memcmp(x0.c_str(),"ab\0\0\0\0", 6), 0 );
    
    boost::btree::fixstr<5> x3("ab");
    x3 += "c";
    BOOST_TEST_EQ(std::memcmp(x3.c_str(),"abc\0\0\0", 6), 0 );
    
    boost::btree::fixstr<5> x5("abc");
    x5 += "def";
    BOOST_TEST_EQ(std::memcmp(x5.c_str(),"abcde\0", 6), 0 );
  }

  void append_from_string()
  {
    boost::btree::fixstr<5> x0("ab");
    x0 += std::string("");
    BOOST_TEST_EQ(std::memcmp(x0.c_str(),"ab\0\0\0\0", 6), 0 );
    
    boost::btree::fixstr<5> x3("ab");
    x3 += std::string("c");
    BOOST_TEST_EQ(std::memcmp(x3.c_str(),"abc\0\0\0", 6), 0 );
    
    boost::btree::fixstr<5> x5("abc");
    x5 += std::string("def");
    BOOST_TEST_EQ(std::memcmp(x5.c_str(),"abcde\0", 6), 0 );
  }

  void clear()
  {
    boost::btree::fixstr<5> x0("");
    x0.clear();
    BOOST_TEST_EQ(std::memcmp(x0.c_str(),"\0\0\0\0\0\0", 6), 0 );
    
    boost::btree::fixstr<5> x3("abc");
    x3.clear();
    BOOST_TEST_EQ(std::memcmp(x3.c_str(),"\0\0\0\0\0\0", 6), 0 );
    
    boost::btree::fixstr<5> x5("abcdef");
    x5.clear();
    BOOST_TEST_EQ(std::memcmp(x5.c_str(),"\0\0\0\0\0\0", 6), 0 );
  }

  void operator_square_brackets_const()
  {
    const boost::btree::fixstr<5> x0("");
    BOOST_TEST_EQ(x0[0], '\0');
    
    const boost::btree::fixstr<5> x3("abc");
    BOOST_TEST_EQ(x3[0], 'a');
    BOOST_TEST_EQ(x3[1], 'b');
    BOOST_TEST_EQ(x3[2], 'c');
    BOOST_TEST_EQ(x3[3], '\0');
  }

  void operator_square_brackets()
  {
    const boost::btree::fixstr<5> x0("");
    BOOST_TEST_EQ(x0[0], '\0');
    
    boost::btree::fixstr<5> x3("abc");
    x3[0] = 'x';
    x3[1] = 'y';
    x3[2] = 'z';
    BOOST_TEST_EQ(x3[0], 'x');
    BOOST_TEST_EQ(x3[1], 'y');
    BOOST_TEST_EQ(x3[2], 'z');
    BOOST_TEST_EQ(x3[3], '\0');
  }

  void string()
  {
    boost::btree::fixstr<5> x0("");
    BOOST_TEST_EQ(x0.string(), std::string(""));
    
    boost::btree::fixstr<5> x3("abc");
    BOOST_TEST_EQ(x3.string(), std::string("abc"));
    
    boost::btree::fixstr<5> x5("abcdef");
    BOOST_TEST_EQ(x5.string(), std::string("abcde"));
  }

  void relationals()
  {
    boost::btree::fixstr<5> nul("");
    boost::btree::fixstr<5> a("a");
    boost::btree::fixstr<5> aa("aa");
    boost::btree::fixstr<5> b("b");

    BOOST_TEST(nul == nul);
    BOOST_TEST(!(nul != nul));
    BOOST_TEST(!(nul < nul));
    BOOST_TEST(nul <= nul);
    BOOST_TEST(!(nul > nul));
    BOOST_TEST(nul >= nul);

    BOOST_TEST(!(a == nul));
    BOOST_TEST(a != nul);
    BOOST_TEST(!(a < nul));
    BOOST_TEST(!(a <= nul));
    BOOST_TEST(a > nul);
    BOOST_TEST(a >= nul);

    BOOST_TEST(!(nul == a));
    BOOST_TEST(nul != a);
    BOOST_TEST(nul < a);
    BOOST_TEST(nul <= a);
    BOOST_TEST(!(nul > a));
    BOOST_TEST(!(nul >= a));

    BOOST_TEST(a == a);
    BOOST_TEST(!(a != a));
    BOOST_TEST(!(a < a));
    BOOST_TEST(a <= a);
    BOOST_TEST(!(a > a));
    BOOST_TEST(a >= a);

    BOOST_TEST(!(a == aa));
    BOOST_TEST(a != aa);
    BOOST_TEST(a < aa);
    BOOST_TEST(a <= aa);
    BOOST_TEST(!(a > aa));
    BOOST_TEST(!(a >= aa));

    BOOST_TEST(!(aa == a));
    BOOST_TEST(aa != a);
    BOOST_TEST(!(aa < a));
    BOOST_TEST(!(aa <= a));
    BOOST_TEST(aa > a);
    BOOST_TEST(aa >= a);

    BOOST_TEST(!(a == b));
    BOOST_TEST(a != b);
    BOOST_TEST(a < b);
    BOOST_TEST(a <= b);
    BOOST_TEST(!(a > b));
    BOOST_TEST(!(a >= b));

    BOOST_TEST(!(b == a));
    BOOST_TEST(b != a);
    BOOST_TEST(!(b < a));
    BOOST_TEST(!(b <= a));
    BOOST_TEST(b > a);
    BOOST_TEST(b >= a);
  }
}

int cpp_main(int, char *[])
{
  default_construct();
  construct_from_c_str();
  construct_from_string();
  copy_construct();
  copy_assign();
  copy_from_c_str();
  copy_from_string();
  append_from_c_str();
  append_from_string();
  clear();
  operator_square_brackets_const();
  operator_square_brackets();
  string();
  relationals();

  return ::boost::report_errors();
}