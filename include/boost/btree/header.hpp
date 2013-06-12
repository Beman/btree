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
#include <boost/endian/types.hpp>
#include <boost/endian/conversion.hpp>
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
//                                     Traits                                           //
//                                                                                      //
//  Traits provide the types for management objects on btree disk nodes. Nodes are      //
//  typically 4096 bytes in length, and every byte wasted in overhead causes a          //
//  measurable reduction in speed if the tree adds levels. That may favor unaligned     //
//  traits, particularly if the user's key and mapped types are also unaligned.         //
//                                                                                      //
//  On the other hand, aligned types are more efficient and generate less code.         //
//                                                                                      //
//  Actual timing tests, however, show virtually no real-world speed differences        //
//  between aligned and unaligned traits, or native and non-native endian traits.       //
//  Other factors, such as maximum cache size, size of mapped data, portability, and    //
//  so on, determine btree performance.                                                 //
//                                                                                      //
//  The big_endian_traits are choosen as the default because file dumps are easier to   //
//  read, files are portable, and no other factors have any measurable effect on        //
//  performance.                                                                        //
//                                                                                      //
//--------------------------------------------------------------------------------------//

    struct big_endian_traits
    {
      typedef endian::big_uint32un_t  node_id_type;     // node ids are page numbers
      typedef uint8_t                 node_level_type;  // level of node; 0 for leaf node.
      typedef endian::big_uint24un_t  node_size_type;   // permits large node sizes
      static const BOOST_SCOPED_ENUM(endian::order) header_endianness
        = endian::order::big;
    };

    struct little_endian_traits
    {
      typedef endian::little_uint32un_t  node_id_type;     // node ids are page numbers
      typedef uint8_t                    node_level_type;  // level of node; 0 for leaf node.
      typedef endian::little_uint24un_t  node_size_type;   // permits large node sizes
      static const BOOST_SCOPED_ENUM(endian::order) header_endianness
        = endian::order::little;
    };
  
    struct native_endian_traits
    {
      typedef endian::native_uint32un_t  node_id_type;     // node ids are page numbers
      typedef uint8_t                    node_level_type;  // level of node; 0 for leaf node.
      typedef endian::native_uint24un_t  node_size_type;   // permits large node sizes
      static const BOOST_SCOPED_ENUM(endian::order) header_endianness
#   ifdef BOOST_BIG_ENDIAN
        = endian::order::big;
#   else
        = endian::order::little;
#   endif
    };

    typedef big_endian_traits  default_traits;  // see rationale above

