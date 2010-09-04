//  boost/bitmask.hpp  -------------------------------------------------------//

//  Copyright Beman Dawes 2006

//  Distributed under the Boost Software License, Version 1.0
//  http://www.boost.org/LICENSE_1_0.txt

//  See http://www.boost.org/libs/utility for documentation.

#ifndef BOOST_BITMASK_HPP
#define BOOST_BITMASK_HPP

#include <boost/cstdint.hpp>

#define BOOST_BITMASK(BitmaskType)                                            \
                                                                              \
  inline BitmaskType operator| (BitmaskType x , BitmaskType y )               \
  { return static_cast<BitmaskType>( static_cast<boost::int_least32_t>(x)     \
      | static_cast<boost::int_least32_t>(y)); }                              \
                                                                              \
  inline BitmaskType operator& (BitmaskType x , BitmaskType y )               \
  { return static_cast<BitmaskType>( static_cast<boost::int_least32_t>(x)     \
      & static_cast<boost::int_least32_t>(y)); }                              \
                                                                              \
  inline BitmaskType operator^ (BitmaskType x , BitmaskType y )               \
  { return static_cast<BitmaskType >( static_cast<boost::int_least32_t>(x)    \
      ^ static_cast<boost::int_least32_t>(y)); }                              \
                                                                              \
  inline BitmaskType operator~ (BitmaskType x )                               \
  { return static_cast<BitmaskType >(~static_cast<boost::int_least32_t>(x)); }\
                                                                              \
  inline BitmaskType & operator&=(BitmaskType & x , BitmaskType y)            \
  { x = x & y ; return x ; }                                                  \
                                                                              \
  inline BitmaskType & operator|=(BitmaskType & x , BitmaskType y)            \
  { x = x | y ; return x ; }                                                  \
                                                                              \
  inline BitmaskType & operator^=(BitmaskType & x , BitmaskType y)            \
  { x = x ^ y ; return x ; }                                                  \

#endif // BOOST_BITMASK_HPP
