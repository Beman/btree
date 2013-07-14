//  mmff.hpp  --------------------------------------------------------------------------//

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                     memory-mapped flat-file; mmff for short                          //
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

namespace boost
{
namespace btree
{
  class extendible_mapped_file
  {
  public:
    typedef boost::iostreams::mapped_file::mapmode mapmode;

    extendible_mapped_file() {}
    explicit extendible_mapped_file(boost::filesystem::path& p, 
      mapmode mm = mapmode::readonly, std::size_t reserv = 0)
    {
      open(p, mm, reserv);
    }

    ~extendible_mapped_file()  {close();}

    void open(boost::filesystem::path& p, 
              mapmode mm = mapmode::readonly,
              std::size_t reserv = 0)
    {
      BOOST_ASSERT(!is_open());
      m_path = p;
      m_mode = mm;
      m_reserve = reserv;
      // TODO: handle non-existant file or request to truncate
      m_file_size = boost::filesystem::file_size(p);
      if (reserv)
        boost::filesystem::resize_file(p, file_size() + reserve());
      m_file.open(p.string(), mm);
    }

    void close()
    {
      if (is_open())
      {
        m_file.close();
        if (file_size() != boost::filesystem::file_size(m_path))
          boost::filesystem::resize_file(path(), file_size());
      }
    }

    bool is_open() const                    {return m_file.is_open();}


    mapmode mode() const                    {return m_mode;}
    std::size_t reserve() const             {return m_reserve;}

    std::size_t file_size() const           {BOOST_ASSERT(is_open());
                                             return m_file_size;}
    std::size_t mapped_size() const         {BOOST_ASSERT(is_open());
                                             return m_file.size();}
    const boost::filesystem::path& path() const
                                            {BOOST_ASSERT(is_open());
                                             return m_path;} 

    void resize(std::size_t new_sz)
    {
      BOOST_ASSERT(is_open());
      m_file.close();
      boost::filesystem::resize_file(path(), new_sz); 
      m_file.open(m_path.string(), mode());
    }

    void increase_size(std::size_t inc)
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
      return reinterpret_cast<T*>(m_path.data());
    }

    template <class T>
    const T* const_data() const
    // Note well: Returned pointer is invalidated by the next resize().
    {
      BOOST_ASSERT(m_file.is_open());
      return reinterpret_cast<const T*>(m_path.const_data());
    }

  private:
    boost::filesystem::path        m_path;
    mapmode                        m_mode;
    std::size_t                    m_reserve;
    boost::iostreams::mapped_file  m_file;
    std::size_t                    m_file_size;
  };


}  // namespace btree
}  // namespace boost

#endif  BOOST_BTREE_MMFF_HPP
