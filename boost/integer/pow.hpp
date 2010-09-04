//  boost/integer/pow.hpp  ---------------------------------------------------//

//  Copyright Beman Dawes 2006

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_INTEGER_POW_HPP
#define BOOST_INTEGER_POW_HPP

#include <cassert>

namespace boost
{
  namespace integer
  {
    template< class T >  // requirement: T is a binary integral type
    T pow( T x, int n ) // raise x to the nth power
    {
      assert( n >= 0 );  // Nick Maclaren pointed out that if n was unsigned,
                         //  pow(2,-1) = 2^4294967295

      // Knuth's Algorithm A, The Art of Computer Programming, Vol 2, 4.6.3
      if ( !n ) return 1;
      T y(1);
      while ( true )
      {
        bool odd( n & 1 );
        n >>= 1;
        if ( odd )
        {
          y *= x;
          if ( !n ) return y;
        }
        x *= x;
      }
    }
  } // namespace integer
} // namespace boost

#endif // BOOST_INTEGER_POW_HPP


