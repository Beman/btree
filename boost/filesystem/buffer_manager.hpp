//  buffer_manager.hpp -----------------------------------------------------------------//

//  Copyright Beman Dawes 2005, 2010

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  See library home buffer at http://www.boost.org/libs/fileio

//--------------------------------------------------------------------------------------//

#ifndef BOOST_BUFFER_MANAGER_HPP
#define BOOST_BUFFER_MANAGER_HPP

#include <boost/file_io/binary_file.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/intrusive/set.hpp>
#include <boost/intrusive/list.hpp>
#include <boost/scoped_array.hpp>
#include <boost/cstdint.hpp>
#include <boost/assert.hpp>
#include <iosfwd>
#include <cstddef>  // for size_t
#include <cstring>  // for memset

//#include <iostream>  // comment me out!

#define BOOST_BUFFER_FILE_THROW(x) throw x

#include <boost/config/abi_prefix.hpp>  // must be the last #include

#ifdef BOOST_MSVC
// for intrusive_list and intrusive_set, disable C4251, ...needs to have
// dll-interface to be used by clients of class ...
#  pragma warning(push)
#  pragma warning(disable: 4251) 
#endif

namespace boost
{
  namespace filesystem3
  {
    class buffer_manager_error : public std::runtime_error
    {
    public:
      buffer_manager_error(const std::string& what, const path& p)
        : runtime_error(what + p.string()) {}
    };

    class buffer;
    class buffer_manager;

//--------------------------------------------------------------------------------------//
//                                                                                      //
//      buffer_ptr - a smart pointer to reference counted class buffer objects          //
//                                                                                      //
//--------------------------------------------------------------------------------------//

  /***************** TODO: replace with boost::intrusive_ptr? ***************/

    class buffer_ptr
    {
    public:

      buffer_ptr() : m_ptr(0) {}
      buffer_ptr(buffer& p);
      buffer_ptr(const buffer_ptr& r);
      buffer_ptr& operator=(const buffer_ptr& r)
      {
        buffer_ptr(r).swap(*this);  // correct for self-assignment
        return *this;
      }
      ~buffer_ptr();

      void swap(buffer_ptr& r)
      {
        buffer* tmp(m_ptr);
        m_ptr = r.m_ptr;
        r.m_ptr = tmp;
      }

      void reset();

      buffer* get() const {return m_ptr;}

      buffer& operator*() const
      {
        BOOST_ASSERT(m_ptr);
        return *m_ptr;
      }
      buffer* operator->() const
      {
        BOOST_ASSERT(m_ptr);
        return m_ptr;
      }

      typedef void (*unspecified_bool_type)();
      static void unspecified_bool_true() {}
      operator unspecified_bool_type() const {return m_ptr == 0 ?0:unspecified_bool_true;} 
      bool operator! () const {return m_ptr == 0;}

      bool operator==(const buffer_ptr& r) const {return m_ptr == r.m_ptr;}
      bool operator!=(const buffer_ptr& r) const {return m_ptr != r.m_ptr;}
      bool operator< (const buffer_ptr& r) const {return m_ptr < r.m_ptr;}

    protected:
      buffer* m_ptr;
    };

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                       buffer - a buffer holding disk buffers                         //
//                                                                                      //
//--------------------------------------------------------------------------------------//

