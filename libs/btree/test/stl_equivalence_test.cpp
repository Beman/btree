//  stl_equivalence_test.hpp  ----------------------------------------------------------//

//  Copyright Beman Dawes 2010

//  Distributed under the Boost Software License, Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt

//  See http://www.boost.org/libs/btree for documentation.

#include <boost/btree/map.hpp>
#include <map>
#include <boost/random.hpp>
#include <boost/cstdint.hpp>
#include <iostream>
#include <cstdlib>  // for atol()
#include <cstring>  // for strcmp(), strncmp()
#include <stdexcept>
#include <utility>

using std::atol;
using std::strcmp;
using std::strncmp;
using std::cout;
using std::endl;
using std::pair;
using std::runtime_error;

namespace
{
  std::string command_args;
  std::string path_prefix("stl_equivalence");
  int32_t max = 10000;
  int32_t min = 10;
  int32_t cycles = 3;
  int32_t seed = 1;
  int32_t page_sz = 128;
  int32_t cache_sz = 2;
  bool restart = false;
  bool verbose = false;

  uint64_t insert_success_count = 0;
  uint64_t insert_fail_count = 0;
  uint64_t erase_success_count = 0;
  uint64_t erase_fail_count = 0;
  uint64_t iterate_forward_count = 0;
  uint64_t iterate_backward_count = 0;
  uint64_t find_count = 0;
  uint64_t lower_bound_count = 0;
  uint64_t upper_bound_count = 0;
  uint32_t cycles_complete = 0;

  boost::rand48  insert_rng;
  boost::rand48  erase_rng;
  boost::uniform_int<int32_t> n_dist(0, max-1);
  boost::variate_generator<boost::rand48&, boost::uniform_int<int32_t> >
    insert_key(insert_rng, n_dist);
  boost::variate_generator<boost::rand48&, boost::uniform_int<int32_t> >
    erase_key(erase_rng, n_dist);

  typedef boost::btree::btree_map<int32_t, int32_t> bt_type;
  bt_type bt;

  typedef std::map<int32_t, int32_t>  stl_type;
  stl_type stl;

  typedef stl_type::value_type value_type;


  //  insert test  ---------------------------------------------------------------------//

  void insert_test()
  {
    cout << "insert test..." << endl;
    while (stl.size() < static_cast<stl_type::size_type>(max))
    {
      value_type element(insert_key(),0);
      element.second = element.first;

      pair<stl_type::iterator, bool> stl_result = stl.insert(element);
      pair<bt_type::const_iterator, bool> bt_result = bt.insert(element);

      if (stl_result.second != bt_result.second)
      {
        cout << "failure inserting element " << element.first << endl;
        throw runtime_error("insert: stl_result.second != bt_result.second");
      }

      if (stl_result.second)
        ++insert_success_count;
      else
        ++insert_fail_count;
    }
    if (stl.size() != bt.size())
    {
      cout << "stl.size() " << stl.size() << " != bt.size() " << bt.size() << endl;
      throw runtime_error("insert: size check failure");
    }
    cout << "  insert test complete" << endl;
  }
  //  iteration test  ------------------------------------------------------------------//

  void iteration_test()
  {
    cout << "iteration test..." << endl;
    stl_type::const_iterator stl_itr = stl.begin();
    bt_type::const_iterator bt_itr = bt.begin();

    for (; stl_itr != stl.end() && bt_itr != bt.end(); ++stl_itr, ++bt_itr)
    {
      if (stl_itr->first != bt_itr->first)
      {
        cout << "stl_itr->first " << stl_itr->first << " != "
              << "bt_itr->first " << bt_itr->first << endl;
        throw runtime_error("iteration: first check failure");
      }
      if (stl_itr->second != bt_itr->second)
      {
        cout << "stl_itr->second " << stl_itr->second << " != "
              << "bt_itr->second " << bt_itr->second << endl;
        throw runtime_error("iteration: second check failure");
      }
      ++iterate_forward_count;
    }

    if (stl_itr != stl.end())
      throw runtime_error("iteration: bt at end() but stl not at end()");
    if (bt_itr != bt.end())
      throw runtime_error("iteration: stl at end() but bt not at end()");
    cout << "  iteration test complete" << endl;
  }

