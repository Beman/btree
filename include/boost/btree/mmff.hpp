//  boost/btree/mmff.hpp  --------------------------------------------------------------//

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                     memory-mapped flat-file; mmff for short                          //
//                                                                                      //
//  Memory mapped files can't be extended beyond the size of the actual file, and       //
//  furthermore can't even be opened if zero length. To deal with these facts of life,  //
//  the uses specifies a reserve amount, and the external file is extended by that      //
//  amount before the memory mapped file is opened. When the memory mapped file is      //
//  closed, the external file is truncated by the amount of the reserve that has not    //
//  been used. The user is required to notify the mmff of a planned increase in size    //
//  so that reserve exaustion can be detected and a resize function invoked. The resize //
//  function closes the memory mapped file, resizes the disk file, and then reopens the //
//  memory mapped file.                                                                 //
//                                                                                      //
//--------------------------------------------------------------------------------------//

/*

  Initial design is based on KISS (Keep It Simple, Stupid!)

  * Do manage the size of the file, but not much else.


*/

#ifndef BOOST_BTREE_MMFF_HPP
#define BOOST_BTREE_MMFF_HPP

#if defined(_MSC_VER)  
#  pragma warning(push)
#  pragma warning(disable: 4251)
#endif

#include <boost/iostreams/device/mapped_file.hpp>

#if defined(_MSC_VER)  
#  pragma warning(pop)
#endif

#include <boost/filesystem.hpp>
#include <boost/btree/helpers.hpp>
#include <boost/btree/detail/binary_file.hpp>

namespace boost
{
namespace btree
{
  class extendible_mapped_file
  {
    // non-copyable, non-copy assignable
    extendible_mapped_file(const extendible_mapped_file&);
    extendible_mapped_file& operator=(const extendible_mapped_file&);
  public:
    typedef std::size_t size_type;       // size_type and position_type are the same
    typedef std::size_t position_type;   // type; the names differ to denote purpose

    extendible_mapped_file() {}
    explicit extendible_mapped_file(boost::filesystem::path& p, 
      flags::bitmask flgs = flags::read_only, size_type reserv = 0)
    {
      open(p, flgs, reserv);
    }

    ~extendible_mapped_file()  {close();}

    void open(const boost::filesystem::path& p, 
              flags::bitmask flgs = flags::read_only,
              size_type reserv = 0)
    {
      BOOST_ASSERT(!is_open());
      m_path = p;
      m_reopen_flags = flgs & ~flags::truncate;
      if (flgs & flags::truncate)
        m_reopen_flags |= flags::read_write;
      m_reserve = reserv;
      if (((flgs & ~flags::read_write) || (flgs & ~flags::truncate))
        && !boost::filesystem::exists(p))
      {
        binary_file fi(p, oflag::out);  // create an empty file
      }
      if (flgs & flags::truncate)
      {
        boost::filesystem::resize_file(p, 0);
        flgs |= flags::read_write;
      }
      m_file_size = boost::filesystem::file_size(p);

      //  memory-mapped files can't grow beyond the actual size of the file,
      //  and furthermore can't even be opened if zero length. So increase the
      //  external file size accordingly.
      boost::filesystem::resize_file(p, m_file_size + m_reserve
        + (m_reserve ? 0 : 1));
      m_file.open(p.string(), flgs & flags::read_write
        ? boost::iostreams::mapped_file::readwrite
        : boost::iostreams::mapped_file::readonly);
    }

    void close()
    {
      if (is_open())
      {
        m_file.close();
        if (m_file_size != boost::filesystem::file_size(path()))
          boost::filesystem::resize_file(path(), m_file_size);
      }
    }

    bool is_open() const                         {return m_file.is_open();}
                                                 
    flags::bitmask reopen_flags() const          {return m_reopen_flags;}
                                                 
    size_type reserve() const                    {return m_reserve;}
                                                 
    boost::iostreams::mapped_file::mapmode       
      mode() const                               {BOOST_ASSERT(is_open());
                                                  return m_file.flags();}
    size_type file_size() const                  {BOOST_ASSERT(is_open());
                                                  return m_file_size;}
    size_type mapped_size() const                {BOOST_ASSERT(is_open());
                                                  return m_file.size();}
    const boost::filesystem::path& path() const  {return m_path;} 

    void resize(size_type new_sz)
    {
      BOOST_ASSERT(is_open());
      m_file.close();
      boost::filesystem::resize_file(path(), new_sz); 
      m_file.open(path().string(), reopen_flags() & flags::read_write
        ? boost::iostreams::mapped_file::readwrite
        : boost::iostreams::mapped_file::readonly);
    }

    void increment_file_size(size_type inc)
    {
      BOOST_ASSERT(is_open());
      BOOST_ASSERT_MSG(data<void>(), "Error: attempt to extend read_only file");
      m_file_size += inc;
      if (file_size() > mapped_size())
        resize(file_size() + reserve());
    }

    // TODO: may wish to add overload for const char*, etc, std basic_string
    // or remove_ptr and/or decay

    template <class T>
    position_type push_back(const T& value, size_type n = 1)
    {
      BOOST_ASSERT(m_file.is_open());
      BOOST_ASSERT(n);
      BOOST_ASSERT_MSG(m_file.data(), "Error: attempt to push_back read_only file");
      size_type position = file_size();
      increment_file_size(sizeof(T)*n);
      std::memcpy(m_file.data()+position, &value, sizeof(T)*n);
      return position;
    }

    template <class T>
    T* data() const
    // Note well: Returned pointer is invalidated by the next resize().
    {
      BOOST_ASSERT(m_file.is_open());
      return reinterpret_cast<T*>(m_file.data());
    }

    template <class T>
    const T* const_data() const
    // Note well: Returned pointer is invalidated by the next resize().
    {
      BOOST_ASSERT(m_file.is_open());
      return reinterpret_cast<const T*>(m_file.const_data());
    }

  private:
    boost::filesystem::path        m_path;
    flags::bitmask                 m_reopen_flags;
    size_type                      m_reserve;
    boost::iostreams::mapped_file  m_file;
    size_type                      m_file_size;
  };


}  // namespace btree
}  // namespace boost

#endif  BOOST_BTREE_MMFF_HPP
