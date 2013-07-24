//  bt_time.cpp  -----------------------------------------------------------------------//

//  Copyright Beman Dawes 1994, 2010

//  Distributed under the Boost Software License, Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt

//  See http://www.boost.org/libs/btree for documentation.

#include <boost/btree/map.hpp>
#include <boost/filesystem.hpp>
#include <boost/random.hpp>
#include <boost/timer/timer.hpp>
#include <boost/detail/lightweight_main.hpp>

#include <iostream>
#include <locale>
#include <string>
#include <map>
#include <cstring>
#include <cstdlib>  // for atoll() or Microsoft equivalent
#include <cctype>   // for isdigit()
#ifndef _MSC_VER
# define BOOST_BTREE_ATOLL std::atoll
#else
# define BOOST_BTREE_ATOLL _atoi64
#endif

using namespace boost;
namespace fs = boost::filesystem;
using std::cout;
using std::cerr;
using std::endl;

namespace
{
  std::string command_args;
  int64_t n;        // number of test cases
  int64_t seed = 1; // random number generator seed
  int64_t lg = 0;   // if != 0, log every lg iterations
  std::size_t cache_sz = btree::default_max_cache_nodes;
  std::size_t node_sz = btree::default_node_size;
  char thou_separator = ',';
  bool do_create (true);
  bool do_preload (false);
  bool do_insert (true);
  bool do_pack (false);
  bool do_find (true);
  bool do_iterate (true);
  bool do_erase (true);
  bool verbose (false);
  bool stl_tests (false);
  bool html (false);
  bool buffer_stats (true);
  bool header_info (true);
  btree::flags::bitmask common_flags = btree::flags::none;
  const int places = 2;
  std::string path("bt_time.btree");
  std::string path_old("bt_time.btree.old");
  BOOST_SCOPED_ENUM(endian::order) whichaway = endian::order::big;

  timer::cpu_times insert_tm;
  timer::cpu_times find_tm;
  timer::cpu_times iterate_tm;
  timer::cpu_times erase_tm;
  const double sec = 1000000000.0;

  struct thousands_separator : std::numpunct<char> { 
   char do_thousands_sep() const { return thou_separator; } 
   std::string do_grouping() const { return "\3"; }
};

  template <class BT>
  void log(timer::auto_cpu_timer& t, timer::cpu_times& then, int64_t i, const BT& bt)
  {
    t.stop();
    timer::cpu_times now = t.elapsed();
    cout << i << ", " << (now.wall-then.wall)/sec << " sec, "
         << lg / ((now.wall-then.wall)/sec) << " per sec,  cache size "
         << bt.manager().buffers_in_memory()
         << endl;
    then = now;
    t.resume();
  }

