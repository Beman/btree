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

/*
TODO:

* need to specify directory for temporary files


*/

namespace
{
  boost::filesystem::path source("d:/temp/btree.dat");
  boost::filesystem::path target("d:/temp/btree.map");
  std::size_t avail_mem = 200u;
}

int cpp_main(int argc, char* argv[]) 
{

  bulk_load_map<uint32_t, uint32_t> map;    // KISS
  map(source, target, cout, avail_mem, flags::truncate);

  return 0;
}
