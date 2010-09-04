//  error_code support implementation file  ----------------------------------//

//  Copyright Beman Dawes 2002, 2006

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

// define BOOST_SYSTEM_SOURCE so that <boost/system/config.hpp> knows
// the library is being built (possibly exporting rather than importing code)
#define BOOST_SYSTEM_SOURCE 

#include <boost/system/config.hpp>
#include <boost/system/error_code.hpp>
#include <boost/cerrno.hpp>
#include <vector>

using namespace boost::system;

#include <cstring> // for strerror/strerror_r

# ifdef BOOST_NO_STDC_NAMESPACE
    namespace std { using ::strerror; }
# endif

# if defined( BOOST_WINDOWS_API )
#   include "windows.h"
#   ifndef ERROR_INCORRECT_SIZE
#    define ERROR_INCORRECT_SIZE ERROR_BAD_ARGUMENTS
#   endif
# endif

//----------------------------------------------------------------------------//

namespace
{

  //  Windows native -> errno decode table  ----------------------------------//  

#ifdef BOOST_WINDOWS_API
  struct native_to_errno_t
  { 
    boost::int32_t native_value;
    int to_errno;
  };

  const native_to_errno_t native_to_errno[] = 
  {
    // see WinError.h comments for descriptions of errors
    
    // most common errors first to speed sequential search
    { ERROR_FILE_NOT_FOUND, ENOENT },
    { ERROR_PATH_NOT_FOUND, ENOENT },

    // rest are alphabetical for easy maintenance
    { 0, 0 }, // no error 
    { ERROR_ACCESS_DENIED, EACCES },
    { ERROR_ALREADY_EXISTS, EEXIST },
    { ERROR_BAD_UNIT, ENODEV },
    { ERROR_BUFFER_OVERFLOW, ENAMETOOLONG },
    { ERROR_BUSY, EBUSY },
    { ERROR_BUSY_DRIVE, EBUSY },
    { ERROR_CANNOT_MAKE, EACCES },
    { ERROR_CANTOPEN, EIO },
    { ERROR_CANTREAD, EIO },
    { ERROR_CANTWRITE, EIO },
    { ERROR_CURRENT_DIRECTORY, EACCES },
    { ERROR_DEV_NOT_EXIST, ENODEV },
    { ERROR_DEVICE_IN_USE, EBUSY },
    { ERROR_DIR_NOT_EMPTY, ENOTEMPTY },
    { ERROR_DIRECTORY, EINVAL }, // WinError.h: "The directory name is invalid"
    { ERROR_DISK_FULL, ENOSPC },
    { ERROR_FILE_EXISTS, EEXIST },
    { ERROR_HANDLE_DISK_FULL, ENOSPC },
    { ERROR_INVALID_ACCESS, EACCES },
    { ERROR_INVALID_DRIVE, ENODEV },
    { ERROR_INVALID_FUNCTION, ENOSYS },
    { ERROR_INVALID_HANDLE, EBADHANDLE },
    { ERROR_INVALID_NAME, EINVAL },
    { ERROR_LOCK_VIOLATION, EACCES },
    { ERROR_LOCKED, EACCES },
    { ERROR_NEGATIVE_SEEK, EINVAL },
    { ERROR_NOACCESS, EACCES },
    { ERROR_NOT_ENOUGH_MEMORY, ENOMEM },
    { ERROR_NOT_READY, EAGAIN },
    { ERROR_NOT_SAME_DEVICE, EXDEV },
    { ERROR_OPEN_FAILED, EIO },
    { ERROR_OPEN_FILES, EBUSY },
    { ERROR_OUTOFMEMORY, ENOMEM },
    { ERROR_READ_FAULT, EIO },
    { ERROR_SEEK, EIO },
    { ERROR_SHARING_VIOLATION, EACCES },
    { ERROR_TOO_MANY_OPEN_FILES, ENFILE },
    { ERROR_WRITE_FAULT, EIO },
    { ERROR_WRITE_PROTECT, EROFS }
  };

#endif

} // unnamed namespace

namespace boost
{
  namespace system
  {
    //  standard error categories  -------------------------------------------//

    class BOOST_SYSTEM_DECL errno_error_category : public error_category
    {
    public:
      const std::string & name() const;
      int                 to_errno( boost::int_least32_t ev ) const;
      std::string         message( boost::int_least32_t ev ) const;
      wstring_t           wmessage( boost::int_least32_t ev ) const;
    };
  
# ifdef BOOST_POSIX_API
    typedef errno_error_category native_error_category;
# else
    class BOOST_SYSTEM_DECL native_error_category  : public error_category
    {
    public:
      const std::string & name() const;
      int                 to_errno( boost::int_least32_t ev ) const;
      std::string         message( boost::int_least32_t ev ) const;
      wstring_t           wmessage( boost::int_least32_t ev ) const;
    };
# endif

    const errno_error_category errno_ecat_const;
    BOOST_SYSTEM_DECL const error_category & errno_ecat = errno_ecat_const;

    const native_error_category native_ecat_const;
    BOOST_SYSTEM_DECL const error_category & native_ecat = native_ecat_const;

    //  errno_error_category implementation  ---------------------------------//

    const std::string & errno_error_category::name() const
    {
      static std::string s( "errno" );
      return s;
    }

    int errno_error_category::to_errno( boost::int_least32_t ev ) const
    {
      return ev;
    }

