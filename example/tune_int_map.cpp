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

using std::cout;
using namespace boost::btree;

int cpp_main(int argc, char *argv[])
{
  long n = 10000;
  if (argc > 1)
    n = std::atol(*(argv+1));

  flags::bitmask hint = flags::fastest;  // hint can be least_memory, low_memory,
                                         // balanced, fast, or fastest 

  typedef btree_map<int, int> BT;
  BT bt("tune_int_map.btr", flags::truncate | hint);

  cout << "test with " << n << " elements" << std::endl;
  boost::timer::auto_cpu_timer t(2);

  for (int i = 1; i <= n; ++i)
    bt.emplace(std::rand(), i);

  bt.close();  // include cost of close() in timings
  t.stop();
  cout << "emplace() complete"<< std::endl;
  t.report();

  std::srand(1);
  t.start();
  bt.open("tune_int_map.btr", hint);   // open previously constructed btree

  for (int i = 1; i <= n; ++i)
    bt.find(std::rand());

  bt.close();
  t.stop();
  cout << "find() complete\n";
  t.report();

  return 0;
}
///$endid
