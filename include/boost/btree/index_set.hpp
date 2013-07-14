//  boost/btree/set.hpp  ---------------------------------------------------------------//

//  Copyright Beman Dawes 2000, 2006, 2010, 2013

//  Distributed under the Boost Software License, Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt

//  See http://www.boost.org/libs/btree for documentation.

#ifndef BOOST_BTREE_SET_HPP
#define BOOST_BTREE_SET_HPP

#include <boost/config.hpp>
#include <boost/btree/detail/index_common.hpp>

#include <boost/btree/set.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/cstdint.hpp>
#include <boost/btree/dynamic_size.hpp>
#include <boost/btree/header.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/assert.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_pointer.hpp>

namespace boost
{
  namespace btree
  {

//--------------------------------------------------------------------------------------//
//                                class index_set                                       //
//--------------------------------------------------------------------------------------//

    template <class Key, class Traits = default_traits, class Comp = btree::less<Key> >
      : public index_base<Key, index_set_base<Key,Comp>, Traits, Comp>
    class index_set
    {
    public:
      typedef std::size_t position_type;
    private:
      btree_set<position_type, Traits, Comp>  m_btree;
      boost::iostreams::mapped_file           m_file;
      std::size_t                             m_file_size;  // in bytes
    public:

      BOOST_STATIC_ASSERT_MSG( !boost::is_pointer<Key>::value,
        "Key must not be a pointer type");

      explicit index_set(const Comp& comp = Comp()) : m_btree(comp) {}

      explicit index_set(const boost::filesystem::path& p,
        flags::bitmask flgs = flags::read_only,
        uint64_t signature = -1,  // for existing files, must match signature from creation
        std::size_t node_sz = default_node_size,  // ignored if existing file
        const Comp& comp = Comp())
        : m_btree(p,
            flags::open_flags(flgs) | flags::key_only | flags::unique,
            signature, node_sz, comp),
          m_file(p.string().append(".file"), boost::iostreams::mapmode::readwrite, 
            1000000),
          m_file_size(0) {}

      //template <class InputIterator>
      //index_set(InputIterator begin, InputIterator end,
      //  const boost::filesystem::path& p,
      //  flags::bitmask flgs = flags::read_only,
      //  uint64_t signature = -1,  // for existing files, must match signature from creation
      //  std::size_t node_sz = default_node_size,  // ignored if existing file
      //  const Comp& comp = Comp())
      //: btree_base<Key,index_set_base<Key,Comp>,Traits,Comp>(p,
      //    flags::open_flags(flgs) | flags::key_only | flags::unique,
      //    signature, node_sz, comp)
      //{
      //  for (; begin != end; ++begin)
      //  {
      //    btree_base<Key,index_set_base<Key,Comp>,Traits,Comp>::m_insert_unique(
      //      *begin, *begin);
      //  }
      //}
 
      void open(const boost::filesystem::path& p,
        flags::bitmask flgs = flags::read_only,
        uint64_t signature = -1,  // for existing files, must match signature from creation
        std::size_t node_sz = default_node_size) // node_sz ignored if existing file
        : m_btree(p,
          flags::open_flags(flgs) | flags::key_only | flags::unique, signature, node_sz),
          m_file(p.string().append(".file"), boost::iostreams::mapmode::readwrite), 
            1000000),
          m_file_size(0) {}

      ////  emplace(const Key&) special case not requiring c++0x support
      //std::pair<typename btree_base<Key,index_set_base<Key,Comp>,Traits,Comp>::const_iterator, bool>
      //emplace(const typename btree_base<Key,index_set_base<Key,Comp>,Traits,Comp>::value_type& value)
      //{
      //  return btree_base<Key,index_set_base<Key,Comp>,Traits,Comp>::m_insert_unique(
      //    value, value);
      //}

      //std::pair<typename btree_base<Key,index_set_base<Key,Comp>,Traits,Comp>::const_iterator, bool>
      //insert(const typename btree_base<Key,index_set_base<Key,Comp>,Traits,Comp>::value_type& value)
      //{
      //  return btree_base<Key,index_set_base<Key,Comp>,Traits,Comp>::m_insert_unique(
      //    value, value);
      //}