    std::string errno_error_category::message( boost::int_least32_t ev ) const
    {
    // strerror_r is preferred because it is always thread safe,
    // however, we fallback to strerror in certain cases because:
    //   -- Windows doesn't provide strerror_r.
    //   -- HP and Sundo provide strerror_r on newer systems, but there is
    //      no way to tell if is available at runtime and in any case their
    //      versions of strerror are thread safe anyhow.
    //   -- Linux only sometimes provides strerror_r.
  # if defined(BOOST_WINDOWS_API) || defined(__hpux) || defined(__sun)\
       || (defined(__linux) && (!defined(__USE_XOPEN2K) || defined(BOOST_SYSTEM_USE_STRERROR)))
      const char * c_str = std::strerror( ev );
      return std::string( c_str ? c_str : "EINVAL" );
  # else
      char buf[64];
      char * bp = buf;
      std::size_t sz = sizeof(buf);
  #  if defined(__CYGWIN__) || defined(__USE_GNU)
      // Oddball version of strerror_r
      const char * c_str = strerror_r( ev, bp, sz );
      return std::string( c_str ? c_str : "EINVAL" );
  #  else
      // POSIX version of strerror_r
      int result;
      for (;;)
      {
        // strerror_r returns 0 on success, otherwise ERANGE if buffer too small,
        // EINVAL if ev not a valid error number
        if ( (result = strerror_r( ev, bp, sz )) == 0 )
          break;
        else
        {
  #  if defined(__linux)
          // Linux strerror_r returns -1 on error, with error number in errno
          result = errno;
  #  endif
          if ( result !=  ERANGE ) break;
        if ( sz > sizeof(buf) ) std::free( bp );
        sz *= 2;
        if ( (bp = static_cast<char*>(std::malloc( sz ))) == 0 )
          return std::string( "ENOMEM" );
        }
      }
      try
      {
      std::string msg( ( result == EINVAL ) ? "EINVAL" : bp );
      if ( sz > sizeof(buf) ) std::free( bp );
        sz = 0;
      return msg;
      }
      catch(...)
      {
        if ( sz > sizeof(buf) ) std::free( bp );
        throw;
      }
  #  endif
  # endif
    }

    wstring_t errno_error_category::wmessage( boost::int_least32_t ev ) const
    {
      std::string str = message( ev );
      wstring_t wstr;

      for (std::size_t i = 0; i < str.size(); ++i )
        { wstr += static_cast<wchar_t>(str[i]); }
      return wstr;
    }

    //  native_error_category implementation  --------------------------------// 

    const std::string & native_error_category::name() const
    {
      static std::string s( "native" );
      return s;
    }

# if !defined( BOOST_WINDOWS_API )
    int native_error_category::to_errno( boost::int_least32_t ev ) const
    {
      return ev;
    }

    std::string native_error_category::message( boost::int_least32_t ev ) const
    {
      return errno_ecat.message( ev );
    }

    wstring_t native_error_category::wmessage( boost::int_least32_t ev ) const
    {
      return errno_ecat.wmessage( ev );
    }
# else
    int native_error_category::to_errno( boost::int_least32_t ev ) const
    {
      const native_to_errno_t * cur = native_to_errno;
      do
      {
        if ( ev == cur->native_value ) return cur->to_errno;
        ++cur;
      } while ( cur != native_to_errno
        + sizeof(native_to_errno)/sizeof(native_to_errno_t) );
      return EOTHER;
    }

// TODO:
  
//Some quick notes on the implementation (sorry for the noise if
//someone has already mentioned them):
//
//- The ::LocalFree() usage isn't exception safe.
//
//See:
//
//<http://boost.cvs.sourceforge.net/boost/boost/boost/asio/system_exception.hpp?revision=1.1&view=markup>
//
//in the implementation of what() for an example.
//
//Cheers,
//Chris
    std::string native_error_category::message( boost::int_least32_t ev ) const
    {
      LPVOID lpMsgBuf;
      ::FormatMessageA( 
          FORMAT_MESSAGE_ALLOCATE_BUFFER | 
          FORMAT_MESSAGE_FROM_SYSTEM | 
          FORMAT_MESSAGE_IGNORE_INSERTS,
          NULL,
          ev,
          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
          (LPSTR) &lpMsgBuf,
          0,
          NULL 
      );
      std::string str( static_cast<LPCSTR>(lpMsgBuf) );
      ::LocalFree( lpMsgBuf ); // free the buffer
      while ( str.size()
        && (str[str.size()-1] == '\n' || str[str.size()-1] == '\r') )
          str.erase( str.size()-1 );
      return str;
    }

    wstring_t native_error_category::wmessage( boost::int_least32_t ev ) const
    {
      LPVOID lpMsgBuf;
      ::FormatMessageW( 
          FORMAT_MESSAGE_ALLOCATE_BUFFER | 
          FORMAT_MESSAGE_FROM_SYSTEM | 
          FORMAT_MESSAGE_IGNORE_INSERTS,
          NULL,
          ev,
          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
          (LPWSTR) &lpMsgBuf,
          0,
          NULL 
      );
      wstring_t str( static_cast<LPCWSTR>(lpMsgBuf) );
      ::LocalFree( lpMsgBuf ); // free the buffer
      while ( str.size()
        && (str[str.size()-1] == L'\n' || str[str.size()-1] == L'\r') )
          str.erase( str.size()-1 );
      return str;
    }
# endif
  } // namespace system
} // namespace boost
