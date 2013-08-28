//  bt_time.cpp  -----------------------------------------------------------------------//

//  Copyright Beman Dawes 1994, 2010

//  Distributed under the Boost Software License, Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt

//  See http://www.boost.org/libs/btree for documentation.

#include <boost/btree/btree_map.hpp>
#include <boost/btree/btree_set.hpp>
#include <boost/btree/btree_index_set.hpp>
#include <boost/btree/support/random_string.hpp>
#include <boost/filesystem.hpp>
#include <boost/random.hpp>
#include <boost/timer/timer.hpp>
#include <boost/detail/lightweight_main.hpp>

#include <iostream>
#include <locale>
#include <utility>
#include <string>
#include <map>
#include <set>
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
using std::strcmp;
using std::memcmp;

namespace
{
  std::string command_args;
  int64_t n;        // number of test cases
  std::string bt_class ("btree_map");
  int64_t seed = 1; // random number generator seed
  int64_t lg = 0;   // if != 0, log every lg iterations
  std::size_t cache_sz = -2;
  std::size_t node_sz = btree::default_node_size;
  char thou_separator = ',';
  int min_string_len = 0;
  int max_string_len = 512;
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

  struct thousands_separator : std::numpunct<char>
  { 
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

  //------------------------------------------------------------------------------------//
  //     abstract away differences in value_type and random value generation            //
  //------------------------------------------------------------------------------------//

  class map_64_64_generator
  {
    mt19937_64  m_rng;
    uniform_int<int64_t>  m_dist;
    variate_generator<mt19937_64&, uniform_int<int64_t> >  m_key;
  public:
    typedef std::map<int64_t, int64_t>  stl_type;

    map_64_64_generator(int64_t n) : m_dist(0, n-1), m_key(m_rng, m_dist) {}

    void     seed(int64_t seed_)           {m_rng.seed(seed_);}
    int64_t  key()                         {return m_key();}
    std::pair<const int64_t, int64_t>
      value(int64_t mapped_value)          {return std::make_pair(m_key(), mapped_value);}

    static int64_t stl_key(const std::pair<const int64_t, int64_t>& vt) {return vt.first;}
    static stl_type::value_type
      stl_value(const std::pair<const int64_t, int64_t>& vt) {return vt;}
  };

  class set_64_generator
  {
    mt19937_64  m_rng;
    uniform_int<int64_t>  m_dist;
    variate_generator<mt19937_64&, uniform_int<int64_t> >  m_key;
  public:
    typedef std::set<int64_t>  stl_type;

    set_64_generator(int64_t n) : m_dist(0, n-1), m_key(m_rng, m_dist) {}

    void     seed(int64_t seed_)           {m_rng.seed(seed_);}
    int64_t  key()                         {return m_key();}
    int64_t  value(int64_t)                {return m_key();}

    static int64_t stl_key(int64_t vt)                 {return vt;}
    static stl_type::value_type stl_value(int64_t vt)  {return vt;}
  };

  class set_index_64_generator
  {
    mt19937_64  m_rng;
    uniform_int<int64_t>  m_dist;
    variate_generator<mt19937_64&, uniform_int<int64_t> >  m_key;
  public:
    typedef std::set<int64_t>  stl_type;

    set_index_64_generator(int64_t n) : m_dist(0, n-1), m_key(m_rng, m_dist) {}

    void     seed(int64_t seed_)           {m_rng.seed(seed_);}
    int64_t  key()                         {return m_key();}
    int64_t  value(int64_t)                {return m_key();}

    static int64_t stl_key(int64_t vt)                 {return vt;}
    static stl_type::value_type stl_value(int64_t vt)  {return vt;}
  };

  class set_index_string_view_generator
  {
    boost::random_string m_rsg;
    std::string m_string;
  public:
    typedef std::set<std::string>  stl_type;

    set_index_string_view_generator(int64_t) 
      : m_rsg(min_string_len, max_string_len) {}

    void     seed(int64_t seed_)           {m_rsg.seed(static_cast<int>(seed_));}
    boost::string_view  key()
    {
      m_string = m_rsg();
      return boost::string_view(m_string);
    }
    boost::string_view  value(int64_t)     {return key();}

    static std::string stl_key(const boost::string_view& vt)
                                           {return std::string(vt.data(), vt.size());}
    static stl_type::value_type stl_value(const boost::string_view& vt)
                                           {return std::string(vt.data(), vt.size());}
  };

  //------------------------------------------------------------------------------------//
  //                                  test harness                                      //
  //------------------------------------------------------------------------------------//

