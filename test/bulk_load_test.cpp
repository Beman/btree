//  btree/test/bulk_load_test.cpp  -----------------------------------------------------//

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

#include "../tools/data.hpp"
#include <boost/btree/bulk_load.hpp>
#include <boost/timer/timer.hpp>
#include <boost/filesystem.hpp>
#include <boost/detail/lightweight_main.hpp>
#include <locale>
#include <iostream>
#include <string>
#include <cstdlib>  // for atoll() or Microsoft equivalent
#include <cctype>   // for isdigit()
#ifndef _MSC_VER
# define BOOST_BTREE_ATOLL std::atoll
#else
# define BOOST_BTREE_ATOLL _atoi64
#endif

using namespace boost::btree;
using std::cout; using std::endl;

/*
TODO:

* need to specify directory for temporary files

* add tests for multimaps, sets, multisets

*/

namespace
{
  boost::filesystem::path source_path;
  boost::filesystem::path btree_path;
  boost::filesystem::path temp_path(boost::filesystem::temp_directory_path());
  std::size_t max_memory_megabytes = 1000U;
  const std::size_t one_megabyte = 1000000U;
  std::size_t max_memory = max_memory_megabytes * one_megabyte;

  std::string command_args;
  bulk_opts::bitmask opts = bulk_opts::none;
  int64_t log_point = 0;   // if != 0, log_point every lg iterations

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
}

int cpp_main(int argc, char* argv[]) 
{
  for (int a = 0; a < argc; ++a)
  {
    command_args += argv[a];
    if (a != argc-1)
      command_args += ' ';
  }
  cout << command_args << '\n';
  const int req = 3;

  if (argc >= req)
  {
    source_path = argv[1];
    btree_path = argv[2];

    for (; argc > req; ++argv, --argc) 
    {
      if (*argv[req] != '-')
        temp_path = argv[req];
      else
      {
        if ( *(argv[req]+1) == 'm' && std::isdigit(*(argv[req]+2)) )
        {
          max_memory_megabytes = static_cast<std::size_t>(BOOST_BTREE_ATOLL(argv[req]+2));
          max_memory = max_memory_megabytes * one_megabyte;
        }
        else if ( *(argv[req]+1) == 'k' && std::isdigit(*(argv[req]+2)) )
        {
          std::size_t tmp = static_cast<std::size_t>(BOOST_BTREE_ATOLL(argv[req]+2));
          max_memory = tmp * 1024U;
        }
        else if ( *(argv[req]+1) == 'l' && std::isdigit(*(argv[req]+2)) )
          log_point = BOOST_BTREE_ATOLL( argv[req]+2 );
        else if ( *(argv[req]+1) == 'x' && *(argv[req]+2) == 'd'
                  && *(argv[req]+3) == '\0')
          opts  |= bulk_opts::skip_distribution;
        else if ( std::strncmp( argv[req]+1, "sep", 3 )==0
            && (std::ispunct(*(argv[req]+4)) || *(argv[req]+4)== '\0') )
          thou_separator = *(argv[req]+4) ? *(argv[req]+4) : ' ';
        else
        {
          cout << "Error - unknown option: " << argv[req] << "\n\n";
          argc = -1;
          break;
        }
      }
    }
  }

  if (argc < 3) 
  {
    cout << "Usage: bulk_load_test source-path target-path [Options]\n"
      "   source-path  File of binary key/mapped data pairs; must exist\n"
      "   target-path  BTree file the source data pairs will be inserted\n"
      "                into; error if already exists\n"
      " Options:\n"
      "   temp-path    Directory for temporary files; default: " << temp_path << "\n"
      "   -m#          Maximum memory # megabytes; default: " <<
                       max_memory_megabytes << "\n"
      "                Note well: The number of temporary files will be\n"
      "                source file size / maximum memory. Ensure this is reasonable!\n"
      "   -x           Skip distribution phase; use existing temporary files;\n"
      "                default: do not skip"
      "   -l#          Log progress every # actions; default: no action logging\n"
      "   -sep[punct]  Thousands separator; space if punct omitted, default -sep,\n"
      "   -k#          Maximum memory # kilobytes; default: see -m#. Note: for testing only\n"
// TODO:      "   -v       Verbose output statistics\n"
      ;
    return 1;
  }

  cout.imbue(std::locale(std::locale(), new thousands_separator));
  boost::timer::auto_cpu_timer t(1);

  //bulk_load_map<uint32_t, uint32_t> map;    // KISS
  bulk_load_map<volume::u128_t, uint64_t> loader;    
  loader(source_path, btree_path, temp_path, cout, max_memory, opts,
    log_point, flags::truncate);
  //bulk_load_set<volume::u128_t> loader;    
  //loader(source_path, btree_path, temp_path, cout, max_memory, opts,
   log_point, flags::truncate);

  t.stop();
  t.report();

  return 0;
}