  //  backward iteration test  ---------------------------------------------------------//

  void backward_iteration_test()
  {
    cout << "backward iteration test..." << endl;
    stl_type::const_iterator stl_itr = stl.end();
    bt_type::const_iterator bt_itr = bt.end();

    do
    {
      --stl_itr;
      --bt_itr;
      if (stl_itr->first != bt_itr->first)
      {
        cout << "stl_itr->first " << stl_itr->first << " != "
              << "bt_itr->first " << bt_itr->first << endl;
        throw runtime_error("backward iteration: first check failure");
      }
      if (stl_itr->second != bt_itr->second)
      {
        cout << "stl_itr->second " << stl_itr->second << " != "
              << "bt_itr->second " << bt_itr->second << endl;
        throw runtime_error("backward iteration: second check failure");
      }
      ++iterate_backward_count;
    } while (stl_itr != stl.begin() && bt_itr != bt.begin());

    if (stl_itr != stl.begin())
      throw runtime_error("iteration: bt at begin() but stl not at begin()");
    if (bt_itr != bt.begin())
      throw runtime_error("iteration: stl at begin() but bt not at begin()");
    cout << "  backward iteration complete" << endl;
  }

  //  erase test  ----------------------------------------------------------------------//

  void erase_test()
  {
    cout << "erase test..." << endl;
    while (stl.size() > static_cast<stl_type::size_type>(min))
    {
      int32_t k = erase_key();
      stl_type::size_type stl_result = stl.erase(k);
      bt_type::size_type bt_result = bt.erase(k);

      if (stl_result != bt_result)
      {
        cout << "stl_result " << stl_result << " != bt_result " << bt_result << endl;
        throw runtime_error("erase: result failure");
      }

      if (stl_result)
        ++erase_success_count;
      else
        ++erase_fail_count;
    }
    if (stl.size() != bt.size())
    {
      cout << "stl.size() " << stl.size() << " != bt.size() " << bt.size() << endl;
      throw runtime_error("erase: size check failure");
    }
    cout << "  erase test complete" << endl;
  }

  //  run test cycles  -----------------------------------------------------------------//

  void tests()
  {
    insert_rng.seed(seed);
    erase_rng.seed(seed);

    bt.open(path_prefix + ".btr", boost::btree::flags::truncate, page_sz);
    bt.max_cache_pages(cache_sz);  // small cache to incease stress

    for (int32_t cycle = 1; cycle <= cycles; ++cycle)
    {
      cout << "Beginning cycle " << cycle << " ..." << endl;

      insert_test();
      iteration_test();
      backward_iteration_test();
      erase_test();
      iteration_test();
      backward_iteration_test();

      cout << "  cycle " << cycle << " complete" << endl;
      ++cycles_complete;
    }

  //  // find test

  //  cout << "finding " << n << " elements..." << endl;
  //  rng.seed(1);
  //  for (long i = 0; i < n; ++i)
  //  {
  ////    cout << i << ',';
  //    long k = key();
  //    stl_type::const_iterator stl_itr = stl.find(k);
  //    BOOST_TEST(stl_itr != stl.end());
  //    if (stl_itr == stl.end()) return report_errors();
  //    BOOST_TEST_EQ(stl_itr->first, k);
  //    if (stl_itr->first != k) return report_errors();

  //    bt_type::const_iterator bt_itr = bt.find(k);
  //    BOOST_TEST(bt_itr != bt.end());
  //    if (bt_itr == bt.end()) return report_errors();
  //    BOOST_TEST_EQ(bt_itr->first, k);
  //    if (bt_itr->first != k) return report_errors();
  //    BOOST_TEST_EQ(stl_itr->second, bt_itr->second);
  //    if (stl_itr->second != bt_itr->second) return report_errors();
  //  }
  //  cout << "  finds complete" << endl;


  //  cout << "all tests complete" << endl;

  //  cout << bt.manager();
  }

}  // unnamed namespace

