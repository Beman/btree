//  example/tune_int_map.cpp 

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

///$id code=
#include <boost/btree/btree_map.hpp>
#include <iostream>
#include <boost/detail/lightweight_main.hpp>
#include <boost/timer/timer.hpp>
#include <cstdlib>  // for rand()
#include <string>

using std::cout;
using namespace boost::btree;

int cpp_main(int argc, char *argv[])
{
  long n = 10000;
  flags::bitmask hint = flags::fastest;

  if (argc > 1)
    { n = std::atol(*++argv); --argc; }
  if (argc > 1)
  {
    std::string s(*++argv);
    if (s == "least_memory") hint = flags::least_memory;
    else if (s == "low_memory") hint = flags::low_memory;
    else if (s == "balanced") hint = flags::balanced;
    else if (s == "fast") hint = flags::fast;
    else if (s == "fastest") hint = flags::fastest;
    else
    {
      cout <<
        "invoke: tune_int_map [#] least_memory|low_memory|balanced|fast|fastest\n"
        "  where # is the number of elements to test\n"
        "  example: tune_int_map 1000000 fast\n";
      return 1;
    }
  }
  
  typedef btree_map<int, int> BT;
  BT bt;                                                // unopened btree_map
  bt.open("tune_int_map.btr", flags::truncate | hint);  // open it

  cout << "test with " << n << " elements" << std::endl;
  boost::timer::auto_cpu_timer t(2);

  //  emplace n elements
  for (int i = 1; i <= n; ++i)
    bt.emplace(std::rand(), i);

  bt.close();  // include close() cost in timings
  t.stop();
  cout << "emplace() complete"<< std::endl;
  t.report();

  std::srand(1);
  t.start();
  bt.open("tune_int_map.btr", hint);

  //  find n elements
  for (int i = 1; i <= n; ++i)
    bt.find(std::rand());

  bt.close();
  t.stop();
  cout << "find() complete\n";
  t.report();

  return 0;
}
///$endid