  template <class BT>
  void test()
  {
    timer::auto_cpu_timer t(3);
    mt19937_64  rng;
    uniform_int<int64_t> n_dist(0, n-1);
    variate_generator<mt19937_64&, uniform_int<int64_t> > key(rng, n_dist);

    {
      btree::flags::bitmask flgs =
        do_create ? btree::flags::truncate
                  : btree::flags::read_write;
     flgs |= common_flags;
     if (!do_create && do_preload)
        flgs |= btree::flags::preload;

      cout << "\nopening " << path << endl;
      t.start();
      BT bt(path, flgs, -1, node_sz);
      t.stop();
      t.report();

      bt.max_cache_size(cache_sz);

      if (do_insert)
      {
        cout << "\ninserting " << n << " btree elements..." << endl;
        rng.seed(seed);
        t.start();
        timer::cpu_times then = t.elapsed();
        for (int64_t i = 1; i <= n; ++i)
        {
          if (lg && i % lg == 0)
            log(t, then, i, bt);
          bt.emplace(key(), i);
        }
        bt.flush();
        t.stop();
        insert_tm = t.elapsed();
        t.report();
        cout << endl;
        if (header_info)
          cout << bt;
        if (buffer_stats)
          cout << bt.manager();
      }

      if (do_pack)
      {
        cout << "\npacking btree..." << endl;
        bt.manager().clear_statistics();
        bt.close();
        fs::remove(path_old);
        fs::rename(path, path_old);
        t.start();
        BT bt_old(path_old);
        bt_old.max_cache_size(cache_sz);
        BT bt_new(path, btree::flags::truncate, -1, node_sz);
        bt_new.max_cache_size(cache_sz);
        for (typename BT::iterator it = bt_old.begin(); it != bt_old.end(); ++it)
        {
          bt_new.emplace(bt_old.key(*it), bt_old.mapped(*it));
        }
        bt_new.flush();
        cout << "  bt_old.size() " << bt_old.size() << std::endl;
        cout << "  bt_new.size() " << bt_new.size() << std::endl;
        BOOST_ASSERT(bt_new.size() == bt_old.size());
        t.report();
        cout << endl;
        cout << path_old << " file size: " << fs::file_size(path_old) << '\n';
        if (header_info)
          cout << bt_old;
        if (buffer_stats)
          cout << bt_old.manager();
        cout << endl;
        cout << path << "     file size: " << fs::file_size(path) << '\n';
        if (header_info)
          cout << bt_new;
        if (buffer_stats)
          cout << bt_new.manager();
        bt_old.close();
        bt_new.close();
        bt.open(path, btree::flags::read_write | common_flags);
        bt.max_cache_size(cache_sz);
      }

      if (do_find)
      {
        cout << "\nfinding " << n << " btree elements..." << endl;
        bt.manager().clear_statistics();
        rng.seed(seed);
        typename BT::const_iterator itr;
        int64_t k;
        t.start();
        timer::cpu_times then = t.elapsed();
        for (int64_t i = 1; i <= n; ++i)
        {
          if (lg && i % lg == 0)
            log(t, then, i, bt);
          k = key();
          itr = bt.find(k);
#       if !defined(NDEBUG)
          if (itr == bt.end())
            throw std::runtime_error("btree find() returned end()");
          if (bt.key(*itr) != k)
            throw std::runtime_error("btree find() returned wrong iterator");
#       endif 
        }
        t.stop();
        find_tm = t.elapsed(); 
        t.report();
        cout << endl;
        if (header_info)
          cout << bt;
        if (buffer_stats)
          cout << bt.manager();
      }

      if (do_iterate)
      {
        cout << "\niterating over " << bt.size() << " btree elements..." << endl;
        bt.manager().clear_statistics();
        uint64_t count = 0;
        int64_t prior_key = -1L;
        t.start();
        for (typename BT::const_iterator itr = bt.begin();
          itr != bt.end();
          ++itr)
        {
          ++count;
          if (itr->first <= prior_key)
            throw std::runtime_error("btree iteration sequence error");
          prior_key = itr->first;
        }
        t.stop();
        iterate_tm = t.elapsed();
        t.report();
        cout << endl;
        if (header_info)
          cout << bt;
        if (buffer_stats)
          cout << bt.manager();
        if (count != bt.size())
          throw std::runtime_error("btree iteration count error");
      }

      if (verbose)
      {
        bt.flush();
        cout << '\n' << bt << endl;
      }

      if (do_erase)
      {
        cout << "\nerasing " << n << " btree elements..." << endl;
        bt.manager().clear_statistics();
        rng.seed(seed);
        t.start();
        timer::cpu_times then = t.elapsed();
        for (int64_t i = 1; i <= n; ++i)
        {
          if (lg && i % lg == 0)
            log(t, then, i, bt);
          //int64_t k = key();
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
        t.stop();
        bt.flush();
        erase_tm = t.elapsed();
        t.report();
        cout << endl;
        if (header_info)
          cout << bt;
        if (buffer_stats)
          cout << bt.manager();
      }

      cout << "B-tree timing complete" << endl;

      if (verbose)
      {
        bt.flush();
        cout << '\n' << bt << endl;
      }

      bt.close();
    }

    typedef std::map<int64_t, int64_t>  stl_type;
    stl_type stl;

    if (stl_tests)
    {
      cout << "\ninserting " << n << " std::map elements..." << endl;
      rng.seed(seed);
      timer::cpu_times this_tm;
      t.start();
      for (int64_t i = 1; i <= n; ++i)
      {
        stl_type::value_type element(key(), i);
        stl.insert(element);
      }
      t.stop();
      this_tm = t.elapsed();
      t.report();
      if (html)
      {
        cerr << "<tr>\n  <td><code>" << command_args << "</code></td>\n";  
        cerr.setf(std::ios_base::fixed, std::ios_base::floatfield);
        cerr.precision(places);
        if (this_tm.wall)
        {
          double ratio = (insert_tm.wall * 1.0) / this_tm.wall;
          if (ratio < 1.0)
            cerr << "  <td align=\"right\" bgcolor=\"#99FF66\">" 
                 << insert_tm.wall / sec << " sec<br>"
                 << ratio << " ratio</td>\n";
          else
            cerr << "  <td align=\"right\">" 
                 << insert_tm.wall / sec << " sec<br>"
                 << ratio << " ratio</td>\n";
        }
        else
          cerr << "  <td align=\"right\">N/A</td>\n";
      }
      if (this_tm.wall)
        cout << "  ratio of btree to stl wall clock time: "
             << (insert_tm.wall * 1.0) / this_tm.wall << '\n';
      if (verbose && this_tm.system + this_tm.user)
        cout << "  ratio of btree to stl cpu time: "
             << ((insert_tm.system + insert_tm.user) * 1.0)
                / (this_tm.system + this_tm.user) << '\n';

      cout << "\nfinding " << n << " std::map elements..." << endl;
      stl_type::const_iterator itr;
      int64_t k;
      rng.seed(seed);
      t.start();
      for (int64_t i = 1; i <= n; ++i)
      {
        if (lg && i % lg == 0)
          std::cout << i << std::endl; 
          k = key();
          itr = stl.find(k);
#       if !defined(NDEBUG)
          if (itr == stl.end())
            throw std::runtime_error("stl find() returned end()");
          if (itr->first != k)
            throw std::runtime_error("stl find() returned wrong iterator");
#       endif
      }
      t.stop();
      this_tm = t.elapsed();
      t.report();
      if (html)
      {
        if (this_tm.wall)
        {
          double ratio = (find_tm.wall * 1.0) / this_tm.wall;
          if (ratio < 1.0)
            cerr << "  <td align=\"right\" bgcolor=\"#99FF66\">" 
                 << find_tm.wall / sec << " sec<br>"
                 << ratio << " ratio</td>\n";
          else
            cerr << "  <td align=\"right\">" 
                 << find_tm.wall / sec << " sec<br>"
                 << ratio << " ratio</td>\n";
        }
        else
          cerr << "  <td align=\"right\">N/A</td>\n";
      }
      if (this_tm.wall)
        cout << "  ratio of btree to stl wall clock time: "
             << (find_tm.wall * 1.0) / this_tm.wall << '\n';
      if (verbose && this_tm.system + this_tm.user)
        cout << "  ratio of btree to stl cpu time: "
             << ((find_tm.system + find_tm.user) * 1.0)
                / (this_tm.system + this_tm.user) << '\n';

      cout << "\niterating over " << stl.size() << " stl elements..." << endl;
      uint64_t count = 0;
      int64_t prior_key = -1L;
      t.start();
      for (stl_type::const_iterator itr = stl.begin();
        itr != stl.end();
        ++itr)
      {
        ++count;
        if (itr->first <= prior_key)
          throw std::runtime_error("stl iteration sequence error");
        prior_key = itr->first;
      }
      t.stop();
      this_tm = t.elapsed();
      t.report();
      if (html)
      {
        if (this_tm.wall)
        {
          double ratio = (iterate_tm.wall * 1.0) / this_tm.wall;
          if (ratio < 1.0)
            cerr << "  <td align=\"right\" bgcolor=\"#99FF66\">" 
                 << iterate_tm.wall / sec << " sec<br>"
                 << ratio << " ratio</td>\n";
          else
            cerr << "  <td align=\"right\">" 
                 << iterate_tm.wall / sec << " sec<br>"
                 << ratio << " ratio</td>\n";
        }
        else
          cerr << "  <td align=\"right\">N/A</td>\n";
      }
      if (this_tm.wall)
        cout << "  ratio of btree to stl wall clock time: "
             << (iterate_tm.wall * 1.0) / this_tm.wall << '\n';
      if (verbose && this_tm.system + this_tm.user)
        cout << "  ratio of btree to stl cpu time: "
             << ((iterate_tm.system + iterate_tm.user) * 1.0)
                / (this_tm.system + this_tm.user) << '\n';
      if (count != stl.size())
        throw std::runtime_error("stl iteration count error");

      cout << "\nerasing " << n << " std::map elements..." << endl;
      rng.seed(seed);
      t.start();
      for (int64_t i = 1; i <= n; ++i)
      {
        if (lg && i % lg == 0)
          std::cout << i << std::endl; 
        stl.erase(key());
      }
      t.stop();
      this_tm = t.elapsed();
      t.report();
      if (html)
      {
        if (this_tm.wall)
        {
          double ratio = (erase_tm.wall * 1.0) / this_tm.wall;
          if (ratio < 1.0)
            cerr << "  <td align=\"right\" bgcolor=\"#99FF66\">" 
                 << erase_tm.wall / sec << " sec<br>"
                 << ratio << " ratio</td>\n</tr>\n";
          else
            cerr << "  <td align=\"right\">" 
                 << erase_tm.wall / sec << " sec<br>"
                 << ratio << " ratio</td>\n</tr>\n";
        }
        else
          cerr << "  <td align=\"right\">N/A</td>\n</tr>  <td align=\"right\">N/A</td>\n</tr>\n";
      }
      if (this_tm.wall)
        cout << "  ratio of btree to stl wall clock time: "
             << (erase_tm.wall * 1.0) / this_tm.wall << '\n';
      if (verbose && this_tm.system + this_tm.user)
        cout << "  ratio of btree to stl cpu time: "
             << ((erase_tm.system + erase_tm.user) * 1.0)
                / (this_tm.system + this_tm.user) << '\n';
      cout << "STL timing complete" << endl;
    }
  }

}

//-------------------------------------- main()  ---------------------------------------//

int cpp_main(int argc, char * argv[]) 
{
  for (int a = 0; a < argc; ++a)
  {
    command_args += argv[a];
    if (a != argc-1)
      command_args += ' ';
  }

  cout << command_args << '\n';;

  if (argc >=2)
    n = BOOST_BTREE_ATOLL(argv[1]);

  for (; argc > 2; ++argv, --argc) 
  {
    if (*argv[2] != '-')
      path = argv[2];
    else
    {
      if ( std::strcmp( argv[2]+1, "xe" )==0 )
        do_erase = false;
      else if ( std::strcmp( argv[2]+1, "xf" )==0 )
        do_find = false;
      else if ( std::strcmp( argv[2]+1, "xc" )==0 )
        do_create = false;
      else if ( std::strcmp( argv[2]+1, "xi" )==0 )
      {
        do_create = false;
        do_insert = false;
      }
      else if ( std::strcmp( argv[2]+1, "stl" )==0 )
        stl_tests = true;
      else if ( std::strcmp( argv[2]+1, "html" )==0 )
        html = true;
      else if ( std::strcmp( argv[2]+1, "big" )==0 )
        whichaway = endian::order::big;
      else if ( std::strcmp( argv[2]+1, "little" )==0 )
        whichaway = endian::order::little;
      else if ( std::strcmp( argv[2]+1, "native" )==0 )
        whichaway = endian::order::native;
      else if ( std::strcmp( argv[2]+1, "xbstats" )==0 )
        buffer_stats = false;
      else if ( std::strcmp( argv[2]+1, "b" )==0 )
        common_flags |= btree::flags::cache_branches;
      else if ( *(argv[2]+1) == 's' && std::isdigit(*(argv[2]+2)) )
        seed = BOOST_BTREE_ATOLL( argv[2]+2 );
      else if ( *(argv[2]+1) == 'n' && std::isdigit(*(argv[2]+2)) )
        node_sz = atoi( argv[2]+2 );
      else if ( *(argv[2]+1) == 'c'
          && (std::isdigit(*(argv[2]+2)) || *(argv[2]+2) == '-') )
        cache_sz = atoi( argv[2]+2 );
      else if ( *(argv[2]+1) == 'l' && std::isdigit(*(argv[2]+2)) )
        lg = BOOST_BTREE_ATOLL( argv[2]+2 );
      else if ( std::strcmp( argv[2]+1, "p" )==0 )
        do_pack = true;
      else if ( std::strncmp( argv[2]+1, "sep", 3 )==0
          && (std::ispunct(*(argv[2]+4)) || *(argv[2]+4)== '\0') )
        thou_separator = *(argv[2]+4) ? *(argv[2]+4) : ' ';
      else if ( std::strcmp( argv[2]+1, "r" )==0 )
        do_preload = true;
      else if ( std::strcmp( argv[2]+1, "v" )==0 )
        verbose = true;
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
      "   path     Specifies the test file path; default test.btree\n"
      "   -s#      Seed for random number generator; default 1\n"
      "   -n#      Node size (>=128); default " << btree::default_node_size << "\n"
      "              Small node sizes are useful for stress testing\n"
      "   -c#      Cache size; default " << btree::default_max_cache_nodes << " nodes\n"
      "   -l#      log progress every # actions; default is to not log\n"
      "   -sep[punct] cout thousands separator; space if punct omitted, default -sep,\n"
      "   -xc      No create; use file from prior -xe run\n"
      "   -xi      No insert test; forces -xc and doesn't do inserts\n"
      "   -xf      No find test\n"
      "   -xi      No iterate test\n"
      "   -xe      No erase test; use to save file intact\n"
      "   -b       Enable permanent cache of branch pages touched; default is\n"
      "              to make available for flush when use count reaches 0\n"
      "   -p       Pack tree after insert test\n"
      "   -v       Verbose output statistics\n"
      "   -stl     Also run the tests against std::map\n"
      "   -r       Read entire file to preload operating system disk cache;\n"
      "            only applicable if -xc option is active\n"
      "   -big     Use btree::big_endian_traits; this is the default\n"
      "   -little  Use btree::little_endian_traits\n"
      "   -native  Use btree::native_traits\n"
      "   -html    Output html table of results to cerr\n"
      ;
    return 1;
  }
  cout.imbue(std::locale(std::locale(), new thousands_separator));
  cout << "sizeof(bree::header_page) is " << sizeof(btree::header_page) << '\n'
       << "starting tests with node size " << node_sz
       << ", maximum cache nodes " << cache_sz << ",\n";

  switch (whichaway)
  {
  case endian::order::big:
    cout << "and big endian traits\n";
    test< btree::btree_map<int64_t, int64_t, btree::big_endian_traits> >();
    break;
  case endian::order::little:
    cout << "and little endian traits\n";
    test< btree::btree_map<int64_t, int64_t, btree::little_endian_traits> >();
    break;
  case endian::order::native:
    cout << "and native endian traits\n";
    test< btree::btree_map<int64_t, int64_t, btree::native_endian_traits> >();
    break;
  }

  return 0;
}
