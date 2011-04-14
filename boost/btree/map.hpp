//  boost/btree/map.hpp  ---------------------------------------------------------------//

//  Copyright Beman Dawes 2000, 2006, 2010

//  Distributed under the Boost Software License, Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt

//  See http://www.boost.org/libs/btree for documentation.

#ifndef BOOST_BTREE_MAP_HPP
#define BOOST_BTREE_MAP_HPP

#define BOOST_FILESYSTEM_VERSION 3

#include <boost/config.hpp>
#include <boost/cstdint.hpp>
#include <boost/btree/header.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/btree/detail/common.hpp>  // common to all 4 btree_* containers
#include <boost/assert.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_pointer.hpp>

namespace boost
{
  namespace btree
  {

    //  endian traits are the default since they don't require page_id_type alignment

//--------------------------------------------------------------------------------------//
//                                 class btree_map                                      //
//--------------------------------------------------------------------------------------//

    template <class Key, class T, class Traits = default_endian_traits,
              class Comp = btree::less<Key> >
    class btree_map
      : public btree_base<Key, btree_map_base<Key,T,Comp>, Traits, Comp>
    {
    public:

      BOOST_STATIC_ASSERT_MSG( !boost::is_pointer<Key>::value, "Key must not be a pointer type");
      BOOST_STATIC_ASSERT_MSG( !boost::is_pointer<T>::value, "T must not be a pointer type");

      // <Key,T,Comp> is required by GCC but not by VC++
      explicit btree_map(const Comp& comp = Comp())
        : btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>(comp) {}

      explicit btree_map(const boost::filesystem::path& p,
          flags::bitmask flgs = flags::read_only,
          std::size_t pg_sz = default_page_size,  // ignored if existing file
          const Comp& comp = Comp())
        : btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>(p,
            flags::user(flgs) | flags::unique, pg_sz, comp) {}

      template <class InputIterator>
      btree_map(InputIterator begin, InputIterator end,
        const boost::filesystem::path& p,
        flags::bitmask flgs = flags::read_only,
        std::size_t pg_sz = default_page_size,  // ignored if existing file
        const Comp& comp = Comp())
      : btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>(p,
          flags::user(flgs) | flags::unique, pg_sz, comp)
      {
        for (; begin != end; ++begin)
        {
          BOOST_ASSERT_MSG(dynamic_size(begin->key())+dynamic_size(begin->mapped_value())
            < page_size()/3, "value size too large for page size");
          btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>::m_insert_unique(
            begin->key(), begin->mapped_value());
        }
      }
 
      void open(const boost::filesystem::path& p,
        flags::bitmask flgs = flags::read_only,
        std::size_t pg_sz = default_page_size) // pg_sz ignored if existing file
      {
        btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>::m_open(p,
          flags::user(flgs) | flags::unique, pg_sz);
      }

      // typename btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>:: is required by GCC but not VC++
      std::pair<typename btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>::const_iterator, bool>
      insert(const Key& key, const T& mapped_value)
      {
        BOOST_ASSERT_MSG(dynamic_size(key)+dynamic_size(mapped_value)
          < page_size()/3, "value size too large for page size");
        return btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>::m_insert_unique(
          key, mapped_value);
      }

      template <class InputIterator>
      void insert(InputIterator begin, InputIterator end)
      { 
        for (; begin != end; ++begin) 
        {
          BOOST_ASSERT_MSG(dynamic_size(begin->key())+dynamic_size(begin->mapped_value())
            < page_size()/3, "value size too large for page size");
          btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>::m_insert_unique(
            begin->key(), begin->mapped_value());
        }
      }

       // typename btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>:: is required by GCC but not VC++
      typename btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>::iterator
      update(typename btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>::iterator itr,
        const T& mapped_value)
      {
        BOOST_ASSERT_MSG(dynamic_size(itr->key())+dynamic_size(mapped_value)
          < page_size()/3, "value size too large for page size");
        return btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>::m_update(
          itr, mapped_value);
      }

     };

//--------------------------------------------------------------------------------------//
//                               class btree_multimap                                   //
//--------------------------------------------------------------------------------------//

    template <class Key, class T, class Traits = default_endian_traits,
              class Comp = btree::less<Key> >
    class btree_multimap
      : public btree_base<Key, btree_map_base<Key,T,Comp>, Traits, Comp>
    {
    public:

      BOOST_STATIC_ASSERT_MSG( !boost::is_pointer<Key>::value, "Key must not be a pointer type");
      BOOST_STATIC_ASSERT_MSG( !boost::is_pointer<T>::value, "T must not be a pointer type");

      // <Key,T,Comp> is required by GCC but not by VC++
      explicit btree_multimap(const Comp& comp = Comp())
        : btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>(comp) {}

      explicit btree_multimap(const boost::filesystem::path& p,
          flags::bitmask flgs = flags::read_only,
          std::size_t pg_sz = default_page_size,  // ignored if existing file
          const Comp& comp = Comp())
        : btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>(p,
            flags::user(flgs), pg_sz, comp) {}

      template <class InputIterator>
      btree_multimap(InputIterator begin, InputIterator end,
          const boost::filesystem::path& p,
          flags::bitmask flgs = flags::read_only,
          std::size_t pg_sz = default_page_size,  // ignored if existing file
          const Comp& comp = Comp())
        : btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>(p,
            flags::user(flgs), pg_sz, comp)
      {
        for (; begin != end; ++begin)
        {
          BOOST_ASSERT_MSG(dynamic_size(begin->key())+dynamic_size(begin->mapped_value())
            < page_size()/3, "value size too large for page size");
          btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>::m_insert_non_unique(
            begin->key(), begin->mapped_value());
        }
      }

      void open(const boost::filesystem::path& p,
      flags::bitmask flgs = flags::read_only,
      std::size_t pg_sz = default_page_size) // pg_sz ignored if existing file
      {
        btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>::m_open(p,
          flags::user(flgs), pg_sz);
      }

      // typename btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>:: is required by GCC but not VC++
      typename btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>::const_iterator
      insert(const Key& key, const T& mapped_value)
      {
        BOOST_ASSERT_MSG(dynamic_size(key)+dynamic_size(mapped_value)
          < page_size()/3, "value size too large for page size");
        return btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>::m_insert_non_unique(
          key, mapped_value);
      }

      template <class InputIterator>
      void insert(InputIterator begin, InputIterator end)
      {
        for (; begin != end; ++begin)
        {
          BOOST_ASSERT_MSG(dynamic_size(begin->key())+dynamic_size(begin->mapped_value())
            < page_size()/3, "value size too large for page size");
          btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>::m_insert_non_unique(
            begin->key(), begin->mapped_value());
        }
      }

       // typename btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>:: is required by GCC but not VC++
      typename btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>::iterator
      update(typename btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>::iterator itr,
        const T& mapped_value)
      {
        BOOST_ASSERT_MSG(dynamic_size(itr->key())+dynamic_size(mapped_value)
          < page_size()/3, "value size too large for page size");
        return btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>::m_update(
          itr, mapped_value);
      }
    };


  } // namespace btree
} // namespace boost

#endif  // BOOST_BTREE_MAP_HPP