//--------------------------------------------------------------------------------------//

    namespace flags
    {
      enum bitmask
      {
        none          = 0,

        // bitmasks set by implemenation, ignored if passed in by user:
        unique        = 1,    // not multi; uniqueness required
        key_only      = 2,    // set or multiset
        key_varies    = 4,    // key is variable length
        mapped_varies = 8,    // mapped is variable length
 
        // open values (choose one):
        read_only   = 0x100,   // file must exist
        read_write  = 0x200,   // open existing file, otherwise create new file
        truncate    = 0x400,   // same as read_write except existing file truncated

        // bitmask options set by user; not present in header:
        preload     = 0x1000, // existing file read to preload O/S file cache
        no_cache_branches     // disable permanent cache of all branch pages touched
                    = 0x2000, //   (but still cache temporarily as usual)
      };

      BOOST_BITMASK(bitmask);

      inline bitmask open_flags(bitmask m) {return m & (read_write|truncate|preload); }
      inline bitmask permanent_flags(bitmask m) {return m & ~(read_write|truncate|preload); }
    }

    static const uint16_t major_version = 0;  // version identification
    static const uint16_t minor_version = 1;

    static const std::size_t default_node_size = 4096;
    static const std::size_t default_max_cache_nodes = 32;

    namespace flags
    {
      enum bitmask;
    }

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                                 class header_page                                    //
//                                                                                      //
//  This class defines its own node id, size, and level types rather than being         //
//  tempated on Traits because an attempt to do so bogged down in messy details.        //
//                                                                                      //
//  Node id size is 32-bits because that is sufficient for now, and this elminates      //
//  yet more messy details of alignment management on 32-bit systems.                   //
//                                                                                      //
//  Such niceties can wait until version 2.                                             //
//                                                                                      //
//--------------------------------------------------------------------------------------//

    class header_page
    {
    public:
      typedef uint32_t    node_id_type;
      typedef uint32_t    node_size_type;
      typedef uint8_t     node_level_type;  // even fanout 2 allows 2 to 256th pow elements
      typedef uint32_t    flags_type;
      typedef uint16_t    version_type;
      typedef uint32_t    key_size_type;
      typedef uint32_t    mapped_size_type;

    private:
      // stuff often looked at in dumps comes first, subject to alignment requirements

      char                m_marker[6];          
      uint8_t             m_endianness;         // 0x01 == big, 0x02 == little 
      node_level_type     m_root_level;
      uint64_t            m_element_count;      // achieves alignment without padding
      uint64_t            m_signature;
      flags_type          m_flags;           
      key_size_type       m_key_size;            // sizeof(key_type); -1 imples indirect
      mapped_size_type    m_mapped_size;         // sizeof(mapped_type); -1 implies indirect
      node_size_type      m_node_size;           // disk node size in bytes

      node_id_type        m_root_node_id;
      node_id_type        m_first_node_id;
      node_id_type        m_last_node_id;
      node_id_type        m_node_count;          // total including free node list
      node_id_type        m_leaf_node_count;     // active only; free nodes not include
      node_id_type        m_branch_node_count;   // active only; free nodes not include
      node_id_type        m_free_node_list_head_id;  // list of recycleable nodes
      node_id_type        m_unassigned[2];
      version_type        m_major_version;   
      version_type        m_minor_version; 

      char                m_splash_c_str[16]; // "boost.org btree" or whatever;
                                              // '\0' filled and terminated
      char                m_user_c_str[32];   // '\0' filled and terminated


    public:
      header_page() { clear(); }

      void clear()
      {
        std::memset(this, 0, sizeof(header_page));
        std::memset(m_marker, 0xBB, sizeof(m_marker));
        m_major_version = btree::major_version;
        m_minor_version = btree::minor_version;   
      }

      //  "permanent" members that do not change over the life of the file
      bool             marker_ok() const 
      {
        char mark[sizeof(m_marker)];
        std::memset(mark, 0xBB, sizeof(m_marker));
        return std::memcmp(m_marker, mark, sizeof(m_marker)) == 0;
      }
      bool             big_endian() const            { return m_endianness == 1; }
      uint64_t         signature() const             { return m_signature; }
      const char*      splash_c_str() const          { return m_splash_c_str; }
      version_type     major_version() const         { return m_major_version; }  
      version_type     minor_version() const         { return m_minor_version; }  
      std::size_t      node_size() const             { return m_node_size; }
      std::size_t      key_size() const              { return m_key_size; }
      std::size_t      mapped_size() const           { return m_mapped_size; }
      flags::bitmask   flags() const                 { return static_cast<flags::bitmask>(m_flags); }

      //  "updated" members that change as the file changes
      uint64_t         element_count() const         { return m_element_count; }
      node_id_type     root_node_id() const          { return m_root_node_id; }
      node_id_type     first_node_id() const         { return m_first_node_id; }
      node_id_type     last_node_id() const          { return m_last_node_id; }
      node_id_type     node_count() const            { return m_node_count; }
      node_id_type     leaf_node_count() const       { return m_leaf_node_count; }
      node_id_type     branch_node_count() const     { return m_branch_node_count; }
      node_id_type     free_node_list_head_id() const{ return m_free_node_list_head_id; }
      node_level_type  root_level() const            { return m_root_level; }
      node_level_type  levels() const                { return m_root_level+1; }

      //  user supplied-data members
      const char*      user_c_str() const            { return m_user_c_str; }
      void             user_c_str(const char* str) 
      { 
        std::strncpy(m_user_c_str, str, sizeof(m_user_c_str)-1);
        m_user_c_str[sizeof(m_user_c_str)-1] = '\0';
      }

      void  big_endian(bool x)                       { m_endianness = x ? 0x01 : 0x02; }
      void  signature(uint64_t x)                    { m_signature = x; }
      void  splash_c_str(const char* str)
      {
        std::strncpy(m_splash_c_str, str, sizeof(m_splash_c_str)-1);
        m_splash_c_str[sizeof(m_splash_c_str)-1] = '\0';
      }
      void  major_version(version_type value)        { m_major_version = value; } 
      void  minor_version(version_type value)        { m_minor_version = value; }  
      void  node_size(std::size_t sz)                { m_node_size = static_cast<node_size_type>(sz); }
      void  key_size(std::size_t sz)                 { m_key_size = static_cast<key_size_type>(sz); }
      void  mapped_size(std::size_t sz)              { m_mapped_size = static_cast<mapped_size_type>(sz); }
      void  flags(flags::bitmask flgs)               { m_flags = flgs; }
      void  element_count(boost::uint64_t value)     { m_element_count = value; }
      void  increment_element_count()                { ++m_element_count; }
      void  decrement_element_count()                { --m_element_count; }
      void  root_node_id(node_id_type id)            { m_root_node_id = id; }
      void  first_node_id(node_id_type id)           { m_first_node_id = id; }
      void  last_node_id(node_id_type id)            { m_last_node_id = id; }
      void  node_count(node_id_type value)           { m_node_count = value; }
      void  increment_node_count()                   { ++m_node_count; }
      void  leaf_node_count(node_id_type value)      { m_leaf_node_count = value; }
      void  increment_leaf_node_count()              { ++m_leaf_node_count; }
      void  decrement_leaf_node_count()              { --m_leaf_node_count; }
      void  branch_node_count(node_id_type value)    { m_branch_node_count = value; }
      void  increment_branch_node_count()            { ++m_branch_node_count; }
      void  decrement_branch_node_count()            { --m_branch_node_count; }
      void  free_node_list_head_id(node_id_type id)  { m_free_node_list_head_id = id; }
      void  root_level(node_level_type value)        { m_root_level = value; }
      node_level_type  increment_root_level()        { return ++m_root_level; }
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
          endian::reverse(m_node_size);
          endian::reverse(m_key_size);
          endian::reverse(m_mapped_size);
          endian::reverse(m_element_count);
          endian::reverse(m_signature);
          endian::reverse(m_flags);
          endian::reverse(m_root_node_id);
          endian::reverse(m_first_node_id);
          endian::reverse(m_last_node_id);
          endian::reverse(m_node_count);
          endian::reverse(m_leaf_node_count);
          endian::reverse(m_branch_node_count);
          endian::reverse(m_free_node_list_head_id);
        }
      }
    };

  }  // namespace btree
}  // namespace boost

#endif // BOOST_BTREE_HEADER_HPP
