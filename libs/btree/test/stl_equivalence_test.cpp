//  stl_equivalence_test.hpp  ----------------------------------------------------------//

//  Copyright Beman Dawes 2010

//  Distributed under the Boost Software License, Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt

//  See http://www.boost.org/libs/btree for documentation.

#include <boost/btree/map.hpp>
//#include <boost/btree/set.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <boost/random.hpp>
#include <map>
#include <iostream>
#include <cstdlib>  // for atol()

using namespace boost;
using std::cout;
using std::endl;

namespace
{
  long n = 10000;  // number to test cases to run

  rand48  rng;
  uniform_int<long> n_dist(0, n-1);
  variate_generator<rand48&, uniform_int<long> > key(rng, n_dist);

}  // unnamed namespace

int main(int argc, char *argv[])
{
  if (argc > 1)
  {
    n = std::atol(argv[1]);
  }

  typedef btree::btree_map<long, long> bt_type;
  typedef std::map<long, long>  stl_type;
  bt_type bt("stl_equivalent.btree", btree::flags::truncate,
    128);  // small page size to incease stress
  bt.max_cache_pages(2);  // small cache to incease stress
  stl_type stl;

  // insert test

  cout << "inserting " << n << " elements..." << endl;
  rng.seed(1);
  for (long i = 0; i < n; ++i)
  {
    stl_type::value_type element(key(), i);
//    cout << element.first << ',';
    bt.insert(element);
    stl.insert(element);
    BOOST_TEST_EQ(bt.size(), stl.size());
    if (bt.size() != stl.size())
      return report_errors();
  }
  cout << "  insertion complete" << endl;

  // find test

  cout << "finding " << n << " elements..." << endl;
  rng.seed(1);
  for (long i = 0; i < n; ++i)
  {
//    cout << i << ',';
    long k = key();
    stl_type::const_iterator stl_itr = stl.find(k);
    BOOST_TEST(stl_itr != stl.end());
    if (stl_itr == stl.end()) return report_errors();
    BOOST_TEST_EQ(stl_itr->first, k);
    if (stl_itr->first != k) return report_errors();

    bt_type::const_iterator bt_itr = bt.find(k);
    BOOST_TEST(bt_itr != bt.end());
    if (bt_itr == bt.end()) return report_errors();
    BOOST_TEST_EQ(bt_itr->first, k);
    if (bt_itr->first != k) return report_errors();
    BOOST_TEST_EQ(stl_itr->second, bt_itr->second);
    if (stl_itr->second != bt_itr->second) return report_errors();
  }
  cout << "  finds complete" << endl;

  // iteration test

  cout << "iterating over " << n << " elements..." << endl;
  {
    stl_type::const_iterator stl_itr = stl.begin();
    bt_type::const_iterator bt_itr = bt.begin();
    for (; stl_itr != stl.end(); ++stl_itr, ++bt_itr)
    {
      //cout << stl_itr->first << ',' << stl_itr->second << ';';
      BOOST_TEST_EQ(stl_itr->first, bt_itr->first);
      if (stl_itr->first != bt_itr->first) return report_errors();
      BOOST_TEST_EQ(stl_itr->second, bt_itr->second);
      if (stl_itr->second != bt_itr->second) return report_errors();
    }
  }
  cout << "  iteration complete" << endl;

  // backward iteration test

  cout << "backward iterating over " << n << " elements..." << endl;
  {
    stl_type::const_iterator stl_itr = stl.end();
    bt_type::const_iterator bt_itr = bt.end();
    do
    {
      --stl_itr;
      --bt_itr;
      BOOST_TEST_EQ(stl_itr->first, bt_itr->first);
      if (stl_itr->first != bt_itr->first) return report_errors();
      BOOST_TEST_EQ(stl_itr->second, bt_itr->second);
      if (stl_itr->second != bt_itr->second) return report_errors();
    } while (stl_itr != stl.begin());
    BOOST_TEST(bt_itr == bt.begin());
  }
  cout << "  backward iteration complete" << endl;

  // erase test

  cout << "erasing elements..." << endl;
  rng.seed(1);
  long m = stl.size()/2;
  while (m)
  {
//    cout << "  erasing " << m << " elements..." << endl;

    for (long i = 0; i < m; ++i)
    {
      long k = key();
  //    cout << i << ',' << element.first << ';';

      stl_type::const_iterator stl_itr(stl.find(k));
      if (stl_itr == stl.end())
      {
        bt_type::const_iterator bt_itr(bt.find(k));
        BOOST_TEST(bt_itr == bt.end());
        if (bt_itr != bt.end())
          return report_errors();
      }
      else
      {
        stl.erase(stl_itr);
        bt_type::const_iterator bt_itr(bt.find(k));
        BOOST_TEST(bt_itr != bt.end());
        if (bt_itr == bt.end())
          return report_errors();
        bt.erase(bt_itr);
        BOOST_TEST_EQ(bt.size(), stl.size());
        if (bt.size() != stl.size())
          return report_errors();
      }
    }

//    cout << "  verifying " << stl.size() << " elements..." << endl;
    stl_type::const_iterator stl_itr = stl.begin();
    bt_type::const_iterator bt_itr = bt.begin();
    for (; stl_itr != stl.end(); ++stl_itr, ++bt_itr)
    {
//    cout << stl_itr->first /*<< ':' << stl_itr->second*/ << ',';
      BOOST_TEST_EQ(stl_itr->first, bt_itr->first);
      if (stl_itr->first != bt_itr->first) return report_errors();
      BOOST_TEST_EQ(stl_itr->second, bt_itr->second);
      if (stl_itr->second != bt_itr->second) return report_errors();
    }

    m = stl.size()/2;
  }

  cout << "  erases complete" << endl;

  cout << "all tests complete" << endl;

  cout << bt.manager();

  return report_errors();
}
