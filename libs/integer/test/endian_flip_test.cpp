//  endian_flip_test.cpp  --------------------------------------------------------------//

//  Copyright Beman Dawes 2010

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//--------------------------------------------------------------------------------------//

#include <boost/integer/endian_flip.hpp>
#include <boost/detail/lightweight_test.hpp>

namespace bi = boost::integer;

int main()
{
  boost::int64_t i64 = 0x0102030405060708;
  bi::endian_flip(i64);
  BOOST_TEST_EQ(i64, 0x0807060504030201);
  bi::endian_flip(i64);
  BOOST_TEST_EQ(i64, 0x0102030405060708);

  i64 = 0xfefdfcfbfaf9f8f7;
  bi::endian_flip(i64);
  BOOST_TEST_EQ(i64, 0xf7f8f9fafbfcfdfe);
  bi::endian_flip(i64);
  BOOST_TEST_EQ(i64, 0xfefdfcfbfaf9f8f7);

  boost::int32_t i32 = 0x01020304;
  bi::endian_flip(i32);
  BOOST_TEST_EQ(i32, 0x04030201);
  bi::endian_flip(i32);
  BOOST_TEST_EQ(i32, 0x01020304);

  i32 = 0xfefdfcfb;
  bi::endian_flip(i32);
  BOOST_TEST_EQ(i32, 0xfbfcfdfe);
  bi::endian_flip(i32);
  BOOST_TEST_EQ(i32, 0xfefdfcfb);

  boost::int16_t i16 = 0x0102;
  bi::endian_flip(i16);
  BOOST_TEST_EQ(i16, 0x0201);
  bi::endian_flip(i16);
  BOOST_TEST_EQ(i16, 0x0102);

  i16 = (boost::int16_t)0xfefd;
  bi::endian_flip(i16);
  BOOST_TEST_EQ(i16, (boost::int16_t)0xfdfe);
  bi::endian_flip(i16);
  BOOST_TEST_EQ(i16, (boost::int16_t)0xfefd);

  boost::uint64_t ui64 = 0x0102030405060708;
  bi::endian_flip(ui64);
  BOOST_TEST_EQ(ui64, 0x0807060504030201);
  bi::endian_flip(ui64);
  BOOST_TEST_EQ(ui64, 0x0102030405060708);

  boost::uint32_t ui32 = 0x01020304;
  bi::endian_flip(ui32);
  BOOST_TEST_EQ(ui32, 0x04030201);
  bi::endian_flip(ui32);
  BOOST_TEST_EQ(ui32, 0x01020304);

  boost::uint16_t ui16 = 0x0102;
  bi::endian_flip(ui16);
  BOOST_TEST_EQ(ui16, 0x0201);
  bi::endian_flip(ui16);
  BOOST_TEST_EQ(ui16, 0x0102);


  return ::boost::report_errors();
}
