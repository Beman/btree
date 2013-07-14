//  mmff_unit_test.cpp  ----------------------------------------------------------------//

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//--------------------------------------------------------------------------------------//
//                                                                                      //
//            memory-mapped flat-file unit test; mmff_unit_test for short               //
//                                                                                      //
//--------------------------------------------------------------------------------------//

#include <boost/btree/mmff.hpp>
#include <boost/detail/lightweight_main.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <boost/btree/support/random_string.hpp>

#include <iostream>

using std::cout; using std::endl;

namespace
{


//-------------------------------------  _test  ----------------------------------------//

void  _test()
{
  cout << "  _test..." << endl;

  cout << "     _test complete" << endl;
}

}  // unnamed namespace

//------------------------------------ cpp_main ----------------------------------------//

int cpp_main(int argc, char* argv[])
{
  std::string command_args;

  for (int a = 0; a < argc; ++a)
  {
    command_args += argv[a];
    if (a != argc-1)
      command_args += ' ';
  }

  cout << command_args << '\n';;


  cout << "all tests complete" << endl;

  return boost::report_errors();
}
