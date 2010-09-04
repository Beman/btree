//  endian_test.cpp  ---------------------------------------------------------//

//  Copyright Beman Dawes 1999-2008

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  See library home page at http://www.boost.org/libs/endian

//----------------------------------------------------------------------------//

//  This test probes for correct endianess, size, and value.

//  See endian_operations_test for tests of operator correctness and interaction
//  between operand types.

//----------------------------------------------------------------------------//

#include <boost/integer/endian.hpp>
#include <boost/cstdint.hpp>
#include <boost/progress.hpp>

#include <iostream>
#include <limits>
#include <climits>
#include <cstdlib>    // for atoi(), exit()
#include <cstring>    // for memcmp()

using namespace std;             // Not the best programming practice, but I
using namespace boost;           //   want to verify this combination of using
using namespace boost::integer;  //   namespaces works. See endian_operations_test
//                               //   for tests that don't do "using namespace".

#define VERIFY(predicate) verify( predicate, __LINE__ )
#define VERIFY_SIZE(actual, expected) verify_size( actual, expected, __LINE__ )
#define VERIFY_VALUE_AND_OPS(endian_t,expected_t,expected) verify_value_and_ops<endian_t, expected_t>( expected, __LINE__ )
#define VERIFY_BIG_REPRESENTATION(t) verify_representation<t>( true, __LINE__ )
#define VERIFY_LITTLE_REPRESENTATION(t) verify_representation<t>( false, __LINE__ )
#define VERIFY_NATIVE_REPRESENTATION(t) verify_native_representation<t>( __LINE__ )

namespace
{
  int err_count;

  void verify( bool x, int line )
  {
    if ( x ) return;
    ++err_count;
    cout << "Error: verify failed on line " << line << endl;
  }

  void verify_size( size_t actual, size_t expected, int line )
  {
    if ( actual == expected ) return;
    ++err_count;
    cout << "Error: verify size failed on line " << line << endl;
    cout << " A structure with an expected sizeof() " << expected
         << " had an actual sizeof() " << actual
         << "\n This will cause common uses of <boost/endian.hpp> to fail\n";
  } 

  template <class Endian, class Base>
  void verify_value_and_ops( const Base & expected, int line )
  {
    Endian v( expected );
    verify( v == expected, line );

    Endian v2;
    v2.operator=( expected );
    verify( v2 == expected, line );

    ++v; // verify integer_cover_operators being applied to this type -
         // will fail to compile if no endian<> specialization is present
  }

  const char * big_rep    = "\x12\x34\x56\x78\x9A\xBC\xDE\xF0";
  const char * little_rep = "\xF0\xDE\xBC\x9A\x78\x56\x34\x12";

  template <class Endian>
  void verify_representation( bool is_big, int line )
  {
    int silence = 0;
    Endian x ( static_cast<typename Endian::value_type>
      (0x123456789abcdef0LL + silence) ); // will truncate

    if ( is_big )
      verify( memcmp( &x,
        reinterpret_cast<const char*>(big_rep)+8-sizeof(Endian),
          sizeof(Endian) ) == 0, line );
    else
      verify( memcmp( &x, little_rep, sizeof(Endian) ) == 0, line );
  }

  template <class Endian>
  inline void verify_native_representation( int line )
  {
#   ifdef BOOST_BIG_ENDIAN
      verify_representation<Endian>( true, line );
#   else
      verify_representation<Endian>( false, line );
#   endif
  }

  //  detect_endianness  -----------------------------------------------------//

  void detect_endianness()
  {
    union View
    { 
      long long i;
      unsigned char c[8];
    };

    View v = { 0x0102030405060708LL };  // initialize v.i
    
    if ( memcmp( v.c, "\10\7\6\5\4\3\2\1", 8) == 0 )
    {
      cout << "This machine is little-endian.\n";
  #   ifdef BOOST_BIG_INTEGER_OPERATORS
        cout << "yet boost/detail/endian.hpp defines BOOST_BIG_INTEGER_OPERATORS.\n"
          "You must fix boost/detail/endian.hpp for boost/endian.hpp to work correctly.\n"
          "Please report the fix to the Boost mailing list.\n";
        exit(1);
  #   endif
    } 
    else if ( memcmp( v.c, "\1\2\3\4\5\6\7\10", 8) == 0 )
    {
      cout << "This machine is big-endian.\n";
  #   ifdef BOOST_LITTLE_INTEGER_OPERATORS
        cout << "yet boost/detail/endian.hpp defines BOOST__LITTLE_INTEGER_OPERATORS.\n"
          "You must fix boost/detail/endian.hpp for boost/endian.hpp to work correctly.\n"
          "Please report the fix to the Boost mailing list.\n";
        exit(1);
  #   endif
    }
    else
    { 
      cout << "This machine is neither strict big-endian nor strict little-endian\n"
        "You must modify boost/endian.hpp for it to work correctly.\n";
      exit(1);
    }
    cout << "That should not matter and is presented for your information only.\n";
  } // detect_endianness

