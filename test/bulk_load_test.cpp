//  btree/test/bulk_loader_test.cpp  -------------------------------------------------//

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

#include "../volume_test/data.hpp"
#include <boost/btree/bulk_load.hpp>
#include <boost/timer/timer.hpp>
#include <boost/detail/lightweight_main.hpp>
#include <iostream>

using namespace boost::btree;
using std::cout; using std::endl;

/*
TODO:

* need to specify directory for temporary files


*/

namespace
{
  boost::filesystem::path source("d:/temp/btree.dat");
  boost::filesystem::path target("d:/temp/btree.map");
  //std::size_t avail_mem = 200u;
  std::size_t avail_mem = 4000000000ULL;
}

int cpp_main(int argc, char* argv[]) 
{
  boost::timer::auto_cpu_timer t(3);

  //bulk_load_map<uint32_t, uint32_t> map;    // KISS
  bulk_load_map<volume::u128_t, uint64_t> map;    
  map(source, target, cout, avail_mem, flags::truncate);

  t.stop();
  t.report();

  return 0;
}
