//  boost run_timer.cpp  -----------------------------------------------------//

//  Copyright Beman Dawes 1994-2006

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org/libs/system for documentation.

//----------------------------------------------------------------------------//

// define BOOST_SYSTEM_SOURCE so that <boost/system/config.hpp> knows
// the library is being built (possibly exporting rather than importing code)
#define BOOST_SYSTEM_SOURCE 

#include <boost/system/timer.hpp>
#include <boost/system/system_error.hpp>
#include <boost/io/ios_state.hpp>
#include <boost/throw_exception.hpp>
#include <boost/cerrno.hpp>
#include <cstring>
#include <cassert>

using boost::system::microsecond_t;
using boost::system::times_t;

# if defined(BOOST_WINDOWS_API)
#   include <windows.h>
# elif defined(BOOST_POSIX_API)
#   include <sys/times.h>
# else
# error unknown API
# endif

namespace
{
  const char * default_format =
    " %ws wall, %us user + %ss system = %ts cpu (%p%)\n";

# if defined(BOOST_POSIX_API)
  long tick_factor()        // multiplier to convert ticks
                             //  to microseconds; -1 if unknown
  {
    static long tick_factor = 0;
    if ( !tick_factor )
    {
      if ( (tick_factor = ::sysconf( _SC_CLK_TCK )) <= 0 )
        tick_factor = -1;
      else
      {
        assert( tick_factor <= 1000000L ); // doesn't handle large ticks
        tick_factor = 1000000L / tick_factor;  // compute factor
        if ( !tick_factor ) tick_factor = -1;
      }
    }
    return tick_factor;
  }
# endif

void show_time( const char * format, int places, std::ostream & os,
    const times_t & times )
  //  NOTE WELL: Will truncate least-significant digits to LDBL_DIG, which may
  //  be as low as 10, although will be 15 for many common platforms.
  {
    if ( times.wall < microsecond_t(0) ) return;
    if ( places > 6 ) places = 6;
    else if ( places < 0 ) places = 0;

    boost::io::ios_flags_saver ifs( os );
    boost::io::ios_precision_saver ips( os );
    os.setf( std::ios_base::fixed, std::ios_base::floatfield );
    os.precision( places );

    const long double sec = 1000000.0L;
    microsecond_t total = times.system + times.user;

    for ( ; *format; ++format )
    {
      if ( *format != '%' || !*(format+1) || !std::strchr("wustp", *(format+1)) )
        os << *format;
      else
      {
        ++format;
        switch ( *format )
        {
        case 'w':
          os << times.wall / sec;
          break;
        case 'u':
          os << times.user / sec;
          break;
        case 's':
          os << times.system / sec;
          break;
        case 't':
          os << total / sec;
          break;
        case 'p':
          os.precision( 1 );
           if ( times.wall && total )
             os << static_cast<long double>(total) /times. wall * 100.0;
           else
             os << 0.0;
          os.precision( places );
          break;
        default:
          assert(0);
        }
      }
    }
  }

}  // unnamed namespace

namespace boost
{
  namespace system
  {
    //  run_timer:: report  --------------------------------------//

    void run_timer::report()
    {
      show_time( !m_format 
          ? default_format
          : m_format,
        m_places, m_os, this->stop() );
    }

    error_code run_timer::report( error_code & ec )
    {
      try
      {
        report();
        ec = error_code();
      }

      catch (...) // eat any exceptions
      {
        ec = error_code( EIO, generic_category() );
      } 
      
      return ec;
    }

  } // namespace system
} // namespace boost