  //  check_data  ------------------------------------------------------------//

  void check_data()
  {
    big8_t big8;
    big16_t big16;
    big24_t big24;
    big32_t big32;
    big40_t big40;
    big48_t big48;
    big56_t big56;
    big64_t big64;

    ubig8_t ubig8;
    ubig16_t ubig16;
    ubig24_t ubig24;
    ubig32_t ubig32;
    ubig40_t ubig40;
    ubig48_t ubig48;
    ubig56_t ubig56;
    ubig64_t ubig64;

    little8_t little8;
    little16_t little16;
    little24_t little24;
    little32_t little32;
    little40_t little40;
    little48_t little48;
    little56_t little56;
    little64_t little64;

    ulittle8_t ulittle8;
    ulittle16_t ulittle16;
    ulittle24_t ulittle24;
    ulittle32_t ulittle32;
    ulittle40_t ulittle40;
    ulittle48_t ulittle48;
    ulittle56_t ulittle56;
    ulittle64_t ulittle64;

    native8_t native8;
    native16_t native16;
    native24_t native24;
    native32_t native32;
    native40_t native40;
    native48_t native48;
    native56_t native56;
    native64_t native64;

    unative8_t unative8;
    unative16_t unative16;
    unative24_t unative24;
    unative32_t unative32;
    unative40_t unative40;
    unative48_t unative48;
    unative56_t unative56;
    unative64_t unative64;

    aligned_big16_t  aligned_big16;
    aligned_big32_t  aligned_big32;
    aligned_big64_t  aligned_big64;

    aligned_ubig16_t aligned_ubig16;
    aligned_ubig32_t aligned_ubig32;
    aligned_ubig64_t aligned_ubig64;

    aligned_little16_t  aligned_little16;
    aligned_little32_t  aligned_little32;
    aligned_little64_t  aligned_little64;

    aligned_ulittle16_t aligned_ulittle16 ;
    aligned_ulittle32_t aligned_ulittle32 ;
    aligned_ulittle64_t aligned_ulittle64 ;

    VERIFY(big8.data() == reinterpret_cast<const char *>(&big8));
    VERIFY(big16.data() == reinterpret_cast<const char *>(&big16));
    VERIFY(big24.data() == reinterpret_cast<const char *>(&big24));
    VERIFY(big32.data() == reinterpret_cast<const char *>(&big32));
    VERIFY(big40.data() == reinterpret_cast<const char *>(&big40));
    VERIFY(big48.data() == reinterpret_cast<const char *>(&big48));
    VERIFY(big56.data() == reinterpret_cast<const char *>(&big56));
    VERIFY(big64.data() == reinterpret_cast<const char *>(&big64));

    VERIFY(ubig8.data() == reinterpret_cast<const char *>(&ubig8));
    VERIFY(ubig16.data() == reinterpret_cast<const char *>(&ubig16));
    VERIFY(ubig24.data() == reinterpret_cast<const char *>(&ubig24));
    VERIFY(ubig32.data() == reinterpret_cast<const char *>(&ubig32));
    VERIFY(ubig40.data() == reinterpret_cast<const char *>(&ubig40));
    VERIFY(ubig48.data() == reinterpret_cast<const char *>(&ubig48));
    VERIFY(ubig56.data() == reinterpret_cast<const char *>(&ubig56));
    VERIFY(ubig64.data() == reinterpret_cast<const char *>(&ubig64));

    VERIFY(little8.data() == reinterpret_cast<const char *>(&little8));
    VERIFY(little16.data() == reinterpret_cast<const char *>(&little16));
    VERIFY(little24.data() == reinterpret_cast<const char *>(&little24));
    VERIFY(little32.data() == reinterpret_cast<const char *>(&little32));
    VERIFY(little40.data() == reinterpret_cast<const char *>(&little40));
    VERIFY(little48.data() == reinterpret_cast<const char *>(&little48));
    VERIFY(little56.data() == reinterpret_cast<const char *>(&little56));
    VERIFY(little64.data() == reinterpret_cast<const char *>(&little64));

    VERIFY(ulittle8.data() == reinterpret_cast<const char *>(&ulittle8));
    VERIFY(ulittle16.data() == reinterpret_cast<const char *>(&ulittle16));
    VERIFY(ulittle24.data() == reinterpret_cast<const char *>(&ulittle24));
    VERIFY(ulittle32.data() == reinterpret_cast<const char *>(&ulittle32));
    VERIFY(ulittle40.data() == reinterpret_cast<const char *>(&ulittle40));
    VERIFY(ulittle48.data() == reinterpret_cast<const char *>(&ulittle48));
    VERIFY(ulittle56.data() == reinterpret_cast<const char *>(&ulittle56));
    VERIFY(ulittle64.data() == reinterpret_cast<const char *>(&ulittle64));

    VERIFY(native8.data() == reinterpret_cast<const char *>(&native8));
    VERIFY(native16.data() == reinterpret_cast<const char *>(&native16));
    VERIFY(native24.data() == reinterpret_cast<const char *>(&native24));
    VERIFY(native32.data() == reinterpret_cast<const char *>(&native32));
    VERIFY(native40.data() == reinterpret_cast<const char *>(&native40));
    VERIFY(native48.data() == reinterpret_cast<const char *>(&native48));
    VERIFY(native56.data() == reinterpret_cast<const char *>(&native56));
    VERIFY(native64.data() == reinterpret_cast<const char *>(&native64));

    VERIFY(unative8.data() == reinterpret_cast<const char *>(&unative8));
    VERIFY(unative16.data() == reinterpret_cast<const char *>(&unative16));
    VERIFY(unative24.data() == reinterpret_cast<const char *>(&unative24));
    VERIFY(unative32.data() == reinterpret_cast<const char *>(&unative32));
    VERIFY(unative40.data() == reinterpret_cast<const char *>(&unative40));
    VERIFY(unative48.data() == reinterpret_cast<const char *>(&unative48));
    VERIFY(unative56.data() == reinterpret_cast<const char *>(&unative56));
    VERIFY(unative64.data() == reinterpret_cast<const char *>(&unative64));

    VERIFY(aligned_big16.data() == reinterpret_cast<const char *>(&aligned_big16));
    VERIFY(aligned_big32.data() == reinterpret_cast<const char *>(&aligned_big32));
    VERIFY(aligned_big64.data() == reinterpret_cast<const char *>(&aligned_big64));

    VERIFY(aligned_ubig16.data() == reinterpret_cast<const char *>(&aligned_ubig16));
    VERIFY(aligned_ubig32.data() == reinterpret_cast<const char *>(&aligned_ubig32));
    VERIFY(aligned_ubig64.data() == reinterpret_cast<const char *>(&aligned_ubig64));

    VERIFY(aligned_little16.data() == reinterpret_cast<const char *>(&aligned_little16));
    VERIFY(aligned_little32.data() == reinterpret_cast<const char *>(&aligned_little32));
    VERIFY(aligned_little64.data() == reinterpret_cast<const char *>(&aligned_little64));

    VERIFY(aligned_ulittle16.data() == reinterpret_cast<const char *>(&aligned_ulittle16));
    VERIFY(aligned_ulittle32.data() == reinterpret_cast<const char *>(&aligned_ulittle32));
    VERIFY(aligned_ulittle64.data() == reinterpret_cast<const char *>(&aligned_ulittle64));
 
  }

