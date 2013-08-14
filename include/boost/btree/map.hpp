//  boost/btree/map.hpp  ---------------------------------------------------------------//

//  Copyright Beman Dawes 2000, 2006, 2010, 2013

//  Distributed under the Boost Software License, Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt

//  See http://www.boost.org/libs/btree for documentation.

#ifndef BOOST_BTREE_MAP_HPP
#define BOOST_BTREE_MAP_HPP

#include <boost/config.hpp>
#include <boost/cstdint.hpp>
#include <boost/btree/header.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/btree/detail/btree_bases.hpp>  // common to all 4 btree_* containers
#include <boost/assert.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_pointer.hpp>
#include <cstring>

/*  Rationale for order of constructor and open arguments:
      * path is required, and is a natural first argument.
      * flags is the most commonly needed of the remaining arguments.
      * signature is encouraged as it eliminates a common source of errors, so is next.
      * a custom compares is more common than a cusom node size, so is next.
      * custom node sizes are discouraged as a real need is rare and they are often
        an indication that a btree map or set is being used when an index map or set
        would be more appropriate.
*/

namespace boost
{
  namespace btree
  {

//--------------------------------------------------------------------------------------//
//                                 class btree_map                                      //
//--------------------------------------------------------------------------------------//

    template <class Key,    // requires memcpyable type without pointers or references
              class T,      // requires memcpyable type without pointers or references
              class Traits = default_traits,
              class Comp = btree::less >
    class btree_map
      : public btree_base<Key, btree_map_base<Key,T,Traits,Comp> >
    {
    public:
      typedef typename
        btree_base<Key, btree_map_base<Key,T,Traits,Comp> >::value_type      value_type;
      typedef typename
        btree_base<Key, btree_map_base<Key,T,Traits,Comp> >::const_iterator  const_iterator;
      typedef typename
        btree_base<Key, btree_map_base<Key,T,Traits,Comp> >::iterator        iterator;
  
      BOOST_STATIC_ASSERT_MSG( !boost::is_pointer<Key>::value,
        "Key must not be a pointer type");
      BOOST_STATIC_ASSERT_MSG( !boost::is_pointer<T>::value,
        "T must not be a pointer type");

      explicit btree_map()
        : btree_base<Key,btree_map_base<Key,T,Traits,Comp> >() {}

      explicit btree_map(const boost::filesystem::path& p,
        flags::bitmask flgs = flags::read_only,
        uint64_t sig = -1,  // for existing files, must match signature from creation
        const Comp& comp = Comp(),
        std::size_t node_sz = default_node_size)  // node_sz ignored if existing file
        : btree_base<Key,btree_map_base<Key,T,Traits,Comp> >(p,
            flags::user_flags(flgs) | flags::unique, sig, comp, node_sz) {}

      template <class InputIterator>
      btree_map(InputIterator begin, InputIterator end,
        const boost::filesystem::path& p,
        flags::bitmask flgs = flags::read_only,
        uint64_t sig = -1,  // for existing files, must match signature from creation
        const Comp& comp = Comp(),
        std::size_t node_sz = default_node_size)  // node_sz ignored if existing file
        : btree_base<Key,btree_map_base<Key,T,Traits,Comp> >(p,
            flags::user_flags(flgs) | flags::unique, sig, comp, node_sz)
      {
        for (; begin != end; ++begin)
        {
          btree_base<Key,btree_map_base<Key,T,Traits,Comp> >::m_insert_unique(
            begin->key(), begin->mapped_value());
        }
      }

     ~btree_map()
      {
        try {btree_base<Key, btree_map_base<Key,T,Traits,Comp> >::close();}
        catch (...) {}
      }
 
      void open(const boost::filesystem::path& p,
        flags::bitmask flgs = flags::read_only,
        uint64_t sig = -1,  // for existing files, must match signature from creation
        const Comp& comp = Comp(),
        std::size_t node_sz = default_node_size)  // node_sz ignored if existing file
      {
        btree_base<Key,btree_map_base<Key,T,Traits,Comp> >::m_open(p,
          flags::user_flags(flgs) | flags::unique, sig, comp, node_sz);
      }

      //  emplace(const Key&, const T&) special case not requiring c++0x support
      std::pair<const_iterator, bool> emplace(const Key& key, const T& mapped_value)
      {
        std::pair<const_iterator, bool> result(
          btree_base<Key,btree_map_base<Key,T,Traits,Comp> >::m_insert_unique(
            key));
        if (result.second)
          std::memcpy(const_cast<T*>(&result.first->second),
            &mapped_value, sizeof(T));
        return result;
      }

      std::pair<const_iterator, bool> insert(const value_type& value)
      {
        std::pair<const_iterator, bool> result(
          btree_base<Key,btree_map_base<Key,T,Traits,Comp> >::m_insert_unique(
            key(value)));
        if (result.second)
          std::memcpy(const_cast<T*>(&result.first->second), &mapped(value), sizeof(T));
        return result;
      }

      template <class InputIterator>
      void insert(InputIterator begin, InputIterator end)
      { 
        for (; begin != end; ++begin) 
        {
           std::pair<const_iterator, bool> result(
            btree_base<Key,btree_map_base<Key,T,Traits,Comp> >::m_insert_unique(
              key(*begin)));
          if (result.second)
            std::memcpy(const_cast<T*>(&mapped(result.first->second)),
              &mapped(*begin), sizeof(T));
        }
      }

      iterator update(iterator itr, const T& mapped_value)
      {
        std::memcpy(const_cast<T*>(&itr->second), &mapped_value, sizeof(T));
        return btree_base<Key,btree_map_base<Key,T,Traits,Comp> >::m_update(itr);
      }

     };

//--------------------------------------------------------------------------------------//
//                               class btree_multimap                                   //
//--------------------------------------------------------------------------------------//

