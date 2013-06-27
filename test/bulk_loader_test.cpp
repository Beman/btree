//  btree/test/bulk_loader_test.cpp  -------------------------------------------------//

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

#include "../volume_test/data.hpp"
#include <boost/btree/bulk_loader.hpp>
#include <boost/detail/lightweight_main.hpp>
#include <iostream>

namespace
{
   boost::btree::bulk_loader<volume::data> loader;

}

int cpp_main(int argc, char * argv[] ) 
{
  return loader.main(argc, argv, std::cout);
}