  //  check_size  ------------------------------------------------------------//

  void check_size()
  {
    VERIFY( numeric_limits<signed char>::digits == 7 );
    VERIFY( numeric_limits<unsigned char>::digits == 8 );

    VERIFY_SIZE( sizeof( big8_t ), 1 );
    VERIFY_SIZE( sizeof( big16_t ), 2 );
    VERIFY_SIZE( sizeof( big24_t ), 3 );
    VERIFY_SIZE( sizeof( big32_t ), 4 );
    VERIFY_SIZE( sizeof( big40_t ), 5 );
    VERIFY_SIZE( sizeof( big48_t ), 6 );
    VERIFY_SIZE( sizeof( big56_t ), 7 );
    VERIFY_SIZE( sizeof( big64_t ), 8 );

    VERIFY_SIZE( sizeof( ubig8_t ), 1 );
    VERIFY_SIZE( sizeof( ubig16_t ), 2 );
    VERIFY_SIZE( sizeof( ubig24_t ), 3 );
    VERIFY_SIZE( sizeof( ubig32_t ), 4 );
    VERIFY_SIZE( sizeof( ubig40_t ), 5 );
    VERIFY_SIZE( sizeof( ubig48_t ), 6 );
    VERIFY_SIZE( sizeof( ubig56_t ), 7 );
    VERIFY_SIZE( sizeof( ubig64_t ), 8 );

    VERIFY_SIZE( sizeof( little8_t ), 1 );
    VERIFY_SIZE( sizeof( little16_t ), 2 );
    VERIFY_SIZE( sizeof( little24_t ), 3 );
    VERIFY_SIZE( sizeof( little32_t ), 4 );
    VERIFY_SIZE( sizeof( little40_t ), 5 );
    VERIFY_SIZE( sizeof( little48_t ), 6 );
    VERIFY_SIZE( sizeof( little56_t ), 7 );
    VERIFY_SIZE( sizeof( little64_t ), 8 );

    VERIFY_SIZE( sizeof( ulittle8_t ), 1 );
    VERIFY_SIZE( sizeof( ulittle16_t ), 2 );
    VERIFY_SIZE( sizeof( ulittle24_t ), 3 );
    VERIFY_SIZE( sizeof( ulittle32_t ), 4 );
    VERIFY_SIZE( sizeof( ulittle40_t ), 5 );
    VERIFY_SIZE( sizeof( ulittle48_t ), 6 );
    VERIFY_SIZE( sizeof( ulittle56_t ), 7 );
    VERIFY_SIZE( sizeof( ulittle64_t ), 8 );

    VERIFY_SIZE( sizeof( native8_t ), 1 );
    VERIFY_SIZE( sizeof( native16_t ), 2 );
    VERIFY_SIZE( sizeof( native24_t ), 3 );
    VERIFY_SIZE( sizeof( native32_t ), 4 );
    VERIFY_SIZE( sizeof( native40_t ), 5 );
    VERIFY_SIZE( sizeof( native48_t ), 6 );
    VERIFY_SIZE( sizeof( native56_t ), 7 );
    VERIFY_SIZE( sizeof( native64_t ), 8 );

    VERIFY_SIZE( sizeof( unative8_t ), 1 );
    VERIFY_SIZE( sizeof( unative16_t ), 2 );
    VERIFY_SIZE( sizeof( unative24_t ), 3 );
    VERIFY_SIZE( sizeof( unative32_t ), 4 );
    VERIFY_SIZE( sizeof( unative40_t ), 5 );
    VERIFY_SIZE( sizeof( unative48_t ), 6 );
    VERIFY_SIZE( sizeof( unative56_t ), 7 );
    VERIFY_SIZE( sizeof( unative64_t ), 8 );

    VERIFY_SIZE( sizeof( aligned_big16_t ), 2 );
    VERIFY_SIZE( sizeof( aligned_big32_t ), 4 );
    VERIFY_SIZE( sizeof( aligned_big64_t ), 8 );

    VERIFY_SIZE( sizeof( aligned_ubig16_t ), 2 );
    VERIFY_SIZE( sizeof( aligned_ubig32_t ), 4 );
    VERIFY_SIZE( sizeof( aligned_ubig64_t ), 8 );

    VERIFY_SIZE( sizeof( aligned_little16_t ), 2 );
    VERIFY_SIZE( sizeof( aligned_little32_t ), 4 );
    VERIFY_SIZE( sizeof( aligned_little64_t ), 8 );

    VERIFY_SIZE( sizeof( aligned_ulittle16_t ), 2 );
    VERIFY_SIZE( sizeof( aligned_ulittle32_t ), 4 );
    VERIFY_SIZE( sizeof( aligned_ulittle64_t ), 8 );
  } // check_size

