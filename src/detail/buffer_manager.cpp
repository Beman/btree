//  buffer_manager.cpp -----------------------------------------------------------------//

//  Copyright Beman Dawes 2010

//  Distributed under the Boost Software License, Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt

//--------------------------------------------------------------------------------------//

// define BOOST_BTREE_SOURCE so that <boost/filesystem/config.hpp> knows
// the library is being built (possibly exporting rather than importing code)
#define BOOST_BTREE_SOURCE 

#include <boost/btree/detail/buffer_manager.hpp>
#include <ostream>

namespace boost
{
namespace btree
{
//------------------------------------ close() -----------------------------------------//

void buffer_manager::close()
{
  BOOST_ASSERT(is_open());

  available_buffers.clear();

  // clear buffers, deleting those with use_count() == 0
  for (buffers_type::iterator itr = buffers.begin(); itr != buffers.end();)
  {
    if (itr->needs_write())
    {
      write(*itr);
      itr->needs_write(false);
    }
    buffers_type::iterator cur = itr++;
    buffer* buf = &*cur;
    buffers.erase(cur);
    if (buf->use_count() == 0)
    {
      //std::cout << "   deleting buffer " << buf->buffer_id() << " at " << buf << std::endl;
      delete buf;
    }
    else
      cur->manager(0);  // mark buffer as orphaned; it has outlived its manager
  }
  BOOST_ASSERT(buffers.empty());
  BOOST_ASSERT(available_buffers.empty());
  //std::cout << " all buffers deleted" << std::endl;
  binary_file::close();
  m_buffer_count = 0;
  m_data_size = 0;
}

//-------------------------------- ~buffer_manager() -----------------------------------//

buffer_manager::~buffer_manager()
{
  if (is_open())
  {
    close();
  }
}
 
//------------------------------------- open() -----------------------------------------//

bool buffer_manager::open(const boost::filesystem::path& p, oflag::bitmask flags,
  std::size_t max_cache_pgs, data_size_type data_sz)
//  Returns: true if existing non-trucated file.
//  NOTE: IF true IS RETURNED, IT IS REQUIRED THAT data_size() BE CALLED WITH
//  AN ARGUMENT OF THE ACTUAL DATA SIZE BEFORE ANY BUFFER RELATED OPERATIONS ARE
//  PERFORMED.
{
  BOOST_ASSERT(!is_open());
  BOOST_ASSERT(data_sz);
  BOOST_ASSERT(buffers.empty());
  BOOST_ASSERT(available_buffers.empty());

  m_buffer_count = 0;
  m_data_size = data_sz;
  m_max_cache_size = max_cache_pgs;

  clear_statistics();

  if (flags & oflag::truncate)
    flags |= oflag::out;
  if (flags & oflag::out)
    flags |= oflag::in;

  if (boost::filesystem::exists(p) && !(flags & oflag::truncate)) // existing file
    m_data_size = 0;  // as yet unknown

  binary_file::open(p, flags);
  return m_data_size == 0;
}

//----------------------------------- data_size() --------------------------------------//
 
void buffer_manager::data_size(data_size_type sz)
{
  BOOST_ASSERT(sz);
  BOOST_ASSERT(!data_size());
  m_data_size = sz;
  offset_type file_size = binary_file::seek(0, seekdir::end);
  m_buffer_count = static_cast<buffer_count_type>(file_size / sz);
  if (m_buffer_count * sz != file_size)
    BOOST_BUFFER_FILE_THROW(buffer_manager_error(
      "buffer_manager_error: file size error; too large or not multiple of data size: ",
      binary_file::path()));
}

//------------------------------- m_prepare_buffer() -----------------------------------//

buffer* buffer_manager::m_prepare_buffer(buffer_id_type pg_id)
{
  buffer* pg;

  if (available_buffers.empty()
    || available_buffers.size() < max_cache_size())
  {
    // allocate a new buffer
    pg = m_alloc(pg_id, *this);
    //std::cout << " allocated buffer " << reinterpret_cast<void*>(pg)
    //  << " buffer.data() at " << reinterpret_cast<void*>(pg->data())
    //   << std::endl;
    ++m_buffer_allocs;
  }
  else
  {
    // reuse an existing buffer
    BOOST_ASSERT(!available_buffers.empty());
    pg = &*available_buffers.begin();
    available_buffers.pop_front();
    buffers.erase(buffers.iterator_to(*pg));
    if (pg->needs_write())
    {
      write(*pg);
      pg->m_needs_write = false;
    }
    pg->reuse(pg_id);
  }
  buffers.insert(*pg);
  return pg;
}
 
//----------------------------------- new_buffer() -------------------------------------//

buffer_ptr buffer_manager::new_buffer()
{
  BOOST_ASSERT(is_open());
  BOOST_ASSERT(data_size());
  ++m_new_buffer_requests;
  buffer* pg = m_prepare_buffer(m_buffer_count++);
  // clear the memory; this makes troubleshooting ever so much easier
  std::memset(pg->data(), 0, data_size());
  pg->needs_write(true);
  return buffer_ptr(*pg);
}
 
//--------------------------------------- read() ---------------------------------------//

buffer_ptr buffer_manager::read(buffer_id_type pg_id)
{
  BOOST_ASSERT(is_open());
  BOOST_ASSERT(data_size());
  BOOST_ASSERT(pg_id < buffer_count());

  buffer key(pg_id);   // TODO: move the key buffer to buffer_manager to avoid construction

  buffers_type::iterator found = buffers.find(key);

  if (found == buffers.end()) // the buffer is not in memory
  {
    ++m_file_buffers_read;
    buffer* pg = m_prepare_buffer(pg_id);
    binary_file::seek(pg_id * data_size());
    binary_file::read(pg->data(), data_size());
    return buffer_ptr(*pg);
  }
  else // the buffer is in memory
  {
    if (found->use_count() == 0)  // buffer not in use
    { 
      if (!found->never_free())  // but is in available_buffers
      {
        // remove from available_buffers
        available_buffers.erase(available_buffers.iterator_to(*found));
        ++m_available_buffers_read;
      }
      else
        ++m_never_free_buffers_read;
    }
    else
      ++m_active_buffers_read;
    return buffer_ptr(*found);
  }
}
 
//-------------------------------------- write() ----------------------------------------//

void buffer_manager::write(buffer& pg)
{
  BOOST_ASSERT(is_open());
  BOOST_ASSERT(pg.buffer_id() < buffer_count());
  seek(pg.buffer_id()*data_size());
  binary_file::write(pg.data(), data_size());
  pg.needs_write(false);
  ++m_file_buffers_written;
}
  
//-------------------------------- clear_write_needed() --------------------------------//

void buffer_manager::clear_write_needed()
{
  for (buffer_manager::buffers_type::iterator itr = buffers.begin();
    itr != buffers.end();
    ++itr)
  {
    itr->needs_write(false);
  }
}

//-------------------------------------- flush() ---------------------------------------//

bool buffer_manager::flush()
{
  BOOST_ASSERT(is_open());
  bool buffer_written = false;
  for (buffers_type::iterator itr = buffers.begin();
    itr != buffers.end();
    ++itr)
  {
    if (itr->needs_write())
    {
      write(*itr);
      itr->needs_write(false);
      buffer_written = true;
    }
  }
  return buffer_written;
}
  
//------------------------------------ operator<<() ------------------------------------//

BOOST_BTREE_DECL
std::ostream& operator<<(std::ostream& os, const buffer_manager& pm)
// aid for debugging, tuning
{
  os 
    << "  buffer size --------------: " << pm.data_size() << "\n"  
    << "  buffer count -------------: " << pm.buffer_count() << "\n"  
    << "  buffer allocs ------------: " << pm.buffer_allocs() << "\n"
    << "  new buffer requests ------: " << pm.new_buffer_requests() << "\n"  
    << "  never-free honored -------: " << pm.never_free_honored() << "\n"  
    << "  file buffers written -----: " << pm.file_buffers_written() << "\n\n"  
    << "  cached buffers read ------: " << pm.cached_buffers_read() << "\n"  
    << "  file buffers read --------: " << pm.file_buffers_read() << "\n"
    << "  total buffers read -------: " << pm.active_buffers_read() + pm.cached_buffers_read()
                                        + pm.file_buffers_read() << "\n\n"
    << "  cached read breakdown:\n"
    << "    active buffers ---------: " << pm.active_buffers_read() << "\n"  
    << "    available buffers ------: " << pm.available_buffers_read() << "\n"  
    << "    never-free buffers -----: " << pm.never_free_buffers_read() << "\n\n"  
    << "  cache size ---------------: " << pm.buffers_in_memory() << "\n"
    << "  cache buffers in use -----: " << pm.buffers_in_use() << "\n"
    << "  cache buffers available --: " << pm.buffers_available() << "\n"
      ;
  return os;
}

}  // namespace btree
}  // namespace boost
