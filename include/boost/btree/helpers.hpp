//  boost/btree/helpers.hpp  -----------------------------------------------------------//

//  Copyright Beman Dawes 2006, 2010

//  Distributed under the Boost Software License, Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt

//  See http://www.boost.org/libs/btree for documentation.

#ifndef BOOST_BTREE_HELPERS_HPP
#define BOOST_BTREE_HELPERS_HPP

#include <boost/config/warning_disable.hpp>

#include <boost/btree/detail/config.hpp>
#include <boost/cstdint.hpp>
#include <boost/detail/bitmask.hpp>
#include <boost/endian/types.hpp>
#include <boost/assert.hpp>
//#include <cstring>
#include <cstdlib>

namespace boost
{
namespace btree
{

//--------------------------------------------------------------------------------------//
//                      version numbers and default constants                           //
//--------------------------------------------------------------------------------------//

    static const uint16_t major_version = 0;  // version identification
    static const uint16_t minor_version = 1;

    static const std::size_t default_node_size = 4096;
    static const std::size_t default_max_cache_nodes = 32;

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
//                                       flags                                          //
//--------------------------------------------------------------------------------------//

namespace flags
{
  enum bitmask
  {
    none          = 0,

    // bitmasks set by implemenation, ignored if passed in by user:
    unique        = 1,    // not multi; uniqueness required
    key_only      = 2,    // set or multiset
 
    // open values (choose one):
    read_only   = 0x100,   // file must exist
    read_write  = 0x200,   // open existing file, otherwise create new file
    truncate    = 0x400,   // same as read_write except existing file truncated

    // bitmask options set by user; not present in header:
    preload        = 0x1000, // existing file read to preload O/S file cache
    cache_branches = 0x2000, // enable permanent cache of all branch pages touched;
                              // otherwise make branch pages available when use count
                              // becomes 0, just like leaf pages.
  };

  BOOST_BITMASK(bitmask);

  inline bitmask open_flags(bitmask m)
    {return m & (read_write|truncate|preload|cache_branches); }
  inline bitmask permanent_flags(bitmask m)
    {return m & ~(read_write|truncate|preload|cache_branches); }
}

//--------------------------------------------------------------------------------------//
//                               less function object                                   //
//--------------------------------------------------------------------------------------//

//  Needed to support heterogeneous comparisons. See N3657, N3421.

//  Note: Allowing less::operator() to accept arguments of two different types does not
//  eliminate the requirement that objects of the two types be themselves < comparable.
//  For example, type fat in test/btree_unit_test.cpp supplies, as required, two
//  non-member operator< functions so that a fat object to be < compared to an int and
//  an int can be compared to a fat object.

struct less
{
  template <class T, class U> bool operator()(const T& t, const U& u) const
  {
    return t < u;
  }
};

}  // namespace btree
}  // namespace boost

#endif // BOOST_BTREE_HELPERS_HPP
