//  boost/btree/header.hpp  ------------------------------------------------------------//

//  Copyright Beman Dawes 2006, 2010

//  Distributed under the Boost Software License, Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt

//  See http://www.boost.org/libs/btree for documentation.

#ifndef BOOST_BTREE_HEADER_HPP
#define BOOST_BTREE_HEADER_HPP

#include <boost/config/warning_disable.hpp>

#include <boost/cstdint.hpp>
#include <boost/detail/bitmask.hpp>
#include <boost/integer/endian.hpp>
#include <boost/integer/endian_flip.hpp>
#include <boost/detail/scoped_enum_emulation.hpp>
#include <boost/assert.hpp>
#include <cstring>
#include <cstddef>
#include <ostream>

namespace boost
{
  namespace btree
  {
    struct default_native_traits
    {
      typedef boost::uint32_t  page_id_type;     // page ids on page
      typedef boost::uint16_t  page_size_type;   // number of elements on page
      typedef boost::uint16_t  page_level_type;  // level of page; 0 for leaf page
      static const BOOST_SCOPED_ENUM(integer::endianness) header_endianness
#   ifdef BOOST_BIG_ENDIAN
        = integer::endianness::big;
#   else
        = integer::endianness::little;
#   endif
    };

    struct default_big_endian_traits
    {
      typedef integer::ubig32_t  page_id_type;
      typedef integer::ubig16_t  page_size_type;
      typedef integer::ubig16_t  page_level_type;
      static const BOOST_SCOPED_ENUM(integer::endianness) header_endianness
        = integer::endianness::big;
    };

    struct default_little_endian_traits
    {
      typedef integer::ulittle32_t  page_id_type;
      typedef integer::ulittle16_t  page_size_type;
      typedef integer::ulittle16_t  page_level_type;
      static const BOOST_SCOPED_ENUM(integer::endianness) header_endianness
        = integer::endianness::little;
    };

    namespace flags
    {
      enum bitmask
      {
        // choose one:
        read_only   = 0,    // file must exist
        read_write  = 1,    // open existing file, otherwise create new file
        truncate    = 2,    // same as read_write except existing file truncated

        // bitmasks set by user:
        preload     = 0x10, // existing file read to preload O/S file cache

        // bitmasks set by implemenation, ignored if passed in by user:
        multi       = 4,    // multimap or multiset; non-uniqueness allowed
        key_only    = 8     // set or multiset
      };

      BOOST_BITMASK(bitmask);

      bitmask user(bitmask m) {return m & (read_write|truncate|preload); }
    }

    static const boost::uint8_t major_version = 0;  // version identification
    static const boost::uint8_t minor_version = 1;

    static const std::size_t default_page_size = 512;
    static const std::size_t default_max_cache_pages = 16;

    namespace flags
    {
      enum bitmask;
    }

//--------------------------------------------------------------------------------------//
//                                 class header_page                                    //
//--------------------------------------------------------------------------------------//

    class header_page
    {
    private:
      // stuff often looked at in dumps comes first, subject to alignment requirements

      boost::uint32_t     m_marker;              // 0xBBBBBBBB
      boost::uint8_t      m_endianness;          // 0x01 == big, 0x02 == little
      boost::uint8_t      m_major_version;
      boost::uint8_t      m_minor_version;
      boost::uint8_t      m_unused1;             // unused

      boost::uint64_t     m_element_count;

      boost::uint32_t     m_page_size;           // disk page size in bytes
      boost::uint32_t     m_flags;
      boost::uint32_t     m_root_page_id;
      boost::uint32_t     m_first_page_id;
      boost::uint32_t     m_last_page_id;
      boost::uint32_t     m_page_count;
      boost::uint32_t     m_free_page_list_head_id;  // list of recycleable pages

      boost::uint16_t     m_root_level;
      boost::uint16_t     m_key_size;            // sizeof(key_type)
      boost::uint16_t     m_mapped_size;         // sizeof(mapped_type)

      // TODO: Add a better way to verify that an existing file is opened using the same
      // Key, T, Comp, and Traits or when originally created. 

      char                m_splash_c_str[32]; // " boost.org btree_map" or whatever;
                                              // '\0' filled and terminated
      char                m_user_c_str[32];   // '\0' filled and terminated

    public:
      header_page() : m_marker(0xBBBBBBBB), m_endianness(0), 
        m_root_page_id(0), m_first_page_id(0), m_last_page_id(0),
        m_element_count(0), m_page_count(0),
        m_root_level(0),
        m_major_version(btree::major_version), m_minor_version(btree::minor_version),   
        m_free_page_list_head_id(0) {}