    template <class Key,    // requires memcpyable type without pointers or references
              class T,      // requires memcpyable type without pointers or references
              class Traits = default_traits,
              class Comp = btree::less >
    class btree_multimap
      : public btree_base<Key, btree_map_base<Key,T,Traits,Comp> >
    {
    public:
      typedef typename
        btree_base<Key, btree_map_base<Key,T,Traits,Comp> >::value_type      value_type;
      typedef typename
        btree_base<Key, btree_map_base<Key,T,Traits,Comp> >::const_iterator  const_iterator;
      typedef typename
        btree_base<Key, btree_map_base<Key,T,Traits,Comp> >::iterator        iterator;

      BOOST_STATIC_ASSERT_MSG( !boost::is_pointer<Key>::value,
        "Key must not be a pointer type");
      BOOST_STATIC_ASSERT_MSG( !boost::is_pointer<T>::value,
        "T must not be a pointer type");

      // <Key,T,Comp> is required by GCC but not by VC++
      explicit btree_multimap()
        : btree_base<Key,btree_map_base<Key,T,Traits,Comp> >() {}

      explicit btree_multimap(const boost::filesystem::path& p,
        flags::bitmask flgs = flags::read_only,
        uint64_t sig = -1,  // for existing files, must match signature from creation
        const Comp& comp = Comp() ,
        std::size_t node_sz = default_node_size)  // node_sz ignored if existing file
        : btree_base<Key,btree_map_base<Key,T,Traits,Comp> >(p,
            flags::user_flags(flgs), sig, comp, node_sz) {}

      template <class InputIterator>
      btree_multimap(InputIterator begin, InputIterator end,
        const boost::filesystem::path& p,
        flags::bitmask flgs = flags::read_only,
        uint64_t sig = -1,  // for existing files, must match signature from creation
        const Comp& comp = Comp(),
        std::size_t node_sz = default_node_size)  // node_sz ignored if existing file
        : btree_base<Key,btree_map_base<Key,T,Traits,Comp> >(p,
            flags::user_flags(flgs), sig, comp, node_sz)
      {
        for (; begin != end; ++begin)
        {
          btree_base<Key,btree_map_base<Key,T,Traits,Comp> >::m_insert_non_unique(
            begin->key(), begin->mapped_value());
        }
      }

     ~btree_multimap()
      {
        try {btree_base<Key, btree_map_base<Key,T,Traits,Comp> >::close();}
        catch (...) {}
      }

      void open(const boost::filesystem::path& p,
        flags::bitmask flgs = flags::read_only,
        uint64_t sig = -1,  // for existing files, must match signature from creation
        const Comp& comp = Comp(),
        std::size_t node_sz = default_node_size)  // node_sz ignored if existing file
      {
        btree_base<Key,btree_map_base<Key,T,Traits,Comp> >::m_open(p,
          flags::user_flags(flgs), sig, comp, node_sz);
      }

      //  emplace(const Key&, const T&) special case not requiring c++0x support
      const_iterator emplace(const Key& key, const T& mapped_value)
      {
        const_iterator result(
          btree_base<Key,btree_map_base<Key,T,Traits,Comp> >::m_insert_non_unique(
            key));
        std::memcpy(const_cast<T*>(&result->second), &mapped_value, sizeof(T));
        return result;          
      }

      const_iterator insert(const value_type& value)
      {
        const_iterator result(
          btree_base<Key,btree_map_base<Key,T,Traits,Comp> >::m_insert_non_unique(
            key(value)));
        std::memcpy(const_cast<T*>(&result->second), &mapped(value), sizeof(T));
        return result;          
      }

      template <class InputIterator>
      void insert(InputIterator begin, InputIterator end)
      {
        for (; begin != end; ++begin)
        {
          const_iterator result(
            btree_base<Key,btree_map_base<Key,T,Traits,Comp> >::m_insert_non_unique(
              key(*begin)));
          std::memcpy(const_cast<T*>(&result->second), &mapped(*begin), sizeof(T));
        }
      }

      iterator update(iterator itr, const T& mapped_value)
      {
        std::memcpy(const_cast<T*>(&itr->second), &mapped_value, sizeof(T));
        return btree_base<Key,btree_map_base<Key,T,Traits,Comp> >::m_update(itr);
      }
    };


  } // namespace btree
} // namespace boost

#endif  // BOOST_BTREE_MAP_HPP
