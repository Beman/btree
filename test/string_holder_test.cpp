//  string_holder_test.cpp  ------------------------------------------------------------//

//  Copyright Beman Dawes 2010, 2013

//  Distributed under the Boost Software License, Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt

#include <boost/config/warning_disable.hpp>

#include <boost/btree/support/string_holder.hpp>
#include <boost/detail/lightweight_main.hpp>
#include <boost/detail/lightweight_test.hpp> 

#include <iostream>

namespace
{
  void default_construct()
  {
    boost::btree::string_holder<5> x;
    
    BOOST_TEST_EQ(x.size(), 0U);
    BOOST_TEST_EQ(x.max_size(), 5U);
    BOOST_TEST(x.empty());
  }

  void construct_from_c_str()
  {
    boost::btree::string_holder<5> x0("");
    BOOST_TEST_EQ(x0.size(), 0U);
    BOOST_TEST(x0.empty());
    
    boost::btree::string_holder<5> x3("abc");
    BOOST_TEST_EQ(x3.size(), 3U);
    BOOST_TEST(!x3.empty());
    BOOST_TEST(x3 == "abc");
    
    boost::btree::string_holder<5> x5("abcdef");
    BOOST_TEST_EQ(x5.size(), 5U);
    BOOST_TEST(!x5.empty());
    BOOST_TEST(x5 == "abcde"); 
  }

  //  size(), empty(), and c_str() have now been well tested, so test contents only
  //  from here on.

  void construct_from_string()
  {
    boost::btree::string_holder<5> x0(std::string(""));
    BOOST_TEST_EQ(x0.size(), 0U);
    BOOST_TEST(x0.empty());
    
    boost::btree::string_holder<5> x3(std::string("abc"));
    BOOST_TEST(x3 == "abc"); 
    
    boost::btree::string_holder<5> x5(std::string("abcdef"));
    BOOST_TEST(x5 == "abcde"); 
  }

  void copy_construct()
  {
    boost::btree::string_holder<5> y0("");
    boost::btree::string_holder<5> x0(y0);
    BOOST_TEST(x0 == ""); 
    
    boost::btree::string_holder<5> y3("abc");
    boost::btree::string_holder<5> x3(y3);
    BOOST_TEST(x3 == "abc"); 
    
    boost::btree::string_holder<5> y5("abcdef");
    boost::btree::string_holder<5> x5(y5);
    BOOST_TEST(x5 == "abcde"); 
  }

  void copy_assign()
  {
    boost::btree::string_holder<5> y0("");
    boost::btree::string_holder<5> x0 = y0;
    BOOST_TEST(x0 == ""); 
    
    boost::btree::string_holder<5> y3("abc");
    boost::btree::string_holder<5> x3 = y3;
    BOOST_TEST(x3 == "abc"); 
    
    boost::btree::string_holder<5> y5("abcdef");
    boost::btree::string_holder<5> x5 = y5;
    BOOST_TEST(x5 == "abcde"); 
  }

  void copy_from_c_str()
  {
    boost::btree::string_holder<5> x0("xxx");
    x0 = "";
    BOOST_TEST(x0 == ""); 
    
    boost::btree::string_holder<5> x3("xxxxx");
    x3 = "abc";
    BOOST_TEST(x3 == "abc"); 
    
    boost::btree::string_holder<5> x5("xxx");
    x5 = "abcdef";
    BOOST_TEST(x5 == "abcde"); 
  }

  void copy_from_string()
  {
    boost::btree::string_holder<5> x0("xxx");
    x0 = std::string("");
    BOOST_TEST(x0 == ""); 
    
    boost::btree::string_holder<5> x3("xxxxx");
    x3 = std::string("abc");
    BOOST_TEST(x3 == "abc"); 
    
    boost::btree::string_holder<5> x5("xxx");
    x5 = std::string("abcdef");
    BOOST_TEST(x5 == "abcde"); 
  }

  void clear()
  {
    boost::btree::string_holder<5> x0("");
    x0.clear();
    BOOST_TEST(x0 == ""); 
    
    boost::btree::string_holder<5> x3("abc");
    x3.clear();
    BOOST_TEST(x3 == ""); 
    
    boost::btree::string_holder<5> x5("abcdef");
    x5.clear();
    BOOST_TEST(x5 == ""); 
  }

  void operator_square_brackets_const()
  {
    const boost::btree::string_holder<5> x3("abc");
    BOOST_TEST_EQ(x3[0], 'a');
    BOOST_TEST_EQ(x3[1], 'b');
    BOOST_TEST_EQ(x3[2], 'c');
  }

  void several_flavors_of_find()
  {
    typedef boost::btree::string_holder<20> T;
    const T x("abcdefbca");
    BOOST_TEST(x.find("xyz") == T::npos);
    BOOST_TEST(x.find("def") == 3);
    BOOST_TEST(x.find_first_of("xyz") == T::npos);
    BOOST_TEST(x.find_first_of("xbz") == 1);
    BOOST_TEST(x.find_first_not_of("fedcba") == T::npos);
    BOOST_TEST(x.find_first_not_of("fdcba") == 4);
    BOOST_TEST(x.find_last_of("xyz") == T::npos);
    BOOST_TEST(x.find_last_of("xbz") == 6);
  }

  void string()
  {
    boost::btree::string_holder<5> x0("");
    BOOST_TEST(x0 == std::string(""));
    
    boost::btree::string_holder<5> x3("abc");
    BOOST_TEST(x3 == std::string("abc"));
    
    boost::btree::string_holder<5> x5("abcdef");
    BOOST_TEST(x5 == std::string("abcde"));
  }

  void relationals()
  {
    boost::btree::string_holder<5> nul("");
    boost::btree::string_holder<5> a("a");
    boost::btree::string_holder<5> aa("aa");
    boost::btree::string_holder<5> b("b");

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

void inserter_test()
{
   boost::btree::string_holder<10> x("Bingo!");
   std::cout << '"' << x << '"' << std::endl;
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
  clear();
  operator_square_brackets_const();
  several_flavors_of_find();
  string();
  relationals();

  inserter_test();

  return ::boost::report_errors();
}