  //  check_alignment  -------------------------------------------------------//

  void check_alignment()
  {
    // structs with offsets % 2 == 1 for type of size > 1 to ensure no alignment
    // bytes added for any size > 1

    struct big_struct
    {
      big8_t    v0;
      big16_t    v1;
      big24_t    v3;
      char      v6;
      big32_t    v7;
      big40_t    v11;
      char      v16;
      big48_t    v17;
      big56_t    v23;
      char      v30;
      big64_t    v31;
    };

    struct ubig_struct
    {
      ubig8_t    v0;
      ubig16_t    v1;
      ubig24_t    v3;
      char       v6;
      ubig32_t    v7;
      ubig40_t    v11;
      char       v16;
      ubig48_t    v17;
      ubig56_t    v23;
      char       v30;
      ubig64_t    v31;
    };

    struct little_struct
    {
      little8_t    v0;
      little16_t    v1;
      little24_t    v3;
      char         v6;
      little32_t    v7;
      little40_t    v11;
      char         v16;
      little48_t    v17;
      little56_t    v23;
      char         v30;
      little64_t    v31;
    };

    struct ulittle_struct
    {
      ulittle8_t    v0;
      ulittle16_t    v1;
      ulittle24_t    v3;
      char          v6;
      ulittle32_t    v7;
      ulittle40_t    v11;
      char          v16;
      ulittle48_t    v17;
      ulittle56_t    v23;
      char          v30;
      ulittle64_t    v31;
    };

    struct native_struct
    {
      native8_t    v0;
      native16_t    v1;
      native24_t    v3;
      char         v6;
      native32_t    v7;
      native40_t    v11;
      char         v16;
      native48_t    v17;
      native56_t    v23;
      char         v30;
      native64_t    v31;
    };

    struct unative_struct
    {
      unative8_t    v0;
      unative16_t    v1;
      unative24_t    v3;
      char          v6;
      unative32_t    v7;
      unative40_t    v11;
      char          v16;
      unative48_t    v17;
      unative56_t    v23;
      char          v30;
      unative64_t    v31;
    };

    int saved_err_count = err_count;

    VERIFY_SIZE( sizeof(big_struct), 39 );
    VERIFY_SIZE( sizeof(ubig_struct), 39 );
    VERIFY_SIZE( sizeof(little_struct), 39 );
    VERIFY_SIZE( sizeof(ulittle_struct), 39 );
    VERIFY_SIZE( sizeof(native_struct), 39 );
    VERIFY_SIZE( sizeof(unative_struct), 39 );

    if ( saved_err_count == err_count )
    { 
      cout <<
        "Size and alignment for structures of endian types are as expected.\n";
    }
  } // check_alignment

