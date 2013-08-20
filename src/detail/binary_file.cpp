//  binary_file.cpp  -------------------------------------------------------------------//

//  Copyright 2006 Beman Dawes

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//--------------------------------------------------------------------------------------// 

#include <boost/config/warning_disable.hpp>

// define BOOST_BTREE_SOURCE so that <boost/btree/detail/config.hpp> knows
// the library is being built (possibly exporting rather than importing code)
#define BOOST_BTREE_SOURCE 

#define _LARGEFILE64_SOURCE

#include <boost/btree/detail/binary_file.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/system/error_code.hpp>
#include <boost/assert.hpp>

#include <string>
#include <cstring>
#include <cerrno>

using boost::system::error_code;
using boost::system::system_category;

# if defined(BOOST_WINDOWS_API)
#   include "windows.h"
#   define _CRT_NONSTDC_NO_DEPRECATE
#   include "sys/stat.h"
#   include "fcntl.h"
#   include "io.h"

# else // BOOST_POSIX_API
#   include<boost/iostreams/detail/config/rtl.hpp>  // for BOOST_IOSTREAMS_FD_SEEK,
                                                    //  BOOST_IOSTREAMS_FD_OFFSET 
#   include <sys/types.h>
#   include "unistd.h"
#   include "fcntl.h"
# endif
// #include <iostream>    // for debugging only; comment out when not in use

namespace
{
  const size_t BUF_SIZE = 32768;
  void preloader(const boost::filesystem::path& p)
  {
  //  preload is just a hint, so ignore errors

  int f = ::open(p.string().c_str(), O_RDONLY
# ifdef O_BINARY
                                     | O_BINARY
# endif
# ifdef O_SEQUENTIAL
                                     | O_SEQUENTIAL
# endif
                                     , 0);
  if ( f == -1 )
    return;
  char* buf = new char [BUF_SIZE];
  while (::read(f, buf, BUF_SIZE) == static_cast<int>(BUF_SIZE))
    {}
  delete [] buf;
  ::close(f);
  }
}

namespace boost
{
  namespace btree
  {
# ifdef BOOST_WINDOWS_API
    BOOST_BTREE_DECL const binary_file::handle_type binary_file::invalid_handle
      = reinterpret_cast<binary_file::handle_type>(-1);
#   endif

//  -----------------------------------  open  ----------------------------------------  //

    const oflag::bitmask omask(oflag::in | oflag::out | oflag::truncate);

    bool binary_file::open(const boost::filesystem::path& p,
      oflag::bitmask flags, system::error_code& ec)
      // Returns: true if successful.
    {
      BOOST_ASSERT(!is_open());
//std::cout << "*** open " << p.string() << std::endl;
      m_path = p;

      if ((flags & oflag::preload) && !(flags & oflag::truncate))
        preloader(p);

#   ifdef BOOST_WINDOWS_API
      DWORD desired_access(0);
      
      if ((flags & oflag::in) || ((flags & (oflag::in|oflag::out)) == 0))
        desired_access |= GENERIC_READ;
      if (flags & oflag::out)
        desired_access |= GENERIC_WRITE;
 
      DWORD creation_disposition(0);

      if ((flags & omask) == oflag::in)
        creation_disposition = OPEN_EXISTING;  // fail if does not exist
      else if ((flags & omask) == oflag::out)
        creation_disposition = CREATE_NEW;     // fail if exists
      else if ((flags & omask) == (oflag::out | oflag::truncate))
        creation_disposition = CREATE_ALWAYS;  // truncate if exists, otherwise create
      else if ((flags & omask) == (oflag::in | oflag::out))
        creation_disposition = OPEN_ALWAYS;    // create if does not exist
      else if ((flags & omask) == (oflag::in | oflag::out  | oflag::truncate))
        creation_disposition = CREATE_ALWAYS;  // truncate if exists, otherwise create
      else
      {
        ec.assign(EINVAL, system::generic_category());
        return false;
      }

       DWORD flags_and_attributes((flags & oflag::out) != 0
        ? FILE_ATTRIBUTE_ARCHIVE : FILE_ATTRIBUTE_NORMAL);
      if ((flags & oflag::random) != 0)
        flags_and_attributes |= FILE_FLAG_RANDOM_ACCESS;
      if ((flags & oflag::sequential) != 0)
        flags_and_attributes |= FILE_FLAG_SEQUENTIAL_SCAN;

      HANDLE h (::CreateFileW(p.c_str(), desired_access,
        FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
        creation_disposition, flags_and_attributes, 0));

      if (h == INVALID_HANDLE_VALUE)
      {
        ec.assign(::GetLastError(), system_category());
        return false;
      }
      m_handle = h;

#   else  // BOOST_POSIX_API
      int openflag;

      if ((flags & oflag::in) && (flags & oflag::out))
        openflag = O_RDWR;
      else if (flags & oflag::out)
        openflag = O_WRONLY;
      else
        openflag = O_RDONLY;

      if (flags & oflag::out)
        openflag |= O_CREAT;

      if (flags & oflag::truncate)
        openflag |= O_TRUNC;

      ::mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

      m_handle = ::open(p.c_str(), openflag, mode);

      if (m_handle < 0)
      {
        ec.assign(errno, system_category());
        return false;
      }
#   endif

      ec.clear();
      if (flags & oflag::seek_end)
      {
        seek(0, seekdir::end, ec);
        if (ec)
          return false;
      }
      return true;
    }