  template <class BT, class Generator>
  void test()
  {
    timer::auto_cpu_timer t(3);
    Generator generator(n); 
    {
      btree::flags::bitmask flgs =
        do_create ? btree::flags::truncate
                  : btree::flags::read_write;
     flgs |= common_flags;
     if (!do_create && do_preload)
        flgs |= btree::flags::preload;

      cout << "\nopening " << path << endl;
      t.start();
      BT bt(path, flgs, -1, btree::less(), node_sz);
      t.stop();
      t.report();

      bt.max_cache_size(cache_sz);

      if (do_insert)
      {
        cout << "\ninserting " << n << " btree elements..." << endl;
        generator.seed(seed);
        t.start();
        timer::cpu_times then = t.elapsed();
        for (int64_t i = 1; i <= n; ++i)
        {
          if (lg && i % lg == 0)
            log(t, then, i, bt);
          bt.insert(generator.value(i));
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
        BT bt_new(path, btree::flags::truncate, -1, btree::less(), node_sz);
        bt_new.max_cache_size(cache_sz);
        for (typename BT::const_iterator it = bt_old.begin(); it != bt_old.end(); ++it)
        {
          bt_new.insert(*it);
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
        generator.seed(seed);
        typename BT::const_iterator itr;
        typename BT::key_type k;
        t.start();
        timer::cpu_times then = t.elapsed();
        for (int64_t i = 1; i <= n; ++i)
        {
          if (lg && i % lg == 0)
            log(t, then, i, bt);
          k = generator.key();
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
        typename BT::key_type prior_key = typename BT::key_type();
        t.start();
        for (typename BT::const_iterator itr = bt.begin();
          itr != bt.end();
          ++itr)
        {
          ++count;
          if (bt.key(*itr) < prior_key)
            throw std::runtime_error("btree iteration sequence error");
          prior_key = bt.key(*itr);
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
        generator.seed(seed);
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
          bt.erase(generator.key());
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

    typedef typename Generator::stl_type  stl_type;
    stl_type stl;

    if (stl_tests)
    {
      cout << "\ninserting " << n << " stl elements..." << endl;
      generator.seed(seed);
      timer::cpu_times this_tm;
      t.start();
      for (int64_t i = 1; i <= n; ++i)
      {
       stl.insert(Generator::stl_value(generator.value(i)));
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

      cout << "\nfinding " << n << " stl elements..." << endl;
      typename stl_type::const_iterator itr;
      typename stl_type::key_type k;
      generator.seed(seed);
      t.start();
      for (int64_t i = 1; i <= n; ++i)
      {
        if (lg && i % lg == 0)
          std::cout << i << std::endl; 
          k = Generator::stl_key(generator.value(i));
          itr = stl.find(k);
#       if !defined(NDEBUG)
          if (itr == stl.end())
            throw std::runtime_error("stl find() returned end()");
          if (Generator::stl_key(*itr) != k)
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
      typename stl_type::key_type prior_key;
      t.start();
      for (typename stl_type::const_iterator itr = stl.begin();
        itr != stl.end();
        ++itr)
      {
        ++count;
        if (itr != stl.begin() && Generator::stl_key(*itr) <= prior_key)
          throw std::runtime_error("stl iteration sequence error");
        prior_key = Generator::stl_key(*itr);
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
      generator.seed(seed);
      t.start();
      for (int64_t i = 1; i <= n; ++i)
      {
        if (lg && i % lg == 0)
          std::cout << i << std::endl; 
        stl.erase(Generator::stl_key(generator.value(i)));
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
      if ( strcmp( argv[2]+1, "noerase" )==0 )
        do_erase = false;
      else if ( strcmp( argv[2]+1, "nofind" )==0 )
        do_find = false;
      else if ( strcmp( argv[2]+1, "nocreate" )==0 )
        do_create = false;
      else if ( strcmp( argv[2]+1, "noiterate" )==0 )
        do_iterate = false;
      else if ( strcmp( argv[2]+1, "noinsert" )==0 )
      {
        do_create = false;
        do_insert = false;
      }
      else if ( strcmp( argv[2]+1, "stl" )==0 )
        stl_tests = true;
      else if ( strcmp( argv[2]+1, "html" )==0 )
        html = true;
      else if ( strcmp( argv[2]+1, "big" )==0 )
        whichaway = endian::order::big;
      else if ( strcmp( argv[2]+1, "little" )==0 )
        whichaway = endian::order::little;
      else if ( strcmp( argv[2]+1, "native" )==0 )
        whichaway = endian::order::native;
      else if ( strcmp( argv[2]+1, "nostats" )==0 )
        buffer_stats = false;
      else if ( strcmp( argv[2]+1, "cache-branches" )==0 )
        common_flags |= btree::flags::cache_branches;
      else if ( memcmp( argv[2]+1, "seed=", 5 )==0 && std::isdigit(*(argv[2]+6)) )
        seed = BOOST_BTREE_ATOLL( argv[2]+6 );
      else if ( memcmp( argv[2]+1, "node-sz=", 8 )==0 && std::isdigit(*(argv[2]+9)) )
        node_sz = atoi( argv[2]+9 );
      else if ( memcmp( argv[2]+1, "cache-sz=", 9 )==0
          && (std::isdigit(*(argv[2]+10)) || *(argv[2]+10) == '-') )
        cache_sz = atoi( argv[2]+10 );
      else if ( memcmp( argv[2]+1, "log=", 4 )==0 && std::isdigit(*(argv[2]+5)) )
        lg = BOOST_BTREE_ATOLL( argv[2]+5 );
      else if ( strcmp( argv[2]+1, "pack" )==0 )
        do_pack = true;
      else if ( memcmp( argv[2]+1, "sep=", 4 )==0
          && (std::ispunct(*(argv[2]+5)) || *(argv[2]+5)== '\0') )
        thou_separator = *(argv[2]+5) ? *(argv[2]+5) : ' ';
      else if ( strcmp( argv[2]+1, "preload" )==0 )
        do_preload = true;
      else if ( strcmp( argv[2]+1, "v" )==0 )
        verbose = true;
      else if ( strcmp( argv[2]+1, "class=btree_map" )==0 )
        bt_class = "btree_map";
      else if ( strcmp( argv[2]+1, "class=btree_set" )==0 )
        bt_class = "btree_set";
      else if ( strcmp( argv[2]+1, "class=btree_index_set" )==0 )
        bt_class = "btree_index_set";
      else if ( strcmp( argv[2]+1, "class=btree_index_set-string_view" )==0 )
        bt_class = "btree_index_set-string_view";
      else if ( strcmp( argv[2]+1, "hint=least-memory" )==0 )
        common_flags |= btree::flags::least_memory;
      else if ( strcmp( argv[2]+1, "hint=low-memory" )==0 )
        common_flags |= btree::flags::low_memory;
      else if ( strcmp( argv[2]+1, "hint=balanced" )==0 )
        common_flags |= btree::flags::balanced;
      else if ( strcmp( argv[2]+1, "hint=fast" )==0 )
        common_flags |= btree::flags::fast;
      else if ( strcmp( argv[2]+1, "hint=fastest" )==0 )
        common_flags |= btree::flags::fastest;
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
      "   path         Specifies the test file path; default test.btree\n"
      "   -class=btree_map|btree_set|btree_index_set|btree_index_set-string_view;\n"
      "                default -class=btree_map\n"
      "   -seed=#      Seed for random number generator; default -seed1\n"
      "   -node-size=# Node size (>=128); default -node-size"
                          << btree::default_node_size << "\n"
      "                   Small node sizes are useful for stress testing\n"
      "   -cache-size=#  Maximum # nodes cached; default is to let -hint determine\n"
      "                  cache size\n"
      "   -log=#       Log progress every # actions; default is to not log\n"
      "   -sep=[punct] cout thousands separator; space if punct omitted, default -sep,\n"
      "   -nocreate    No create; use file from prior -xe run\n"
      "   -noinsert    No insert test; forces -nocreate and doesn't do inserts\n"
      "   -nofind      No find test\n"
      "   -noiterate   No iterate test\n"
      "   -noerase     No erase test; use to save file intact\n"
      "   -nostats     No buffer statistics\n"
      "   -cache-branches  Enable caching of all branch pages touched;\n"
      "                      default is to let -hint determine branch caching\n"
      "   -pack        Pack tree after insert test\n"
      "   -v           Verbose output statistics\n"
      "   -stl         Also run the tests against std::map\n"
      "   -preload     Read entire file to preload operating system disk cache;\n"
      "                  only applicable if -nocreate option present\n"
      "   -hint=least-memory|low-memory|balanced|fast|fastest \n"
      "                  default is -hint=balanced\n"
      "   -big         Use btree::big_endian_traits; this is the default\n"
      "   -little      Use btree::little_endian_traits\n"
      "   -native      Use btree::native_traits\n"
      "   -html        Output html table of results to cerr\n"
      ;
    return 1;
  }

  if (cache_sz == static_cast<std::size_t>(-2))
    cache_sz = btree::max_cache_default(common_flags, 0);

  cout.imbue(std::locale(std::locale(), new thousands_separator));
  cout << "sizeof(bree::header_page) is " << sizeof(btree::header_page) << '\n'
       << "starting tests with node size " << node_sz
       << ", maximum cache nodes " << cache_sz << ",\n";

  if (bt_class == "btree_set")
  {
    cout << "and class btree_set" << endl;
    test< btree::btree_set<int64_t>, set_64_generator >();
    return 0;
  }
  else if (bt_class == "btree_index_set")
  {
    cout << "and class btree_index_set" << endl;
    test< btree::btree_index_set<int64_t>, set_index_64_generator >();
    return 0;
  }
  else if (bt_class == "btree_index_set-string_view")
  {
    cout << "and class btree_index_set<boost::string_view>" << endl;
    test< btree::btree_index_set<boost::string_view>, set_index_string_view_generator >();
    return 0;
  }
  else if (bt_class == "btree_map")
  {
    switch (whichaway)
    {
    case endian::order::big:
      cout << "and big endian traits\n";
      test< btree::btree_map<int64_t, int64_t, btree::big_endian_traits>,
        map_64_64_generator>();
      break;
    case endian::order::little:
      cout << "and little endian traits\n";
      test< btree::btree_map<int64_t, int64_t, btree::little_endian_traits>,
        map_64_64_generator >();
      break;
    case endian::order::native:
      cout << "and native endian traits\n";
      test< btree::btree_map<int64_t, int64_t, btree::native_endian_traits>,
        map_64_64_generator >();
      break;
    }
  }
  else
  {
    cout << "Error, unknown class: -class=" << bt_class << endl;
    return 1;
  }

  return 0;
}
