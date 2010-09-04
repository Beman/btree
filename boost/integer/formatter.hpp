//  boost/integer/formatter.hpp  ---------------------------------------------//

//  Copyright Beman Dawes 2006

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  Inspired by Bjarne Stroustrup, C++ Programming Language, Sp. Ed., 21.4.6.3

#include <sstream>
#include <locale>
#include <cstdlib>
#include <cassert>
#include <boost/cstdint.hpp>
#include <boost/integer/pow.hpp>
#include <boost/io/ios_state.hpp>

//  TODO:
//
//    * Templatize on char_type so could be used with wide streams?
//    * Separate compilation, so can be used in multiple translation units.
//    * Move implementation details (bound formatters, etc.) to namespace detail.
//    * m_loc could be static? Or function returning & to nested static.
//    * Change ordering of formatter private/public.
//    * Implementation (of operator<<) should use formatter public functions.

namespace boost
{
  namespace integer
  {
    class bound_formatter;
    class ubound_formatter;

    class formatter
    {
      typedef char char_type;
      
      friend std::ostream & operator<<( std::ostream &, const bound_formatter & );
      friend std::ostream & operator<<( std::ostream &, const ubound_formatter & );

      int         m_width;      // width [to left of decimal-point];
                                //  0 for as wide as necessary
      char_type   m_fill;
      int         m_places;     // implied decimal places for fixed point
      int         m_precision;  // displayed decimal places

      std::locale m_loc;

      // Q. Why does the implementation do integer scaling rather than convert
      //    to long double and then do fixed floating point output?
      // A. The C (and thus C++) standard requires only 10 digits be preserved.
      //    On common platforms such as 32-bit x86 systems, LDBL_DIG is 15, yet
      //    19 is required for long long int's.

      // values computed by m_set:
      intmax_t    m_scale;        // fraction scaling factor; 10 to the
                                  //  m_places power
      intmax_t    m_rem_scale;    // remainder scaling factor; 10 to the
                                  //  (m_places - m_precision) power
      intmax_t    m_half_adjust;  // half adjustment for rounding

      void        m_set()
      {
        if ( m_precision > m_places ) m_precision = m_places;
        m_scale = pow( 10L, m_places );
        m_rem_scale = pow( 10L, m_places - m_precision );
        m_half_adjust = m_rem_scale / 2;
      }

    public:

      explicit formatter( int places_ = 0 )
        : m_width(0), m_fill( ' ' ), m_places(places_), m_precision(places_)
      {
        m_set();
        m_loc = std::locale( "" );
      }

      bound_formatter operator()( long long v ) const;
      bound_formatter operator()( long v ) const;
      bound_formatter operator()( int v ) const;
      bound_formatter operator()( short v ) const;
      bound_formatter operator()( signed char v ) const;

      ubound_formatter operator()( unsigned long long v ) const;
      ubound_formatter operator()( unsigned long v ) const;
      ubound_formatter operator()( unsigned int v ) const;
      ubound_formatter operator()( unsigned short v ) const;
      ubound_formatter operator()( unsigned char v ) const;

      int         width() const       { return m_width; }
      char_type   fill() const        { return m_fill; }
      int         precision() const   { return m_precision; }
      int         places() const      { return m_places; }

      formatter & width( int w )      { m_width = w; return *this; }
      formatter & fill( char_type c ) { m_fill = c; return *this; }
      formatter & precision( int p )  { m_precision = p; m_set(); return *this; }
      formatter & places( int pl, int pr = -1 )
      { 
        m_places = pl;
        if ( pr < 0 ) m_precision = m_places;
        m_set();
        return *this;
      }
    };

    class bound_formatter
    {
    public:
      const formatter &  fmt;
      long long          val;

      bound_formatter( const formatter & f, long long v ) : fmt(f), val(v) {}
    };

    inline bound_formatter formatter::operator()( long long v ) const { return bound_formatter( *this, v ); }
    inline bound_formatter formatter::operator()( long v ) const { return bound_formatter( *this, v ); }
    inline bound_formatter formatter::operator()( int v ) const { return bound_formatter( *this, v ); }
    inline bound_formatter formatter::operator()( short v ) const { return bound_formatter( *this, v ); }
    inline bound_formatter formatter::operator()( signed char v ) const { return bound_formatter( *this, v ); }

    std::ostream & operator<<( std::ostream & os, const bound_formatter & bf )
    {
      io::ios_all_saver svr( os );
      os.imbue( bf.fmt.m_loc );
      os.width( bf.fmt.m_width );
      os.fill( bf.fmt.m_fill );
      
      // some compilers don't have lldiv yet
      if ( bf.fmt.m_places )
      {
        long long quot = bf.val + (bf.val >= 0 ? bf.fmt.m_half_adjust : -bf.fmt.m_half_adjust);
        os << (quot = bf.val / bf.fmt.m_scale);

        if ( bf.fmt.m_precision )
        {
          os << '.';
          os.width( bf.fmt.m_precision );
          os.fill( '0' );
          long long rem = bf.val - (quot * bf.fmt.m_scale);
          if ( rem < 0 ) rem = -rem;
          if ( bf.fmt.m_places != bf.fmt.m_precision ) rem /= bf.fmt.m_rem_scale;
          os << rem;
        }
      }
      else os << bf.val;
      return os;
    }

    class ubound_formatter
    {
    public:
      const formatter &  fmt;
      unsigned long long val;

      ubound_formatter( const formatter & f, unsigned long long v ) : fmt(f), val(v) {}
    };

    inline ubound_formatter formatter::operator()( unsigned long long v ) const { return ubound_formatter( *this, v ); }
    inline ubound_formatter formatter::operator()( unsigned long v ) const { return ubound_formatter( *this, v ); }
    inline ubound_formatter formatter::operator()( unsigned int v ) const { return ubound_formatter( *this, v ); }
    inline ubound_formatter formatter::operator()( unsigned short v ) const { return ubound_formatter( *this, v ); }
    inline ubound_formatter formatter::operator()( unsigned char v ) const { return ubound_formatter( *this, v ); }

    std::ostream & operator<<( std::ostream & os, const ubound_formatter & bf )
    {
      io::ios_all_saver svr( os );
      os.imbue( bf.fmt.m_loc );
      os.width( bf.fmt.m_width );
      os.fill( bf.fmt.m_fill );
      
      // some compilers don't have lldiv yet
      if ( bf.fmt.m_places )
      {
        long long quot = bf.val + (bf.val >= 0 ? bf.fmt.m_half_adjust : -bf.fmt.m_half_adjust);
        os << (quot = bf.val / bf.fmt.m_scale);

        if ( bf.fmt.m_precision )
        {
          os << '.';
          os.width( bf.fmt.m_precision );
          os.fill( '0' );
          long long rem = bf.val - (quot * bf.fmt.m_scale);
          if ( rem < 0 ) rem = -rem;
          if ( bf.fmt.m_places != bf.fmt.m_precision ) rem /= bf.fmt.m_rem_scale;
          os << rem;
        }
      }
      else os << bf.val;
      return os;
    }
  } // namespace integer
} // namespace boost