    void binary_file::open(const boost::filesystem::path& p, oflag::bitmask flags)
    {
      error_code ec;
      open(p, flags, ec);
      if (ec)
        BOOST_BTREE_THROW(filesystem::filesystem_error("binary_file::open",
          path(), ec));
    }

//  -----------------------------------  close  ----------------------------------------  //

    bool binary_file::close(system::error_code& ec)
    {
      ec.clear();

#   ifdef BOOST_WINDOWS_API
      if (m_handle == INVALID_HANDLE_VALUE)
        return false;
//std::cout << "*** close " << m_path.string() << std::endl;
      bool ok (::CloseHandle(m_handle) != 0);
      m_handle = INVALID_HANDLE_VALUE;
      if (ok)
        return true;
      ec.assign(::GetLastError(), system_category());
      return false;

#   else  // BOOST_POSIX_API
      if (m_handle < 0)
        return false;
      bool ok (::close(m_handle) == 0);
      m_handle = -1;
      if (ok)
        return true;
      ec.assign(errno, system_category());
      return false;

#   endif
    }

    void binary_file::close()
    {
      error_code ec;
      close(ec);
      if (ec)
        BOOST_BTREE_THROW(filesystem::filesystem_error("binary_file::close",
          path(), ec));
    }

//  --------------------------------  destructor  ------------------------------------  //

    binary_file::~binary_file()
    {
      if (is_open())
      {
        error_code ec;
        close(ec);
      }
    }

//  -----------------------------------  read  ---------------------------------------  //

    std::size_t
    binary_file::raw_read(void* target, std::size_t sz, system::error_code& ec)
    {
      BOOST_ASSERT(is_open());
//std::cout << "*** raw_read " << m_path.string()
//  << " into " << target << " size " << sz << std::endl;
#   ifdef BOOST_WINDOWS_API
      DWORD sz_read;
      if (!::ReadFile(handle(), target, DWORD(sz), &sz_read, 0))
      {
        ec.assign(::GetLastError(), system_category());
        return -1;
      }
      else
        ec.clear();
      return sz_read;

#   else  // BOOST_POSIX_API
      std::size_t sz_read = ::read(handle(), target, sz);
      if (sz_read == static_cast<std::size_t>(-1))
        ec.assign(errno, system_category());
      else
        ec.clear();
      return sz_read;

#   endif
    }

    std::size_t binary_file::raw_read(void* target, std::size_t sz)
    {
      BOOST_ASSERT(is_open());
      error_code ec;
      std::size_t sz_read(raw_read(target, sz, ec));
      if (ec)
        BOOST_BTREE_THROW(filesystem::filesystem_error("binary_file::raw_read",
          path(), ec));
      return sz_read;
    }

    bool
    binary_file::m_read(void* target, std::size_t sz, system::error_code& ec)
    {
      BOOST_ASSERT(is_open());
//std::cout << "*** read " << m_path.string()
//  << " into " << target << " size " << sz << std::endl;
#   ifdef BOOST_WINDOWS_API
      std::size_t sz_read = raw_read(target, sz, ec);
      // It isn't clear to me from the Platform SDK docs if a partial read
      // in the POSIX sense can occur. For now, consider a partial read an
      // error.
      if (sz_read != 0 && sz_read != sz)
      {
        ec = error_code(ERROR_READ_FAULT, system_category());
        return false;
      }
      return sz_read != 0;

#   else  // BOOST_POSIX_API
      //  Allow for partial reads
      ssize_t sz_read=0;
      ssize_t sz_to_read=sz;
      do
      {
        sz_read = ::read(handle(), target, sz_to_read);
        if (sz_read < 0)
        {
          ec.assign(errno, system_category());
          return false;
        }
        if (sz_read == 0)
        {
          if (sz_to_read == static_cast<ssize_t>(sz)) // no bytes read, so it is a normal eof
            ec.clear();
          else  // premature eof
            ec.assign(EIO, system_category());
          return false;
        }
        target = static_cast<char*>(target) + sz_read;
        sz_to_read -= sz_read;

      } while (sz_to_read); // more to read

      ec.clear();
      return true;

#   endif
    }

