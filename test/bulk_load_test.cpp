//  btree/test/bulk_loader_test.cpp  -------------------------------------------------//

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

#include "../volume_test/data.hpp"
#include <boost/btree/bulk_load.hpp>
#include <boost/detail/lightweight_main.hpp>
#include <iostream>

using namespace boost::btree;
using std::cout; using std::endl;

namespace
{
  boost::filesystem::path source("test.dat");
  boost::filesystem::path target("test.btree");
  std::size_t avail_mem = 10000u;
}

int cpp_main(int argc, char* argv[]) 
{

  bulk_load_map<uint32_t, uint64_t> map;
  map(source, target, cout, avail_mem, flags::truncate);

  return 0;
}