  //  check_representation_and_range_and_ops  --------------------------------//

  void check_representation_and_range_and_ops()
  {

    VERIFY_BIG_REPRESENTATION( big8_t );
    VERIFY_VALUE_AND_OPS( big8_t, int_least8_t,  0x7f );
    VERIFY_VALUE_AND_OPS( big8_t, int_least8_t, -0x80 );

    VERIFY_BIG_REPRESENTATION( big16_t );
    VERIFY_VALUE_AND_OPS( big16_t, int_least16_t,  0x7fff );
    VERIFY_VALUE_AND_OPS( big16_t, int_least16_t, -0x8000 );

    VERIFY_BIG_REPRESENTATION( big24_t );
    VERIFY_VALUE_AND_OPS( big24_t, int_least32_t,  0x7fffff );
    VERIFY_VALUE_AND_OPS( big24_t, int_least32_t, -0x800000 );

    VERIFY_BIG_REPRESENTATION( big32_t );
    VERIFY_VALUE_AND_OPS( big32_t, int_least32_t,  0x7fffffff );
    VERIFY_VALUE_AND_OPS( big32_t, int_least32_t, -0x7fffffff-1 );

    VERIFY_BIG_REPRESENTATION( big40_t );
    VERIFY_VALUE_AND_OPS( big40_t, int_least64_t,  0x7fffffffffLL );
    VERIFY_VALUE_AND_OPS( big40_t, int_least64_t, -0x8000000000LL );

    VERIFY_BIG_REPRESENTATION( big48_t );
    VERIFY_VALUE_AND_OPS( big48_t, int_least64_t,  0x7fffffffffffLL );
    VERIFY_VALUE_AND_OPS( big48_t, int_least64_t, -0x800000000000LL );

    VERIFY_BIG_REPRESENTATION( big56_t );
    VERIFY_VALUE_AND_OPS( big56_t, int_least64_t,  0x7fffffffffffffLL );
    VERIFY_VALUE_AND_OPS( big56_t, int_least64_t, -0x80000000000000LL );

    VERIFY_BIG_REPRESENTATION( big64_t );
    VERIFY_VALUE_AND_OPS( big64_t, int_least64_t,  0x7fffffffffffffffLL );
    VERIFY_VALUE_AND_OPS( big64_t, int_least64_t, -0x7fffffffffffffffLL-1 );

    VERIFY_BIG_REPRESENTATION( ubig8_t );
    VERIFY_VALUE_AND_OPS( ubig8_t, uint_least8_t,  0xff );

    VERIFY_BIG_REPRESENTATION( ubig16_t );
    VERIFY_VALUE_AND_OPS( ubig16_t, uint_least16_t, 0xffff );

    VERIFY_BIG_REPRESENTATION( ubig24_t );
    VERIFY_VALUE_AND_OPS( ubig24_t, uint_least32_t, 0xffffff );

    VERIFY_BIG_REPRESENTATION( ubig32_t );
    VERIFY_VALUE_AND_OPS( ubig32_t, uint_least32_t, 0xffffffff );

    VERIFY_BIG_REPRESENTATION( ubig40_t );
    VERIFY_VALUE_AND_OPS( ubig40_t, uint_least64_t, 0xffffffffffLL );

    VERIFY_BIG_REPRESENTATION( ubig48_t );
    VERIFY_VALUE_AND_OPS( ubig48_t, uint_least64_t, 0xffffffffffffLL );

    VERIFY_BIG_REPRESENTATION( ubig56_t );
    VERIFY_VALUE_AND_OPS( ubig56_t, uint_least64_t, 0xffffffffffffffLL );

    VERIFY_BIG_REPRESENTATION( ubig64_t );
    VERIFY_VALUE_AND_OPS( ubig64_t, uint_least64_t, 0xffffffffffffffffLL );

    VERIFY_LITTLE_REPRESENTATION( little8_t );
    VERIFY_VALUE_AND_OPS( little8_t, int_least8_t,   0x7f );
    VERIFY_VALUE_AND_OPS( little8_t, int_least8_t,  -0x80 );

    VERIFY_LITTLE_REPRESENTATION( little16_t );
    VERIFY_VALUE_AND_OPS( little16_t, int_least16_t,  0x7fff );
    VERIFY_VALUE_AND_OPS( little16_t, int_least16_t, -0x8000 );

    VERIFY_LITTLE_REPRESENTATION( little24_t );
    VERIFY_VALUE_AND_OPS( little24_t, int_least32_t,  0x7fffff );
    VERIFY_VALUE_AND_OPS( little24_t, int_least32_t, -0x800000 );

    VERIFY_LITTLE_REPRESENTATION( little32_t );
    VERIFY_VALUE_AND_OPS( little32_t, int_least32_t,  0x7fffffff );
    VERIFY_VALUE_AND_OPS( little32_t, int_least32_t, -0x7fffffff-1 );

    VERIFY_LITTLE_REPRESENTATION( little40_t );
    VERIFY_VALUE_AND_OPS( little40_t, int_least64_t,  0x7fffffffffLL );
    VERIFY_VALUE_AND_OPS( little40_t, int_least64_t, -0x8000000000LL );

    VERIFY_LITTLE_REPRESENTATION( little48_t );
    VERIFY_VALUE_AND_OPS( little48_t, int_least64_t,  0x7fffffffffffLL );
    VERIFY_VALUE_AND_OPS( little48_t, int_least64_t, -0x800000000000LL );

    VERIFY_LITTLE_REPRESENTATION( little56_t );
    VERIFY_VALUE_AND_OPS( little56_t, int_least64_t,  0x7fffffffffffffLL );
    VERIFY_VALUE_AND_OPS( little56_t, int_least64_t, -0x80000000000000LL );

    VERIFY_LITTLE_REPRESENTATION( little64_t );
    VERIFY_VALUE_AND_OPS( little64_t, int_least64_t,  0x7fffffffffffffffLL );
    VERIFY_VALUE_AND_OPS( little64_t, int_least64_t, -0x7fffffffffffffffLL-1 );

    VERIFY_LITTLE_REPRESENTATION( ulittle8_t );
    VERIFY_VALUE_AND_OPS( ulittle8_t, uint_least8_t, 0xff );

    VERIFY_LITTLE_REPRESENTATION( ulittle16_t );
    VERIFY_VALUE_AND_OPS( ulittle16_t, uint_least16_t, 0xffff );

    VERIFY_LITTLE_REPRESENTATION( ulittle24_t );
    VERIFY_VALUE_AND_OPS( ulittle24_t, uint_least32_t, 0xffffff );

    VERIFY_LITTLE_REPRESENTATION( ulittle32_t );
    VERIFY_VALUE_AND_OPS( ulittle32_t, uint_least32_t, 0xffffffff );

    VERIFY_LITTLE_REPRESENTATION( ulittle40_t );
    VERIFY_VALUE_AND_OPS( ulittle40_t, uint_least64_t, 0xffffffffffLL );

    VERIFY_LITTLE_REPRESENTATION( ulittle48_t );
    VERIFY_VALUE_AND_OPS( ulittle48_t, uint_least64_t, 0xffffffffffffLL );

    VERIFY_LITTLE_REPRESENTATION( ulittle56_t );
    VERIFY_VALUE_AND_OPS( ulittle56_t, uint_least64_t, 0xffffffffffffffLL );

    VERIFY_LITTLE_REPRESENTATION( ulittle64_t );
    VERIFY_VALUE_AND_OPS( ulittle64_t, uint_least64_t, 0xffffffffffffffffLL );

    VERIFY_NATIVE_REPRESENTATION( native8_t );
    VERIFY_VALUE_AND_OPS( native8_t, int_least8_t,   0x7f );
    VERIFY_VALUE_AND_OPS( native8_t, int_least8_t,  -0x80 );

    VERIFY_NATIVE_REPRESENTATION( native16_t );
    VERIFY_VALUE_AND_OPS( native16_t, int_least16_t,  0x7fff );
    VERIFY_VALUE_AND_OPS( native16_t, int_least16_t, -0x8000 );

    VERIFY_NATIVE_REPRESENTATION( native24_t );
    VERIFY_VALUE_AND_OPS( native24_t, int_least32_t,  0x7fffff );
    VERIFY_VALUE_AND_OPS( native24_t, int_least32_t, -0x800000 );

    VERIFY_NATIVE_REPRESENTATION( native32_t );
    VERIFY_VALUE_AND_OPS( native32_t, int_least32_t,  0x7fffffff );
    VERIFY_VALUE_AND_OPS( native32_t, int_least32_t, -0x7fffffff-1 );

    VERIFY_NATIVE_REPRESENTATION( native40_t );
    VERIFY_VALUE_AND_OPS( native40_t, int_least64_t,  0x7fffffffffLL );
    VERIFY_VALUE_AND_OPS( native40_t, int_least64_t, -0x8000000000LL );

    VERIFY_NATIVE_REPRESENTATION( native48_t );
    VERIFY_VALUE_AND_OPS( native48_t, int_least64_t,  0x7fffffffffffLL );
    VERIFY_VALUE_AND_OPS( native48_t, int_least64_t, -0x800000000000LL );

    VERIFY_NATIVE_REPRESENTATION( native56_t );
    VERIFY_VALUE_AND_OPS( native56_t, int_least64_t,  0x7fffffffffffffLL );
    VERIFY_VALUE_AND_OPS( native56_t, int_least64_t, -0x80000000000000LL );

    VERIFY_NATIVE_REPRESENTATION( native64_t );
    VERIFY_VALUE_AND_OPS( native64_t, int_least64_t,  0x7fffffffffffffffLL );
    VERIFY_VALUE_AND_OPS( native64_t, int_least64_t, -0x7fffffffffffffffLL-1 );

    VERIFY_NATIVE_REPRESENTATION( unative8_t );
    VERIFY_VALUE_AND_OPS( unative8_t, uint_least8_t, 0xff );

    VERIFY_NATIVE_REPRESENTATION( unative16_t );
    VERIFY_VALUE_AND_OPS( unative16_t, uint_least16_t, 0xffff );

    VERIFY_NATIVE_REPRESENTATION( unative24_t );
    VERIFY_VALUE_AND_OPS( unative24_t, uint_least32_t, 0xffffff );

    VERIFY_NATIVE_REPRESENTATION( unative32_t );
    VERIFY_VALUE_AND_OPS( unative32_t, uint_least32_t, 0xffffffff );

    VERIFY_NATIVE_REPRESENTATION( unative40_t );
    VERIFY_VALUE_AND_OPS( unative40_t, uint_least64_t, 0xffffffffffLL );

    VERIFY_NATIVE_REPRESENTATION( unative48_t );
    VERIFY_VALUE_AND_OPS( unative48_t, uint_least64_t, 0xffffffffffffLL );

    VERIFY_NATIVE_REPRESENTATION( unative56_t );
    VERIFY_VALUE_AND_OPS( unative56_t, uint_least64_t, 0xffffffffffffffLL );

    VERIFY_NATIVE_REPRESENTATION( unative64_t );
    VERIFY_VALUE_AND_OPS( unative64_t, uint_least64_t, 0xffffffffffffffffLL );

    VERIFY_BIG_REPRESENTATION( aligned_big16_t );
    VERIFY_VALUE_AND_OPS( aligned_big16_t, int_least16_t,  0x7fff );
    VERIFY_VALUE_AND_OPS( aligned_big16_t, int_least16_t, -0x8000 );

    VERIFY_BIG_REPRESENTATION( aligned_big32_t );
    VERIFY_VALUE_AND_OPS( aligned_big32_t, int_least32_t,  0x7fffffff );
    VERIFY_VALUE_AND_OPS( aligned_big32_t, int_least32_t, -0x7fffffff-1 );

    VERIFY_BIG_REPRESENTATION( aligned_big64_t );
    VERIFY_VALUE_AND_OPS( aligned_big64_t, int_least64_t,  0x7fffffffffffffffLL );
    VERIFY_VALUE_AND_OPS( aligned_big64_t, int_least64_t, -0x7fffffffffffffffLL-1 );

    VERIFY_BIG_REPRESENTATION( aligned_ubig16_t );
    VERIFY_VALUE_AND_OPS( aligned_ubig16_t, uint_least16_t, 0xffff );

    VERIFY_BIG_REPRESENTATION( aligned_ubig32_t );
    VERIFY_VALUE_AND_OPS( aligned_ubig32_t, uint_least32_t, 0xffffffff );

    VERIFY_BIG_REPRESENTATION( aligned_ubig64_t );
    VERIFY_VALUE_AND_OPS( aligned_ubig64_t, uint_least64_t, 0xffffffffffffffffLL );

    VERIFY_LITTLE_REPRESENTATION( aligned_little16_t );
    VERIFY_VALUE_AND_OPS( aligned_little16_t, int_least16_t,  0x7fff );
    VERIFY_VALUE_AND_OPS( aligned_little16_t, int_least16_t, -0x8000 );

    VERIFY_LITTLE_REPRESENTATION( aligned_little32_t );
    VERIFY_VALUE_AND_OPS( aligned_little32_t, int_least32_t,  0x7fffffff );
    VERIFY_VALUE_AND_OPS( aligned_little32_t, int_least32_t, -0x7fffffff-1 );

    VERIFY_LITTLE_REPRESENTATION( aligned_little64_t );
    VERIFY_VALUE_AND_OPS( aligned_little64_t, int_least64_t,  0x7fffffffffffffffLL );
    VERIFY_VALUE_AND_OPS( aligned_little64_t, int_least64_t, -0x7fffffffffffffffLL-1 );

    VERIFY_LITTLE_REPRESENTATION( aligned_ulittle16_t );
    VERIFY_VALUE_AND_OPS( aligned_ulittle16_t, uint_least16_t, 0xffff );

    VERIFY_LITTLE_REPRESENTATION( aligned_ulittle32_t );
    VERIFY_VALUE_AND_OPS( aligned_ulittle32_t, uint_least32_t, 0xffffffff );

    VERIFY_LITTLE_REPRESENTATION( aligned_ulittle64_t );
    VERIFY_VALUE_AND_OPS( aligned_ulittle64_t, uint_least64_t, 0xffffffffffffffffLL );

  } // check_representation_and_range

