//  create_data.cpp  -------------------------------------------------------------------//

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

#include "data.hpp"

#include <boost/config.hpp>
#include <boost/cstdint.hpp>
#include <boost/filesystem.hpp>
#include <boost/random.hpp>
#include <boost/timer/timer.hpp>
#include <boost/detail/lightweight_main.hpp>
#include <boost/assert.hpp>
#include <boost/static_assert.hpp>
#include <fstream>
#include <limits>
#include <iostream>
#include <locale>
#include <string>
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
  char thou_separator = ',';
  bool verbose (false);
  const int places = 2;
  std::string path("data.dat");

  std::ofstream out;

  const double sec = 1000000000.0;

  struct thousands_separator : std::numpunct<char> { 
   char do_thousands_sep() const { return thou_separator; } 
   std::string do_grouping() const { return "\3"; }
};

  void log(timer::auto_cpu_timer& t, timer::cpu_times& then, int64_t i)
  {
    t.stop();
    timer::cpu_times now = t.elapsed();
    cout << i << ", " << (now.wall-then.wall)/sec << " sec, "
         << lg / ((now.wall-then.wall)/sec) << " per sec"
         << endl;
    then = now;
    t.resume();
  }

  void generate_data()
  {
    timer::auto_cpu_timer t(3);
    mt19937_64  rng;
    uniform_int<int64_t> n_dist(0, std::numeric_limits<int64_t>::max());
    variate_generator<mt19937_64&, uniform_int<int64_t> > key(rng, n_dist);

    cout << "creating " << n << " data elements in " << path << endl;
    rng.seed(seed);
    t.start();
    timer::cpu_times then = t.elapsed();

    volume::data dat;

    for (int64_t i = 1; i <= n; ++i)
    {
      if (lg && i % lg == 0)
        log(t, then, i);
      dat.key.assign(key(), key());
      dat.mapped = i;
      out.write(reinterpret_cast<const char*>(&dat), sizeof(dat));
    }
    t.stop();
    t.report();
    cout << "  data element creation complete" << endl;
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
      if ( *(argv[2]+1) == 's' && std::isdigit(*(argv[2]+2)) )
        seed = BOOST_BTREE_ATOLL( argv[2]+2 );
      else if ( *(argv[2]+1) == 'l' && std::isdigit(*(argv[2]+2)) )
        lg = BOOST_BTREE_ATOLL( argv[2]+2 );
      else if ( std::strncmp( argv[2]+1, "sep", 3 )==0
          && (std::ispunct(*(argv[2]+4)) || *(argv[2]+4)== '\0') )
        thou_separator = *(argv[2]+4) ? *(argv[2]+4) : ' ';
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
    cout << "Usage: create_data n [Options]\n"
      " The argument n specifies the number of test cases to run\n"
      " Options:\n"
      "   path     Specifies the test file path; default " << path << "\n"
      "   -s#      Seed for random number generator; default 1\n"
      "   -l#      log progress every # actions; default is to not log\n"
      "   -sep[punct] cout thousands separator; space if punct omitted, default -sep,\n"
// TODO:      "   -v       Verbose output statistics\n"
      ;
    return 1;
  }

  cout.imbue(std::locale(std::locale(), new thousands_separator));

  out.open(path.c_str(), std::ios_base::out | std::ios_base::binary);

  if (!out)
  {
    cout << "failed to open " << path << endl;
    return 1;
  }

  generate_data();
  out.close();

  return 0;
}
