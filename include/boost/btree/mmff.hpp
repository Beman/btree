//  mmff.hpp  --------------------------------------------------------------------------//

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

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/btree/helpers.hpp>
#include <boost/btree/detail/binary_file.hpp>

namespace boost
{
namespace btree
{
  class extendible_mapped_file
  {
  public:

    extendible_mapped_file() {}
    explicit extendible_mapped_file(boost::filesystem::path& p, 
      flags::bitmask flgs = flags::read_only, std::size_t reserv = 0)
    {
      open(p, flgs, reserv);
    }

    ~extendible_mapped_file()  {close();}

    void open(const boost::filesystem::path& p, 
              flags::bitmask flgs = flags::read_only,
              std::size_t reserv = 0)
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
                                                 
    std::size_t reserve() const                  {return m_reserve;}
                                                 
    boost::iostreams::mapped_file::mapmode       
      mode() const                               {BOOST_ASSERT(is_open());
                                                  return m_file.flags();}
    std::size_t file_size() const                {BOOST_ASSERT(is_open());
                                                  return m_file_size;}
    std::size_t mapped_size() const              {BOOST_ASSERT(is_open());
                                                  return m_file.size();}
    const boost::filesystem::path& path() const  {return m_path;} 

    void resize(std::size_t new_sz)
    {
      BOOST_ASSERT(is_open());
      m_file.close();
      boost::filesystem::resize_file(path(), new_sz); 
      m_file.open(path().string(), reopen_flags());
    }

    void increment_file_size(std::size_t inc)
    {
      BOOST_ASSERT(is_open());
      m_file_size += inc;
      if (file_size() > mapped_size())
        resize(file_size() + reserve());
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
    std::size_t                    m_reserve;
    boost::iostreams::mapped_file  m_file;
    std::size_t                    m_file_size;
  };


}  // namespace btree
}  // namespace boost

#endif  BOOST_BTREE_MMFF_HPP