  long iterations = 10000000;
  
  template< class Endian >
  Endian timing_test( const char * s)
  {
    cout << s << " timing test, " << iterations << " iterations: ";
    progress_timer t;

    Endian v = 1;
    for ( long i = 0; i < iterations; ++i )
    {
      v += 1;
      v *= 3;
      ++v;
      v *= i;
      if ( i == 0 ) VERIFY_VALUE_AND_OPS( Endian, typename Endian::value_type, 21 );
    }
    return v;
  }

} // unnamed namespace

//  main  ------------------------------------------------------------------------------//

int main( int argc, char * argv[] )
{
  cout << "Usage: "
       << argv[0] << " [#],\n where # specifies iteration count\n"
          " default iteration count is 1000000" << endl;

  if ( argc > 1 )
    iterations = atol( argv[1] );
  if ( iterations < 1 ) iterations = 1;

  detect_endianness();
  check_size();
  check_alignment();
  check_representation_and_range_and_ops();
  check_data();

  //timing_test<big32_t> ( "big32_t" );
  //timing_test<aligned_big32_t>( "aligned_big32_t" );
  //timing_test<little32_t> ( "little32_t" );
  //timing_test<aligned_little32_t>( "aligned_little32_t" );

  cout << "\n" << err_count << " errors detected\nTest "
       << (err_count==0 ? "passed\n\n" : "failed\n\n");

  return err_count ? 1 : 0;
} // main
