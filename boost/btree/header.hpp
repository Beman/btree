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

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                                   Default Traits                                     //
//                                                                                      //
//  The traits provide the types for management objects on btree disk nodes. Nodes are  //
//  typically 4096 bytes in length, and every byte wasted in overhead causes a          //
//  measurable reduction in speed. Node formats were designed to avoid alignment        //
//  bytes, given the defaults.                                                          //
//                                                                                      //
//--------------------------------------------------------------------------------------//

    struct default_native_traits
    {
      typedef boost::uint32_t  node_id_type;     // node ids
      typedef boost::uint16_t  node_size_type;   // sizes
      typedef boost::uint16_t  node_level_type;  // level of node; 0 for leaf node.
                                                 // Could be smaller, but that would
                                                 // require alignment byte so why bother
      static const BOOST_SCOPED_ENUM(integer::endianness) header_endianness
#   ifdef BOOST_BIG_ENDIAN
        = integer::endianness::big;
#   else
        = integer::endianness::little;
#   endif
    };

    struct default_big_endian_traits
    {
      typedef integer::ubig32_t  node_id_type;
      typedef integer::ubig16_t  node_size_type;
      typedef integer::ubig16_t  node_level_type;
      static const BOOST_SCOPED_ENUM(integer::endianness) header_endianness
        = integer::endianness::big;
    };

    struct default_little_endian_traits
    {
      typedef integer::ulittle32_t  node_id_type;
      typedef integer::ulittle16_t  node_size_type;
      typedef integer::ulittle16_t  node_level_type;
      static const BOOST_SCOPED_ENUM(integer::endianness) header_endianness
        = integer::endianness::little;
    };

    struct default_endian_traits
    {
      typedef integer::ubig32_t  node_id_type;
      typedef integer::ubig16_t  node_size_type;
      typedef integer::ubig16_t  node_level_type;
      static const BOOST_SCOPED_ENUM(integer::endianness) header_endianness
        = integer::endianness::big;
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
        unique      = 4,    // multimap or multiset; non-uniqueness allowed
        key_only    = 8     // set or multiset
      };

      BOOST_BITMASK(bitmask);

// zzzzzzzzzzzzzz bda: changed to inline
      inline bitmask user(bitmask m) {return m & (read_write|truncate|preload); }
    }

    static const boost::uint8_t major_version = 0;  // version identification
    static const boost::uint8_t minor_version = 1;

    static const std::size_t default_node_size = 4096;
    static const std::size_t default_max_cache_nodes = 32;

    namespace flags
    {
      enum bitmask;
    }

//--------------------------------------------------------------------------------------//
//                                 class header_page                                    //
//--------------------------------------------------------------------------------------//

    class header_page
    {
    public:
      typedef boost::uint32_t node_id_type;
    private:
      // stuff often looked at in dumps comes first, subject to alignment requirements

      boost::uint32_t     m_marker;              // 0xBBBBBBBB
      boost::uint8_t      m_endianness;          // 0x01 == big, 0x02 == little
      boost::uint8_t      m_major_version;
      boost::uint8_t      m_minor_version;
      boost::uint8_t      m_unused1;             // unused

      boost::uint64_t     m_element_count;

      boost::uint32_t     m_node_size;           // disk node size in bytes
      boost::uint32_t     m_flags;
      node_id_type        m_root_node_id;
      node_id_type        m_first_node_id;
      node_id_type        m_last_node_id;
      node_id_type        m_node_count;
      node_id_type        m_free_node_list_head_id;  // list of recycleable nodes

      boost::uint16_t     m_root_level;
      boost::uint16_t     m_key_size;            // sizeof(key_type); -1 imples indirect
      boost::uint16_t     m_mapped_size;         // sizeof(mapped_type); -1 implies indirect

      // TODO: Add a better way to verify that an existing file is opened using the same
      // Key, T, Comp, and Traits or when originally created. 

      char                m_splash_c_str[32]; // " boost.org btree_map" or whatever;
                                              // '\0' filled and terminated
      char                m_user_c_str[32];   // '\0' filled and terminated

    public:
      header_page() { clear(); }

      void clear()
      {
        std::memset(this, 0, sizeof(header_page));
        m_marker = 0xBBBBBBBB;
        m_major_version = btree::major_version;
        m_minor_version = btree::minor_version;   
      }

      //  "permanent" members that do not change over the life of the file
      bool             marker_ok() const             { return m_marker == 0xBBBBBBBB; }
      bool             big_endian() const            { BOOST_ASSERT(m_endianness);
                                                       return m_endianness == 1; }
      const char*      splash_c_str() const          { return m_splash_c_str; }
      boost::uint8_t   major_version() const         { return m_major_version; }  
      boost::uint8_t   minor_version() const         { return m_minor_version; }  
      boost::uint32_t  node_size() const             { return m_node_size; }
      boost::uint16_t  key_size() const              { return m_key_size; }
      boost::uint16_t  mapped_size() const           { return m_mapped_size; }
      flags::bitmask   flags() const { return static_cast<flags::bitmask>(m_flags); }

      //  "updated" members that change as the file changes
      boost::uint64_t  element_count() const         { return m_element_count; }
      node_id_type     root_node_id() const          { return m_root_node_id; }
      node_id_type     first_node_id() const         { return m_first_node_id; }
      node_id_type     last_node_id() const          { return m_last_node_id; }
      node_id_type     node_count() const            { return m_node_count; }
      node_id_type     free_node_list_head_id() const{ return m_free_node_list_head_id; }
      unsigned         root_level() const            { return m_root_level; }
      unsigned         levels() const                { return m_root_level+1; }

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
      void  node_size(std::size_t sz)                { m_node_size = sz; }
      void  key_size(std::size_t sz)                 { m_key_size = sz; }
      void  mapped_size(std::size_t sz)              { m_mapped_size = sz; }
      void  flags(flags::bitmask flgs)               { m_flags = flgs; }
      void  element_count(boost::uint64_t value)     { m_element_count = value; }
      void  increment_element_count()                { ++m_element_count; }
      void  decrement_element_count()                { --m_element_count; }
      void  root_node_id(node_id_type id)            { m_root_node_id = id; }
      void  first_node_id(node_id_type id)           { m_first_node_id = id; }
      void  last_node_id(node_id_type id)            { m_last_node_id = id; }
      void  node_count(node_id_type value)           { m_node_count = value; }
      void  increment_node_count()                   { ++m_node_count; }
      void  free_node_list_head_id(node_id_type id)  { m_free_node_list_head_id = id; }
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
          integer::endian_flip(m_node_size);
          integer::endian_flip(m_key_size);
          integer::endian_flip(m_mapped_size);
          integer::endian_flip(m_element_count);
          integer::endian_flip(m_flags);
          integer::endian_flip(m_root_level);
          integer::endian_flip(m_root_node_id);
          integer::endian_flip(m_first_node_id);
          integer::endian_flip(m_last_node_id);
          integer::endian_flip(m_node_count);
          integer::endian_flip(m_free_node_list_head_id);
        }
      }
    };

  }  // namespace btree
}  // namespace boost

#endif // BOOST_BTREE_HEADER_HPP
