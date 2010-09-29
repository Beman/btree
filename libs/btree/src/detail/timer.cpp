//  boost timer.cpp  ---------------------------------------------------------//

//  Copyright Beman Dawes 1994-2006

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org/libs/system for documentation.

//----------------------------------------------------------------------------//

// define BOOST_BTREE_SOURCE so that <boost/system/config.hpp> knows
// the library is being built (possibly exporting rather than importing code)
#define BOOST_BTREE_SOURCE 

#include <boost/btree/detail/timer.hpp>
#include <boost/system/system_error.hpp>
#include <boost/io/ios_state.hpp>
#include <boost/throw_exception.hpp>
#include <boost/cerrno.hpp>
#include <cstring>
#include <cassert>

# if defined(BOOST_WINDOWS_API)
#   include <windows.h>
# elif defined(BOOST_POSIX_API)
#   include <unistd.h>
#   include <sys/times.h>
# else
# error unknown API
# endif

using boost::system::error_code;

# if defined(BOOST_POSIX_API)
namespace
{
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
} // unnamed namespace
# endif

namespace boost
{
  namespace btree
  {

    BOOST_BTREE_DECL
    void times( times_t & current )
    {
      error_code ec;
      if ( times( current, ec ) )
        boost::throw_exception( system::system_error( ec, "boost::btree::times" ) );
    }

    BOOST_BTREE_DECL
    error_code & times( times_t & current, error_code & ec )
    {
      ec = error_code();
#   if defined(BOOST_WINDOWS_API)
      ::GetSystemTimeAsFileTime( (LPFILETIME)&current.wall );
      FILETIME creation, exit;
      if ( ::GetProcessTimes( ::GetCurrentProcess(), &creation, &exit,
             (LPFILETIME)&current.system, (LPFILETIME)&current.user ) )
      {
        current.wall   /= 10;  // Windows uses 100 nanosecond ticks
        current.user   /= 10;
        current.system /= 10;
      }
      else
      {
        ec = error_code( ::GetLastError(), system::system_category() );
        current.wall = current.system = current.user = microsecond_t(-1);
      }
#   else
      tms tm;
      clock_t c = ::times( &tm );
      if ( c == -1 ) // error
      {
        ec = error_code( errno, system::system_category() );
        current.wall = current.system = current.user = microsecond_t(-1);
      }
      else
      {
        current.wall = microsecond_t(c);
        current.system = microsecond_t(tm.tms_stime + tm.tms_cstime);
        current.user = microsecond_t(tm.tms_utime + tm.tms_cutime);
        if ( tick_factor() != -1 )
        {
          current.wall *= tick_factor();
          current.user *= tick_factor();
          current.system *= tick_factor();
        }
        else
        {
          ec = error_code( errno, system::system_category() );
          current.wall = current.user = current.system = microsecond_t(-1);
        }
      }
#   endif
      return ec;
    }

#define  BOOST_TIMES(C)            \
      if ( m_flags & m_nothrow )   \
      {                            \
        error_code ec;             \
        times( C, ec );            \
      }                            \
      else                         \
        times( C );

    //  timer  ---------------------------------------------------------------//

    void timer::start()
    {
      m_flags = static_cast<m_flags_t>(m_flags & ~m_stopped);
      BOOST_TIMES( m_times );
    }

    const times_t & timer::stop()
    {
      if ( stopped() ) return m_times;
      m_flags = static_cast<m_flags_t>(m_flags | m_stopped);
      
      times_t current;
      BOOST_TIMES( current );
      m_times.wall = (current.wall - m_times.wall);
      m_times.user = (current.user - m_times.user);
      m_times.system = (current.system - m_times.system);
      return m_times;
    }

    void timer::elapsed( times_t & current )
    {
      if ( stopped() )
      {
        current.wall = m_times.wall;
        current.user = m_times.user;
        current.system = m_times.system;
      }
      else
      {
        BOOST_TIMES( current );
        current.wall -= m_times.wall;
        current.user -= m_times.user;
        current.system -= m_times.system;
      }
    }

  } // namespace btree
} // namespace boost
