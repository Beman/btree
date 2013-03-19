//  binary_file.hpp - low level read/write file I/O  -----------------------------------//

//  Copyright Beman Dawes 2005, 2010

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  See library home page at http://www.boost.org/libs/fileio

//--------------------------------------------------------------------------------------//

//  The binary_file class is basically a C++ wrapper around POSIX open/read/write/close
//  etc., and Windows equivalent, functionality.
//
//  Key design goals:
//
//  * Modern C++ practice, such as use of Boost, Boost.Filesystem, the standard libary,
//      and detailed error reporting.
//  * Handle files with gaps (i.e. sparse files) correctly.

//--------------------------------------------------------------------------------------// 

#ifndef BOOST_BINARY_FILE_HPP
#define BOOST_BINARY_FILE_HPP

#include <boost/btree/detail/config.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/detail/bitmask.hpp>
#include <boost/assert.hpp>
#include <ios>
#include <cstddef>  // for size_t

#include <boost/config/abi_prefix.hpp>  // must be the last #include

namespace boost
{
  namespace btree
  {
    namespace oflag
    {
      enum bitmask
      {
        in          =1,       // open for input
        out         =1<<1,    // open for output
        truncate    =1<<2,    // truncate an existing file when opening

        seek_end    =1<<3,    // seek to end immediately after opening

        random      =1<<6,    // hint: optimize for random access
        sequential  =1<<7,    // hint: optimize for sequential access

        preload     =1<<8     // hint: read entire file on open to preload O/S disk cache
      };

      BOOST_BITMASK(bitmask);
    }

    namespace seekdir
    {
      enum pos
      {
        begin = std::ios_base::beg,
        current = std::ios_base::cur,
        end = std::ios_base::end
      };
    }

    //  class binary_file  -------------------------------------------------------------//
    
    class BOOST_BTREE_DECL binary_file // noncopyable
    {
    private:
      binary_file(const binary_file&);
      const binary_file& operator=(const binary_file&);

    public:
#    ifdef BOOST_WINDOWS_API
      typedef void*            handle_type; 
      typedef boost::intmax_t  offset_type;
      static const handle_type invalid_handle;
#    else // POSIX API
      typedef int              handle_type; 
      typedef boost::intmax_t  offset_type; 
      static const handle_type invalid_handle = -1;
#    endif

 
      binary_file()
        : m_handle(invalid_handle) {}

      explicit binary_file(const filesystem::path& p, oflag::bitmask flags=oflag::in)
        : m_handle(invalid_handle) { open(p, flags); }

      binary_file(const filesystem::path& p, oflag::bitmask flags, system::error_code& ec)
        : m_handle(invalid_handle) { open(p, flags, ec); }

     ~binary_file();

      void open(const filesystem::path& p, oflag::bitmask flags=oflag::in);
      // Requires: !is_open()
      bool open(const filesystem::path& p, oflag::bitmask flags, system::error_code& ec);
      // Requires: !is_open()
      // Returns: true if successful.

      void close();
      bool close(system::error_code& ec);
      // Effects: If the file is not open, none, othewise as if calls POSIX
      // close(). Sets ec to 0 if no error, otherwise to the system error code.
      // Returns: true if successful.

      bool is_open() const
      { 
        return m_handle != invalid_handle;
      }

      handle_type handle() const  { return m_handle; }

      const filesystem::path& file_path() const  { return m_path; }

      // -------------------------------------------------------------------------------//
 
      // Requirement on type T, void* objects below: Memcpyable

      // Rationale for treating raw_read/raw_write targets and sources as
      // void*, but plain read/write targets and sources as template
      // template parameter T's: This emphasizes that raw_read/raw_write may
      // not complete the entire request, while read/write always completes the
      // requested action for the entire size requested. The template approach
      // for read/write reduces the chance that the wrong size will be
      // specified.

      // -------------------------------------------------------------------------------//

      std::size_t raw_read(void* target, std::size_t sz, system::error_code& ec);
      // Requires: is_open()
      // Effects: As if calls POSIX read(). Sets ec to 0 if no error,
      // otherwise to the system error code.
      // Returns: Count of bytes actually read if no error, 0 if end-of-file,
      //   or -1 if error.
      // unspecified value if error