    class buffer
      : public boost::intrusive::set_base_hook<>, 
        public boost::intrusive::list_base_hook<> 
    {
    public:
      typedef boost::uint32_t    buffer_id_type;
      typedef boost::uint32_t    use_count_type;
      typedef buffer_manager*    buffer_manager_pointer;

      buffer()
        : m_buffer_id(-1), m_use_count(0), m_manager(0), m_needs_write(false),
          m_data(0) {}

      //  construct a dummy buffer w/ id only
      explicit buffer(buffer_id_type id)
        : m_buffer_id(id), m_use_count(0), m_manager(0), m_needs_write(false),
          m_data(0) {}

      //  construct a complete fully-managed buffer
      buffer(buffer_id_type id, buffer_manager& pm);

      buffer_id_type   buffer_id() const       { return m_buffer_id; }
      use_count_type   use_count() const       { return m_use_count; }
      buffer_manager&  manager() const         { return *m_manager; }
      bool             needs_write() const     { return m_needs_write; }
      bool operator<(const buffer& rhs) const  { return buffer_id() < rhs.buffer_id(); }

      void             manager(buffer_manager_pointer pm) { m_manager = pm; }

      void inc_use_count()                     { ++m_use_count; }
      void dec_use_count();

      void             reuse(buffer_id_type id)
      {
        BOOST_ASSERT(m_use_count == 0);  // must not reuse buffer if still in use
        BOOST_ASSERT(!m_needs_write);  // must not reuse buffer if it needs to be written
        m_buffer_id = id;
      }

      void             needs_write(bool x)     { m_needs_write = x; }

      char*            data()                  { return m_data.get(); }
      const char*      data() const            { return m_data.get(); }

    protected:
      friend class buffer_manager;

      buffer_id_type              m_buffer_id;
      use_count_type              m_use_count;
      buffer_manager_pointer      m_manager;
      boost::scoped_array<char>   m_data;  // file buffer
      bool                        m_needs_write;
    };


//--------------------------------------------------------------------------------------//
//                                                                                      //
//  buffer_manager - manages a binary disk file and its associated buffer objects       //
//                                                                                      //
//  Uses cases involve fixed length buffers, random access, and a need to minimize      //
//  seeks via a buffer cache. The need for speed must be great enough that simply       //
//  relying on operating system disk caching is not sufficient.                         //
//                                                                                      //
//  The associated buffer objects are owned by the buffer_manager.                      //
//  Buffer objects are cached; the buffer_manager keeps a map of the buffers, keyed on  //
//  buffer_id, so that requests for a buffer are always satisfied with a buffer_ptr to  //
//  the same buffer object if it is in memory. To prevent memory allocation churn, a    //
//  list of available buffers still in memory is also kept.                             //
//                                                                                      //
//--------------------------------------------------------------------------------------//

    inline buffer* default_buffer_alloc(buffer::buffer_id_type pg_id, buffer_manager& mgr)
      { return new buffer(pg_id, mgr); }

    class BOOST_FILESYSTEM_DECL buffer_manager : public binary_file
    {
      // buffer_manager is a non-copyable type
      buffer_manager(const buffer_manager&);
      buffer_manager& operator=(const buffer_manager&);

    public:
      typedef buffer::buffer_id_type  buffer_id_type;
      typedef boost::uint32_t         buffer_count_type;
      typedef std::size_t             data_size_type;
      typedef buffer* (*buffer_alloc)(buffer_id_type, buffer_manager&);

      explicit buffer_manager(buffer_alloc alloc = default_buffer_alloc)
        //  alloc function pointer allows management of classes derived from buffer
        //  yet still permits separate compilation
        : m_buffer_count(0), m_data_size(0), m_max_cache_buffers(0), m_alloc(alloc) {}

      ~buffer_manager();

      bool open(const boost::filesystem3::path& p,
        oflag::bitmask flags,
        std::size_t max_cache_pgs=16,
        data_size_type data_sz=512);  // data size for new or truncated files
      //  Returns: true if existing non-trucated file.
      //  NOTE: IF true IS RETURNED, IT IS REQUIRED THAT data_size() BE CALLED WITH
      //  AN ARGUMENT OF THE ACTUAL DATA SIZE BEFORE ANY BUFFER RELATED OPERATIONS ARE
      //  PERFORMED.

      void data_size(data_size_type sz);

      buffer_ptr new_buffer();
      //  Returns: Pointer to a new buffer, ready for use
      //  Postconditions: needs_write() is true, buffer_count() is increased by 1
      //  Remarks: buffer_id() for the returned pointer will be new buffer_count() less 1 

      buffer_ptr read(buffer_id_type buffer_id);
      //  Throws: if buffer_id is not a valid (i.e. existing) buffer number

      void write(buffer& pg);

      void flush();

      // modifiers
      void             max_cache_buffers(std::size_t m) {m_max_cache_buffers = m;}

