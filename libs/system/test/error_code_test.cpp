//  error_code_test.cpp  -----------------------------------------------------//

//  Copyright Beman Dawes 2006

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See library home page at http://www.boost.org/libs/system

//----------------------------------------------------------------------------// 

//  VC++ 8.0 warns on usage of certain Standard Library and API functions that
//  can be cause buffer overruns or other possible security issues if misused.
//  See http://msdn.microsoft.com/msdnmag/issues/05/05/SafeCandC/default.aspx
//  But the wording of the warning is misleading and unsettling, there are no
//  portable alternative functions, and VC++ 8.0's own libraries use the
//  functions in question. So turn off the warnings.
#define _CRT_SECURE_NO_DEPRECATE
#define _SCL_SECURE_NO_DEPRECATE

#include <boost/test/minimal.hpp>
#include <boost/system/error_code.hpp>
#include <iostream>
#include <sstream>

//  Although using directives are not the best programming practice, testing
//  with a boost::system using directive increases use scenario coverage.
using namespace boost::system;

# if defined( BOOST_WINDOWS_API )
#   include "winerror.h"
#   include <boost/cerrno.hpp>
# endif

namespace
{
  void check_ostream( error_code ec, const char * expected )
  {
    std::stringstream ss;
    std::string s;

    ss << ec;
    ss >> s;
    BOOST_CHECK( s == expected );
  }
}

//  test_main  ---------------------------------------------------------------//

// TODO: add message decoder tests
// TODO: add hash_value tests

int test_main( int, char ** )
{
  error_code ec;
  error_code ec_0_native( 0, native_ecat );
  error_code ec_0_errno( 0, errno_ecat );
  error_code ec_1_native( 1, native_ecat );
  error_code ec_1_errno( 1, errno_ecat );

  BOOST_CHECK( !ec );
  BOOST_CHECK( ec.value() == 0 );
  BOOST_CHECK( ec.to_errno() == 0 );

  BOOST_CHECK( !ec_0_native );
  BOOST_CHECK( ec_0_native.value() == 0 );
  BOOST_CHECK( ec_0_native.to_errno() == 0 );

  BOOST_CHECK( !ec_0_errno );
  BOOST_CHECK( ec_0_errno.value() == 0 );
  BOOST_CHECK( ec_0_errno.to_errno() == 0 );
  BOOST_CHECK( ec == ec_0_errno );
  BOOST_CHECK( ec_0_native != ec_0_errno );

  BOOST_CHECK( ec_1_native );
  BOOST_CHECK( ec_1_native.value() == 1 );
  BOOST_CHECK( ec_1_native.value() != 0 );
  BOOST_CHECK( ec_1_native.to_errno() != 0 );
  BOOST_CHECK( ec != ec_1_native );
  BOOST_CHECK( ec_0_native != ec_1_native );
  BOOST_CHECK( ec_0_errno != ec_1_native );

  BOOST_CHECK( ec_1_errno );
  BOOST_CHECK( ec_1_errno.value() == 1 );
  BOOST_CHECK( ec_1_errno.to_errno() == 1 );
  BOOST_CHECK( ec_1_errno.to_errno() != 0 );
  BOOST_CHECK( ec != ec_1_errno );
  BOOST_CHECK( ec_0_native != ec_1_errno );
  BOOST_CHECK( ec_0_errno != ec_1_errno );

  BOOST_CHECK( ec_0_errno.category() == errno_ecat );
  BOOST_CHECK( ec_0_errno.category() == ec_1_errno.category() );
  BOOST_CHECK( ec_0_errno.category() != native_ecat );
  BOOST_CHECK( ec_0_errno.category() != ec_0_native.category() );

  BOOST_CHECK( ec_0_errno.category().name() == "errno" );
  BOOST_CHECK( ec_0_native.category().name() == "native" );

  check_ostream( ec_0_errno, "errno:0" );
  check_ostream( ec_1_errno, "errno:1" );
  check_ostream( ec_0_native, "native:0" );
  check_ostream( ec_1_native, "native:1" );

#ifdef BOOST_WINDOWS_API
  // these tests probe the Windows to_errno decoder
  //   test the first entry in the decoder table:
  ec = error_code( ERROR_FILE_NOT_FOUND, native_ecat );
  BOOST_CHECK( ec.value() == ERROR_FILE_NOT_FOUND );
  BOOST_CHECK( ec.to_errno() == ENOENT );

  //   test the second entry in the decoder table:
  ec = error_code( ERROR_PATH_NOT_FOUND, native_ecat );
  BOOST_CHECK( ec.value() == ERROR_PATH_NOT_FOUND );
  BOOST_CHECK( ec.to_errno() == ENOENT );

  //   test the third entry in the decoder table:
  ec = error_code( ERROR_ACCESS_DENIED, native_ecat );
  BOOST_CHECK( ec.value() == ERROR_ACCESS_DENIED );
  BOOST_CHECK( ec.to_errno() == EACCES );

  //   test the last regular entry in the decoder table:
  ec = error_code( ERROR_WRITE_PROTECT, native_ecat );
  BOOST_CHECK( ec.value() == ERROR_WRITE_PROTECT );
  BOOST_CHECK( ec.to_errno() == EROFS );

  //   test not-in-table condition:
  ec = error_code( 1234567890, native_ecat );
  BOOST_CHECK( ec.value() == 1234567890 );
  BOOST_CHECK( ec.to_errno() == EOTHER );

#else
  BOOST_CHECK( ec == ec_0_native );
#endif
  
  return 0;
}