int main(int argc, char *argv[])
{
  for (int a = 0; a < argc; ++a)
  {
    command_args += argv[a];
    if (a != argc-1)
      command_args += ' ';
  }

  cout << "command line arguments: " << command_args << '\n';;

  for (; argc > 1; ++argv, --argc) 
  {
    if (*argv[1] != '-')
      path_prefix = argv[1];
    else
    {
      if (strncmp(argv[1]+1, "max=", 4) == 0)
        max = atol(argv[1]+5);
      else if (strncmp(argv[1]+1, "min=", 4) == 0)
        min = atol(argv[1]+5);
      else if (strncmp(argv[1]+1, "cycles=", 7) == 0)
        cycles = atol(argv[1]+8);
      else if (strncmp(argv[1]+1, "seed=", 5) == 0)
        seed = atol(argv[1]+6);
      else if (strncmp(argv[1]+1, "page_sz=", 8) == 0)
        page_sz = atol(argv[1]+9);
      else if (strncmp(argv[1]+1, "cache_sz=", 9) == 0)
        cache_sz = atol(argv[1]+10);
      else if (strcmp(argv[1]+1, "restart") == 0)
        restart = true;
      else if (strcmp(argv[1]+1, "v") == 0)
        verbose = true;
      else
      {
        cout << "Error - unknown option: " << argv[1] << "\n\n";
        argc = -1;
        break;
      }
    }
  }

  if (argc < 2) 
  {
    cout << "Usage: stl_equivalence_test [Options]\n"
      "The argument n specifies the number of test cases to run\n"
      "Options:\n"
      "   path-prefix  Test files path-prefix; default '" << path_prefix << "'\n"
      "                Two files will be created; path-prefix.btr and path-prefix.stl\n"
      "   -max=#       Maximum number of test elements; default " << max << "\n"
      "   -min=#       Minimum number of test elements; default " << min << "\n"
      "   -cycles=#    Cycle tests specified number of times; default " << cycles << "\n"
      "                -cycles=0 causes tests to cycle forever\n"
      "   -seed=#      Seed for random number generator; default"  << seed << "\n"
      "   -page=#      Page size (>=128); default " << page_sz << "\n"
      "                Small page sizes increase stress\n"
      "   -cache=#     Cache size; default " << cache_sz << " pages\n"
      "   -restart     Restart using files from prior run\n"
      "   -v           Verbose output statistics\n"
      "\n    Each test cycle inserts the same random value into both a btree_map\n"
      "and a std::map until the maximum number of elements is reached. Elements\n"
      "a second random number generator, started with the same seed, will then\n"
      "be erased until the minimum number of elements is reached. The btree is\n"
      "then flushed and copied, and the std::map is dumped to a file, and the\n"
      "cycle ends.\n"
      "    At the maximum and minimum points of each cycle, forward iteration,\n"
      "backward iteration, find, lower_bound, and upper_bound tests are run\n"
      "against both containers, checking that identical results are obtained.\n"
      ;
  }

  if (argc == -1)
    return 1;

  cout << "starting tests with:\n"
       << "  path_prefix = " << path_prefix << '\n'
       << "  max = " << max << '\n'
       << "  min = " << min << '\n'
       << "  cycles = " << cycles << '\n'
       << "  seed = " << seed << '\n'
       << "  page size = " << page_sz << '\n'
       << "  max cache pages = " << cache_sz << "\n\n";

  try
  {
    tests();
  }

  catch (const std::runtime_error& ex)
  {
    cout << "\n*************** exception  ******************\n"
         << ex.what() << endl;
    return 1;
  }
  catch (...)
  {
    cout << "\n*************** exception  ******************\n"
         << "of a type not derived from std::runtime_error" << endl;
    return 1;
  }

  cout << "all test cycles complete" << endl;
  return 0;
}
