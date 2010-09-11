//  buffer_manager.cpp -----------------------------------------------------------------//

//  Copyright Beman Dawes 2010

//  Distributed under the Boost Software License, Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt

//--------------------------------------------------------------------------------------//

// define BOOST_FILESYSTEM_SOURCE so that <boost/filesystem/config.hpp> knows
// the library is being built (possibly exporting rather than importing code)
#define BOOST_FILESYSTEM_SOURCE 

#include <boost/filesystem/detail/buffer_manager.hpp>
#include <ostream>

namespace boost
{
namespace filesystem3
{
 
//-------------------------------- ~buffer_manager() -----------------------------------//

buffer_manager::~buffer_manager()
{
  if (is_open())
  {
    flush();
    close();
    m_buffer_count = 0;
    m_data_size = 0;
  }

  //// deletion of a buffer with a non-zero parent can cause !buffer_available_list.empty()
  //// after it has supposedly been cleared; a fix is to do a pre-delete pass just
  //// to do a parent().reset().
  //// TODO: does this in fact signifiy a logic error? Shouldn't all parents already
  //// have been reset()? Yet BOOST_ASSERT(buffer_available_list.empty()) below fires
  //// without this pre-delete pass.
  //for (buffer_set_type::iterator itr = buffer_set.begin(); itr != buffer_set.end(); ++itr)
  //{
  //  itr->parent().reset();
  //}

  // must clear buffer_available_list before deleting buffers
  //std::cout << " clearing buffer_available_list with " << buffer_available_list.size() << " buffers" << std::endl;
  buffer_available_list.clear();

  // delete all buffers in buffer_set
  //std::cout << " deleting " << buffer_set.size() << " buffers" << std::endl;
  for (buffer_set_type::iterator itr = buffer_set.begin(); itr != buffer_set.end();)
  {
    buffer_set_type::iterator cur = itr++;
    buffer* pg = &*cur;
    buffer_set.erase(cur);  // remove from buffer_set before deleting memory
    //std::cout << "   deleting buffer " << pg->buffer_id() << " at " << pg << std::endl;
    delete pg;
    BOOST_ASSERT(buffer_available_list.empty()); // fires if buffer had non-zero parent() 
  }
  //std::cout << " all buffers deleted" << std::endl;
}
 
//------------------------------------- open() -----------------------------------------//

bool buffer_manager::open(const boost::filesystem3::path& p, oflag::bitmask flags,
  std::size_t max_cache_pgs, data_size_type data_sz)
//  Returns: true if existing non-trucated file.
//  NOTE: IF true IS RETURNED, IT IS REQUIRED THAT data_size() BE CALLED WITH
//  AN ARGUMENT OF THE ACTUAL DATA SIZE BEFORE ANY BUFFER RELATED OPERATIONS ARE
//  PERFORMED.
{
  BOOST_ASSERT(!is_open());
  BOOST_ASSERT(data_sz);
  m_buffer_count = 0;
  m_data_size = data_sz;
  m_max_cache_buffers = max_cache_pgs;

  m_active_buffers_read = m_cached_buffers_read = m_file_buffers_read
    = m_file_buffers_written = m_new_buffer_requests = m_buffer_allocs = 0;

  if (flags & oflag::truncate)
    flags |= oflag::out;
  if (flags & oflag::out)
    flags |= oflag::in;

  if (boost::filesystem3::exists(p) && !(flags & oflag::truncate)) // existing file
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
      binary_file::file_path()));
}

//------------------------------- m_prepare_buffer() -----------------------------------//

buffer* buffer_manager::m_prepare_buffer(buffer_id_type pg_id)
{
  buffer* pg;

  if (buffer_available_list.empty()
    || buffer_available_list.size() < max_cache_buffers())
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
    BOOST_ASSERT(!buffer_available_list.empty());
    pg = &*buffer_available_list.begin();
    buffer_available_list.pop_front();
    buffer_set.erase(buffer_set.iterator_to(*pg));
    if (pg->needs_write())
    {
      write(*pg);
      pg->m_needs_write = false;
    }
    pg->reuse(pg_id);
  }
  buffer_set.insert(*pg);
  return pg;
}
 
//----------------------------------- new_buffer() -------------------------------------//

buffer_ptr buffer_manager::new_buffer()
{
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
  BOOST_ASSERT(data_size());
  BOOST_ASSERT(pg_id < buffer_count());

  buffer key(pg_id);   // TODO: move the key buffer to buffer_manager to avoid construction

  buffer_set_type::iterator found = buffer_set.find(key);

  if (found == buffer_set.end()) // the buffer is not in memory
  {
    ++m_file_buffers_read;
    buffer* pg = m_prepare_buffer(pg_id);
    binary_file::seek(pg_id * data_size());
    binary_file::read(*pg->data(), data_size());
    return buffer_ptr(*pg);
  }
  else // the buffer is in memory
  {
    if (found->use_count() == 0)  // buffer not in use, but is in buffer_available_list
    { 
      ++m_cached_buffers_read;
      // remove from buffer_available_list
      buffer_available_list.erase(buffer_available_list.iterator_to(*found));  
    }
    else
      ++m_active_buffers_read;
    return buffer_ptr(*found);
  }
}
 
//-------------------------------------- write() ----------------------------------------//

void buffer_manager::write(buffer& pg)
{
  BOOST_ASSERT(pg.buffer_id() < buffer_count());
  seek(pg.buffer_id()*data_size());
  binary_file::write(pg.data(), data_size());
  pg.needs_write(false);
  ++m_file_buffers_written;
}
  
//-------------------------------------- flush() ---------------------------------------//

void buffer_manager::flush()
{
  for (buffer_set_type::iterator itr = buffer_set.begin();
    itr != buffer_set.end();
    ++itr)
  {
    if (itr->needs_write())
      write(*itr);
  }
}
  
//------------------------------------ operator<<() ------------------------------------//

BOOST_FILESYSTEM_DECL
std::ostream& operator<<(std::ostream& os, const buffer_manager& pm)
// aid for debugging, tuning
{
  os << pm.file_path() << " statistics:\n"
    << "  buffer size -------------: " << pm.data_size() << "\n"  
    << "  buffer count ------------: " << pm.buffer_count() << "\n"  
    << "  buffer allocs -----------: " << pm.buffer_allocs() << "\n"
    << "  new buffer requests -----: " << pm.new_buffer_requests() << "\n"  
    << "  file buffers written ----: " << pm.file_buffers_written() << "\n\n"  
    << "  in-use buffers read -----: " << pm.active_buffers_read() << "\n"  
    << "  cached buffers read -----: " << pm.cached_buffers_read() << "\n"  
    << "  file buffers read -------: " << pm.file_buffers_read() << "\n"
    << "  -----------------------------\n"
    << "  total buffers read ------: " << pm.active_buffers_read() + pm.cached_buffers_read()
                                        + pm.file_buffers_read() << "\n\n"
    << "  buffers in use ----------: " << pm.buffers_in_memory() - pm.buffers_available() << "\n"
    << "  buffers in cache --------: " << pm.buffers_available() << "\n"  
    << "  -----------------------------\n"
    << "  total buffers in memory -: " << pm.buffers_in_memory() << "\n"  
      ;
  return os;
}

}  // namespace filesystem3
}  // namespace boost
