//  bt_time.cpp  -----------------------------------------------------------------------//

//  Copyright Beman Dawes 1994, 2010

//  Distributed under the Boost Software License, Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt

//  See http://www.boost.org/libs/btree for documentation.

#include <boost/btree/map.hpp>
#include <boost/random.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <boost/system/timer.hpp>
#include <iostream>
#include <cstring>
#include <cstdlib>  // for atol()
#include <map>

using namespace boost;
using std::cout;
using std::endl;

namespace
{
  long count;
  long n;
  long initial_n;
  long seed = 1;
  long lg = 0;
  int cache_sz = 16;
  int page_sz = 512;
  bool do_create (true);
  bool do_preload (false);
  bool do_insert (true);
  bool do_find (true);
  bool do_erase (true);
  bool stl_tests (false);
  std::string path("bt_time.btree");

  system::times_t insert_tm;
  system::times_t find_tm;
  system::times_t erase_tm;
}

//-------------------------------------- main()  ---------------------------------------//

int main(int argc, char * argv[]) 
{

  if (argc >=2)
    n = std::atol(argv[1]);

  for (; argc > 2; ++argv, --argc) 
  {
    if (*argv[2] != '-')
      path = argv[2];
    else
    {
      if ( std::strncmp( argv[2]+1, "xe", 2 )==0 )
        do_erase = false;
      else if ( std::strncmp( argv[2]+1, "xf", 2 )==0 )
        do_find = false;
      else if ( std::strncmp( argv[2]+1, "xc", 2 )==0 )
        do_create = false;
      else if ( std::strncmp( argv[2]+1, "xi", 2 )==0 )
      {
        do_create = false;
        do_insert = false;
      }
      else if ( std::strncmp( argv[2]+1, "stl", 3 )==0 )
        stl_tests = true;
      else if ( *(argv[2]+1) == 's' )
        seed = atol( argv[2]+2 );
      else if ( *(argv[2]+1) == 'p' )
        page_sz = atoi( argv[2]+2 );
      else if ( *(argv[2]+1) == 'c' )
        cache_sz = atoi( argv[2]+2 );
      else if ( *(argv[2]+1) == 'i' )
        initial_n = atol( argv[2]+2 );
      else if ( *(argv[2]+1) == 'l' )
        lg = atol( argv[2]+2 );
      else if ( *(argv[2]+1) == 'r' )
        do_preload = true;
      else
      {
        cout << "Error - unknown option: " << argv[2] << "\n\n";
        argc = -1;
        break;
      }
    }
  }

  if (argc < 2) 
  {
    cout << "Usage: bt_time n [Options]\n"
      " The argument n specifies the number of test cases to run\n"
      " Options:\n"
      "   path   Specifies the test file path; default test.btree\n"
      "   -s#    Seed for random number generator; default 1\n"
      "   -p#    Page size (>=128); default 512\n"
      "            Small page sizes are useful for stress testing\n"
      "   -c#    Cache size; default 16 pages.\n"
      "   -l#    log progress every # actions; default is to not log\n"
      "   -xc    No create; use file from prior -xe run\n"
      "   -xi    No insert; forces -xc and doesn't do inserts\n"
      "   -xf    No find\n"
      "   -xe    No erase; use to save file intact\n"
      "   -stl   Also run the tests against std::map\n"
      "   -r     Read entire file to preload operating system disk cache;"
      "          only applicable if -xc option is active\n"
      ;
    return 1;
  }

  system::run_timer t(3);
  rand48  rng;
  uniform_int<long> n_dist(0, n-1);
  variate_generator<rand48&, uniform_int<long> > key(rng, n_dist);

  {
    typedef btree::btree_map<long, long> bt_type;
    btree::flags::bitmask flgs = do_create ? btree::flags::truncate : btree::flags::read_write;
    if (!do_create && do_preload)
      flgs |= btree::flags::preload;

    cout << "\nopening " << path << endl;
    t.start();
    bt_type bt(path, flgs, page_sz);
    t.stop();
    t.report();

    bt.max_cache_pages(cache_sz);

    if (do_insert)
    {
      cout << "\ninserting " << n << " btree elements..." << endl;
      rng.seed(seed);
      t.start();
      for (long i = 1; i <= n; ++i)
      {
        if (lg && i % lg == 0)
          std::cout << i << std::endl; 
        bt_type::value_type element(key(), i);
        bt.insert(element);
      }
      insert_tm = t.stop();
      t.report();
      cout << "  btree insertion complete" << endl;
    }

    if (do_find)
    {
      cout << "\nfinding " << n << " btree elements..." << endl;
      rng.seed(seed);
      bt_type::const_iterator itr;
      long k;
      t.start();
      for (long i = 1; i <= n; ++i)
      {
        if (lg && i % lg == 0)
          std::cout << i << std::endl;
        k = key();
        itr = bt.find(k);
        if (itr == bt.end())
          throw std::runtime_error("btree find() returned end()");
        if (itr->first != k)
          throw std::runtime_error("btree find() returned wrong iterator");
      }
      find_tm = t.stop();
      t.report();
      cout << "  btree finds complete" << endl;
    }

    if (do_erase)
    {
      cout << "\nerasing " << n << " btree elements..." << endl;
      rng.seed(seed);
      t.start();
      for (long i = 1; i <= n; ++i)
      {
        if (lg && i % lg == 0)
          std::cout << i << std::endl; 
        //long k = key();
        //if (i >= n - 5)
        //{
        //  std::cout << i << ' ' << k << ' ' << bt.size() << std::endl;
        //  std::cout << "erase(k) returns " << bt.erase(k) << std::endl;
        //  std::cout << "and size() returns " << bt.size() << std::endl;
        //}
        //else
        //  bt.erase(k);
        bt.erase(key());
      }
      erase_tm = t.stop();
      t.report();
      cout << "  btree erases complete" << endl;
    }

    bt.flush();
    cout << '\n' << bt << endl;
    cout << bt.manager() << endl;
  }

  typedef std::map<long, long>  stl_type;
  stl_type stl;

  if (stl_tests)
  {
    cout << "\ninserting " << n << " std::map elements..." << endl;
    rng.seed(seed);
    system::times_t this_tm;
    t.start();
    for (long i = 1; i <= n; ++i)
    {
      if (lg && i % lg == 0)
        std::cout << i << std::endl; 
      stl_type::value_type element(key(), i);
      stl.insert(element);
    }
    this_tm = t.stop();
    t.report();
    if (this_tm.wall)
      cout << "  ratio of btree to stl wall time: "
           << (insert_tm.wall * 1.0) / this_tm.wall << '\n';
    if (this_tm.system + this_tm.user)
      cout << "  ratio of btree to stl cpu time: "
           << ((insert_tm.system + insert_tm.user) * 1.0)
              / (this_tm.system + this_tm.user) << '\n';
    cout << "  std::map insertion complete" << endl;

    cout << "\nfinding " << n << " std::map elements..." << endl;
    stl_type::const_iterator itr;
    long k;
    rng.seed(seed);
    t.start();
    for (long i = 1; i <= n; ++i)
    {
      if (lg && i % lg == 0)
        std::cout << i << std::endl; 
        k = key();
        itr = stl.find(k);
        if (itr == stl.end())
          throw std::runtime_error("stl find() returned end()");
        if (itr->first != k)
          throw std::runtime_error("stl find() returned wrong iterator");
    }
    this_tm = t.stop();
    t.report();
    if (this_tm.wall)
      cout << "  ratio of btree to stl wall time: "
           << (find_tm.wall * 1.0) / this_tm.wall << '\n';
    if (this_tm.system + this_tm.user)
      cout << "  ratio of btree to stl cpu time: "
           << ((find_tm.system + find_tm.user) * 1.0)
              / (this_tm.system + this_tm.user) << '\n';
    cout << "  std::map finds complete" << endl;

    cout << "\nerasing " << n << " std::map elements..." << endl;
    rng.seed(seed);
    t.start();
    for (long i = 1; i <= n; ++i)
    {
      if (lg && i % lg == 0)
        std::cout << i << std::endl; 
      stl.erase(key());
    }
    this_tm = t.stop();
    t.report();
    if (this_tm.wall)
      cout << "  ratio of btree to stl wall time: "
           << (erase_tm.wall * 1.0) / this_tm.wall << '\n';
    if (this_tm.system + this_tm.user)
      cout << "  ratio of btree to stl cpu time: "
           << ((erase_tm.system + erase_tm.user) * 1.0)
              / (this_tm.system + this_tm.user) << '\n';
    cout << "  std::map erases complete" << endl;
  }

  return 0;
}