      std::size_t raw_read(void* target, std::size_t sz);
      // Requires: is_open()
      // Effects: As if calls POSIX read().
      // Returns: Count of bytes actually read, except 0 if end-of-file.
      // Throws: On error

      template<typename T>
      bool read(T& target, std::size_t sz, system::error_code& ec)
      // Requires: is_open()
      // Effects: As if calls POSIX read(), except will finish partial
      // reads. ec.clear() if no error, otherwise set ec to the system error code.
      // Returns: true, except false if end-of-file or error.
      {
        BOOST_ASSERT(is_open());
        return m_read(&target, sz, ec);
      }

      template<typename T>
      bool read(T& target, std::size_t sz = sizeof(T))
      // Requires: is_open()
      // Effects: As if POSIX read(), except will finish partial reads.
      // Throws: On error.
      // Returns: true, except false if end-of-file.
      {
        BOOST_ASSERT(is_open());
        return m_read(&target, sz);
      }

      std::size_t raw_write(const void* source, std::size_t sz,
        system::error_code& ec);
      // Requires: is_open()
      // Effects: As if, calls POSIX write(). Sets ec to 0 if no error,
      // otherwise to the system error code.
      // Returns: Count of bytes actually written if no error, 0 if error.

      std::size_t raw_write(const void* source, std::size_t sz);
      // Requires: is_open()
      // Effects: As if, calls POSIX write().
      // Returns: Count of bytes actually written.
      // Throws: On error

      template<typename T>
      void write(const T& source, std::size_t sz, system::error_code& ec)
      // Requires: is_open()
      // Effects: As if, calls POSIX write(), except will finish partial
      // writes. Sets ec to 0 if no error, otherwise to the system error code.
      {
        BOOST_ASSERT(is_open());
        m_write(&source, sz, ec);
      }

      //template<typename T>
      //void write(const T& source, std::size_t sz = sizeof(T))
      //// Requires: is_open()
      //// Effects: As if, POSIX write(), except will finish partial write.
      //// Length of write is n* sizeof(boost::remove_extent<T>::type).
      //// Throws: On error.
      //{
      //  BOOST_ASSERT(is_open());
      //  m_write(&source, sz);
      //}

      void write(const void* source, std::size_t sz)
      // Requires: is_open()
      // Effects: As if, POSIX write(), except will finish partial write.
      // Length of write is n* sizeof(boost::remove_extent<T>::type).
      // Throws: On error.
      {
        BOOST_ASSERT(is_open());
        m_write(source, sz);
      }
      void write(const char* source, std::size_t sz)
      // Requires: is_open()
      // Effects: As if, POSIX write(), except will finish partial write.
      // Length of write is n* sizeof(boost::remove_extent<T>::type).
      // Throws: On error.
      {
        BOOST_ASSERT(is_open());
        m_write(source, sz);
      }

      offset_type seek(offset_type offset, seekdir::pos from,
        system::error_code& ec);
      // Effects: As if POSIX lseek(), except with offset argument of a type
      // sufficient for maximum file size for operating system. Sets ec to 0 if
      // no error, otherwise to the system error code.
      // Returns: As if POSIX lseek(), except with return type sufficient
      // for maximum file size for possible operating system.

      offset_type seek(offset_type offset, seekdir::pos from = seekdir::begin);
      // Effects: As if POSIX lseek(), except with offset argument of a type
      // sufficient for maximum file size for operating system. 
      // Returns: As if POSIX lseek(), except with return type sufficient
      // for maximum file size possible for operating system.
      // Throws: On error.

      // dup, dup2 ?
      // lockf ?
      // static sync?

    private:
      handle_type              m_handle; // -1 indicates not open
      boost::filesystem::path  m_path;

      bool m_read(void* target, std::size_t sz, system::error_code& ec);
      bool m_read(void* target, std::size_t sz);
      void m_write(const void* source, std::size_t sz, system::error_code& ec);
      void m_write(const void* source, std::size_t sz);

    }; // binary_file

  } // namespace btree
} // namespace boost

//--------------------------------------------------------------------------------------//

#include <boost/config/abi_suffix.hpp> // pops abi_prefix.hpp pragmas

#endif  // BOOST_BINARY_FILE_HPP