      //  "permanent" members that do not change over the life of the file
      bool             marker_ok() const             { return m_marker == 0xBBBBBBBB; }
      bool             big_endian() const            { BOOST_ASSERT(m_endianness);
                                                       return m_endianness == 1; }
      const char*      splash_c_str() const          { return m_splash_c_str; }
      boost::uint8_t   major_version() const         { return m_major_version; }  
      boost::uint8_t   minor_version() const         { return m_minor_version; }  
      boost::uint32_t  page_size() const             { return m_page_size; }
      boost::uint16_t  key_size() const              { return m_key_size; }
      boost::uint16_t  mapped_size() const           { return m_mapped_size; }
      flags::bitmask   flags() const { return static_cast<flags::bitmask>(m_flags); }

      //  "updated" members that change as the file changes
      boost::uint64_t  element_count() const         { return m_element_count; }
      boost::uint32_t  root_page_id() const          { return m_root_page_id; }
      boost::uint32_t  first_page_id() const         { return m_first_page_id; }
      boost::uint32_t  last_page_id() const          { return m_last_page_id; }
      boost::uint32_t  page_count() const            { return m_page_count; }
      boost::uint32_t  free_page_list_head_id() const{ return m_free_page_list_head_id; }
      int              root_level() const            { return m_root_level; }
      int              levels() const                { return m_root_level+1; }

      //  user supplied-data members
      const char*      user_c_str() const            { return m_user_c_str; }
      void             user_c_str(const char* str) 
      { 
        std::strncpy(m_user_c_str, str, sizeof(m_user_c_str)-1);
        m_user_c_str[sizeof(m_user_c_str)-1] = '\0';
      }

      void  big_endian(bool x)                       { m_endianness = x ? 0x01 : 0x02; }
      void  splash_c_str(const char* str)
      {
        std::strncpy(m_splash_c_str, str, sizeof(m_splash_c_str)-1);
        m_splash_c_str[sizeof(m_splash_c_str)-1] = '\0';
      }
      void  major_version(boost::uint8_t value)      { m_major_version = value; } 
      void  minor_version(boost::uint8_t value)      { m_minor_version = value; }  
      void  page_size(std::size_t sz)                { m_page_size = sz; }
      void  key_size(std::size_t sz)                 { m_key_size = sz; }
      void  mapped_size(std::size_t sz)              { m_mapped_size = sz; }
      void  flags(flags::bitmask flgs)               { m_flags = flgs; }
      void  element_count(boost::uint64_t value)     { m_element_count = value; }
      void  increment_element_count()                { ++m_element_count; }
      void  decrement_element_count()                { --m_element_count; }
      void  root_page_id(boost::uint32_t id)         { m_root_page_id = id; }
      void  first_page_id(boost::uint32_t id)        { m_first_page_id = id; }
      void  last_page_id(boost::uint32_t id)         { m_last_page_id = id; }
      void  page_count(boost::uint32_t value)        { m_page_count = value; }
      void  increment_page_count()                   { ++m_page_count; }
      void  free_page_list_head_id(boost::uint32_t id){ m_free_page_list_head_id = id; }
      void  root_level(uint16_t value)               { m_root_level = value; }
      boost::uint16_t  increment_root_level()        { return ++m_root_level; }
      void  decrement_root_level()                   { --m_root_level; }

      void endian_flip_if_needed()
      {
        BOOST_ASSERT(m_endianness == 1 || m_endianness == 2);
        if (
#         ifdef BOOST_BIG_ENDIAN
            m_endianness == 2  // big endian platform but little endian header
#         else
            m_endianness == 1  // little endian platform but big endian header
#         endif
          )
        {
          integer::endian_flip(m_page_size);
          integer::endian_flip(m_key_size);
          integer::endian_flip(m_mapped_size);
          integer::endian_flip(m_element_count);
          integer::endian_flip(m_flags);
          integer::endian_flip(m_root_level);
          integer::endian_flip(m_root_page_id);
          integer::endian_flip(m_first_page_id);
          integer::endian_flip(m_last_page_id);
          integer::endian_flip(m_page_count);
          integer::endian_flip(m_free_page_list_head_id);
        }
      }
    };

  }  // namespace btree
}  // namespace boost

#endif // BOOST_BTREE_HEADER_HPP