      //template <class InputIterator>
      //void insert(InputIterator begin, InputIterator end)
      //{
      //  for (; begin != end; ++begin)
      //  {
      //    btree_base<Key,index_set_base<Key,Comp>,Traits,Comp>::m_insert_unique(
      //      *begin, *begin);
      //  }
      //}
    };

//--------------------------------------------------------------------------------------//
//                              class btree_multiset                                    //
//--------------------------------------------------------------------------------------//

    //template <class Key, class Traits = default_traits,
    //          class Comp = btree::less<Key> >              
    //class btree_multiset
    //  : public btree_base<Key, index_set_base<Key,Comp>, Traits, Comp>
    //{
    //public:

    //  BOOST_STATIC_ASSERT_MSG( !boost::is_pointer<Key>::value, "Key must not be a pointer type");

    //  // <Key,Comp> is required by GCC but not by VC++
    //  explicit btree_multiset(const Comp& comp = Comp())
    //    : btree_base<Key,index_set_base<Key,Comp>,Traits,Comp>(comp) {}

    //  explicit btree_multiset(const boost::filesystem::path& p,
    //    flags::bitmask flgs = flags::read_only,
    //    uint64_t signature = -1,  // for existing files, must match signature from creation
    //    std::size_t node_sz = default_node_size,  // ignored if existing file
    //    const Comp& comp = Comp())
    //    : btree_base<Key,index_set_base<Key,Comp>,Traits,Comp>(p,
    //        flags::open_flags(flgs) | flags::key_only, signature, node_sz, comp) {}

    //  template <class InputIterator>
    //  btree_multiset(InputIterator begin, InputIterator end,
    //    const boost::filesystem::path& p,
    //    flags::bitmask flgs = flags::read_only,
    //    uint64_t signature = -1,  // for existing files, must match signature from creation
    //    std::size_t node_sz = default_node_size,  // ignored if existing file
    //    const Comp& comp = Comp())
    //  : btree_base<Key,index_set_base<Key,Comp>,Traits,Comp>(p,
    //      flags::open_flags(flgs) | flags::key_only, signature, node_sz, comp)
    //  {
    //    for (; begin != end; ++begin)
    //    {
    //      btree_base<Key,index_set_base<Key,Comp>,Traits,Comp>::m_insert_non_unique(
    //        *begin, *begin);
    //    }
    //  }

    //  void open(const boost::filesystem::path& p,
    //    flags::bitmask flgs = flags::read_only,
    //    uint64_t signature = -1,  // for existing files, must match signature from creation
    //    std::size_t node_sz = default_node_size) // node_sz ignored if existing file
    //  {
    //     btree_base<Key,index_set_base<Key,Comp>,Traits,Comp>::m_open(p,
    //      flags::open_flags(flgs) | flags::key_only, signature, node_sz);
    //  }

    //  //  emplace(const Key&) special case not requiring c++0x support
    //  std::pair<typename btree_base<Key,index_set_base<Key,Comp>,Traits,Comp>::const_iterator, bool>
    //  emplace(const typename btree_base<Key,index_set_base<Key,Comp>,Traits,Comp>::value_type& value)
    //  {
    //    return btree_base<Key,index_set_base<Key,Comp>,Traits,Comp>::m_insert_non_unique(
    //      value, value);
    //  }

    //  typename btree_base<Key,index_set_base<Key,Comp>,Traits,Comp>::const_iterator
    //  insert(const typename btree_base<Key,index_set_base<Key,Comp>,Traits,Comp>::value_type& value)
    //  {
    //    return btree_base<Key,index_set_base<Key,Comp>,Traits,Comp>::m_insert_non_unique(
    //      value, value);
    //  }

    //  template <class InputIterator>
    //  void insert(InputIterator begin, InputIterator end)
    //  {
    //    for (; begin != end; ++begin) 
    //    {
    //      btree_base<Key,index_set_base<Key,Comp>,Traits,Comp>::m_insert_non_unique(
    //        *begin, *begin);
    //    }
    //  }
    //};

  } // namespace btree
} // namespace boost

#endif  // BOOST_BTREE_SET_HPP