      // observers
      buffer_count_type  buffer_count() const         {return m_buffer_count;}
      std::size_t      max_cache_buffers() const    {return m_max_cache_buffers;}
      data_size_type   data_size() const          {return m_data_size;}  // on disk

      void*            owner() const              {return m_owner;}
      void             owner(void* p)             {m_owner = p;}

      boost::uint32_t  active_buffers_read() const  {return m_active_buffers_read;}
      boost::uint32_t  cached_buffers_read() const  {return m_cached_buffers_read;}
      boost::uint32_t  file_buffers_read() const    {return m_file_buffers_read;}
      boost::uint32_t  file_buffers_written() const {return m_file_buffers_written;}
      boost::uint32_t  new_buffer_requests() const  {return m_new_buffer_requests;}
      boost::uint32_t  buffer_allocs() const        {return m_buffer_allocs;}
      boost::uint32_t  buffers_in_memory() const    {return buffer_set.size();}
      boost::uint32_t  buffers_available() const    {return buffer_available_list.size();}

//    private:

      friend class buffer;

      typedef boost::intrusive::set<buffer>  buffer_set_type;
      typedef boost::intrusive::list<buffer> buffer_list_type;

      buffer_set_type   buffer_set;             // all buffers in memory that have been
                                            // allocated by this buffer manager

      buffer_list_type  buffer_available_list;  // buffers in memory with use_count() == 0;
                                            // in effect this is a LRU list with
                                            // begin() being the least recently used buffer

    private:

      buffer_count_type   m_buffer_count;       // number of buffers in the file
      data_size_type    m_data_size;        // number of bytes per disk buffer
      std::size_t       m_max_cache_buffers;  // maximum number of buffers to cache; may be 0
      void*             m_owner;            // not used by buffer_manager itself
      buffer_alloc        m_alloc;            // memory allocation function pointer

      //  activity counts
      boost::uint32_t   m_active_buffers_read;
      boost::uint32_t   m_cached_buffers_read;
      boost::uint32_t   m_file_buffers_read;
      boost::uint32_t   m_file_buffers_written;
      boost::uint32_t   m_new_buffer_requests;
      boost::uint32_t   m_buffer_allocs;

      buffer* m_prepare_buffer(buffer_id_type pg_id);
    };

    BOOST_FILESYSTEM_DECL
    std::ostream& operator<<(std::ostream& os, const buffer_manager& pm);
    // aid for debugging, tuning

//--------------------------------------------------------------------------------------//

    inline buffer_ptr::buffer_ptr(buffer& p) : m_ptr(&p)
    {
      p.inc_use_count();
    }

    inline buffer_ptr::buffer_ptr(const buffer_ptr& r)
    {
      m_ptr = r.m_ptr;
      if (m_ptr)
        m_ptr->inc_use_count();
    }

    inline buffer_ptr::~buffer_ptr()
    { 
      if (m_ptr)
        m_ptr->dec_use_count();
    }

    inline void buffer_ptr::reset()
    {
      if (m_ptr)
      {
        m_ptr->dec_use_count();
        m_ptr = 0;
      }
    }

    inline buffer::buffer(buffer_id_type id, boost::filesystem3::buffer_manager& pm)
      : m_buffer_id(id), m_use_count(0), m_manager(&pm), m_needs_write(false),
        m_data(new char[pm.data_size()]) {}

    inline void buffer::dec_use_count()
    {
      BOOST_ASSERT(use_count() != 0);
      if ( --m_use_count == 0
        && buffer_id() != -1  // dummy buffers have id -1
        && m_manager)  // TODO: should m_manager be removed after initial testing? 
        manager().buffer_available_list.push_back(*this);
    }


  }  // namespace filesystem3
}  // namespace boost

//--------------------------------------------------------------------------------------//

namespace boost
{
  namespace filesystem
  {
    using filesystem3::buffer;
    using filesystem3::buffer_ptr;
    using filesystem3::buffer_manager;
  }
}

//--------------------------------------------------------------------------------------//

#ifdef BOOST_MSVC
#  pragma warning(pop)
#endif

#include <boost/config/abi_suffix.hpp> // pops abi_prefix.hpp pragmas

#endif  // BOOST_BUFFER_MANAGER_HPP