    bool binary_file::m_read(void* target, std::size_t sz)
    {
      BOOST_ASSERT(is_open());
      error_code ec;
      bool result(m_read(target, sz, ec));
      if (ec)
        BOOST_BTREE_THROW(filesystem::filesystem_error("binary_file::read",
          path(), ec));
      return result;
    }

//  -----------------------------------  write  --------------------------------------  //

    std::size_t
    binary_file::raw_write(const void* source, std::size_t sz, system::error_code& ec)
    {
      BOOST_ASSERT(is_open());
//std::cout << "*** raw_write " << m_path.string()
//  << " from " << source << " size " << sz << std::endl;
#   ifdef BOOST_WINDOWS_API
      DWORD sz_written;
      if (!::WriteFile(handle(), source, DWORD(sz), &sz_written, 0))
      {
        ec.assign(::GetLastError(), system_category());
        return -1;
      }
      ec.clear();
      return sz_written;

#   else  // BOOST_POSIX_API
      ssize_t sz_written = ::write(handle(), source, sz);
      if (sz_written < 0)
        ec.assign(errno, system_category());
      else
        ec.clear();
      return sz_written;

#   endif
    }

    std::size_t binary_file::raw_write(const void* source, std::size_t sz)
    {
      BOOST_ASSERT(is_open());
      error_code ec;
      std::size_t sz_written(raw_write(source, sz, ec));
      if (ec)
        BOOST_BTREE_THROW(filesystem::filesystem_error("binary_file::raw_write", path(), ec));
      return sz_written;
    }

    void binary_file::m_write(const void* source, std::size_t sz, system::error_code& ec)
    {
      BOOST_ASSERT(is_open());
//std::cout << "*** write " << m_path.string()
//  << " from " << source << " size " << sz << std::endl;
#   ifdef BOOST_WINDOWS_API
      DWORD sz_written;
      if (!::WriteFile(handle(), source, DWORD(sz), &sz_written, 0))
        ec.assign(::GetLastError(), system_category());
      // It isn't clear to me from the Platform SDK docs if a partial write
      // in the POSIX sense may occur. For now, consider a partial write an
      // error.
      else if (sz_written != sz)
        ec.assign(ERROR_WRITE_FAULT, system_category());
      else
        ec.clear();

#   else // BOOST_POSIX_API
      // Allow for partial writes - see Advanced Unix Programming (2nd Ed.),
      // Marc Rochkind, Addison-Wesley, 2004, page 94
      ssize_t sz_write = 0;
      ssize_t sz_written = 0;
      do
      {
        if ((sz_write = ::write(handle(), static_cast<const char*>(source) + sz_written,
          sz - sz_written)) < 0)
        {
          ec.assign(errno, system_category());
          return;
        }
        sz_written += sz_write;
      } while (sz_written < static_cast<ssize_t>(sz));
      ec.clear();

#   endif
    }

    void binary_file::m_write(const void* source, std::size_t sz)
    {
      error_code ec;
      m_write(source, sz, ec);
      if (ec)
        BOOST_BTREE_THROW(filesystem::filesystem_error("binary_file::write", path(), ec));
    }

//  -----------------------------------  seek  ---------------------------------------  //

    binary_file::offset_type
    binary_file::seek(offset_type offset, seekdir::pos from, system::error_code& ec)
    {
      BOOST_ASSERT(is_open());
//std::cout << "*** seek " << m_path.string() << " offset " << offset << std::endl;

#   ifdef BOOST_WINDOWS_API
      DWORD mv_method;
      if (from == seekdir::end)
        mv_method = FILE_END;
      else if (from == seekdir::current)
        mv_method = FILE_CURRENT;
      else
        mv_method = FILE_BEGIN;

      LARGE_INTEGER move_off;
      move_off.QuadPart = offset;
      LARGE_INTEGER new_off;
      if (::SetFilePointerEx(m_handle, move_off, &new_off, mv_method) != 0)
      {
        ec.clear();
        return new_off.QuadPart;
      }
      ec.assign(::GetLastError(), system_category());
      return -1LL;

#   else  // BOOST_POSIX_API
      int whence;
      if (from == seekdir::end)
        whence = SEEK_END;
      else if (from == seekdir::current)
        whence = SEEK_CUR;
      else
        whence = SEEK_SET;
      
      BOOST_IOSTREAMS_FD_OFFSET new_offset = ::BOOST_IOSTREAMS_FD_SEEK(handle(), offset, whence);
      if (new_offset == -1)
        ec.assign(errno, system_category());
      else
        ec.clear();
      return new_offset;

#   endif
    }

    binary_file::offset_type binary_file::seek(offset_type offset, seekdir::pos from)
    {
      BOOST_ASSERT(is_open());
      error_code ec;
      offset_type result = seek(offset, from, ec);
      if (ec)
        BOOST_BTREE_THROW(filesystem::filesystem_error("binary_file::seek", path(), ec));
      return result;
    }
  } // namespace btree
} // namespace boost
