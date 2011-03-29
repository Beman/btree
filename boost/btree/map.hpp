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
#include <boost/btree/detail/common.hpp>  // interface common to btree_map and btree_set

namespace boost
{
  namespace btree
  {

//--------------------------------------------------------------------------------------//
//                                 class btree_map                                      //
//--------------------------------------------------------------------------------------//

    template <class Key, class T, class Traits = default_native_traits,
              class Comp = std::less<Key> >
    class btree_map
      : public btree_base<Key, btree_map_base<Key,T,Comp>, Traits, Comp>
    {
    public:
      typedef T  mapped_type;

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
      btree_map(InputIterator first, InputIterator last,
        const boost::filesystem::path& p,
        flags::bitmask flgs = flags::read_only,
        std::size_t pg_sz = default_page_size,  // ignored if existing file
        const Comp& comp = Comp())
      : btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>(p,
          flags::user(flgs) | flags::unique, pg_sz, comp)
      {
        for (; first != last; ++first)
          btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>::m_insert_unique(*first);
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
      insert(const typename btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>::value_type& value)
      {
        return btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>::m_insert_unique(
          value);
      }

      template <class InputIterator>
      void insert(InputIterator first, InputIterator last)
      { 
        for (; first != last; ++first) 
          btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>::m_insert_unique(*first);
      }
    };

//--------------------------------------------------------------------------------------//
//                               class btree_multimap                                   //
//--------------------------------------------------------------------------------------//

    template <class Key, class T, class Traits = default_native_traits,
              class Comp = std::less<Key> >
    class btree_multimap
      : public btree_base<Key, btree_map_base<Key,T,Comp>, Traits, Comp>
    {
    public:
      typedef T  mapped_type;

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
      btree_multimap(InputIterator first, InputIterator last,
          const boost::filesystem::path& p,
          flags::bitmask flgs = flags::read_only,
          std::size_t pg_sz = default_page_size,  // ignored if existing file
          const Comp& comp = Comp())
        : btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>(p,
            flags::user(flgs), pg_sz, comp)
      {
        for (; first != last; ++first)
          btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>::m_insert_non_unique(
            *first);
      }

      void open(const boost::filesystem::path& p,
      flags::bitmask flgs = flags::read_only,
      std::size_t pg_sz = default_page_size) // pg_sz ignored if existing file
      {
        btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>::m_open(p,
          flgs, pg_sz);
      }

      // typename btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>:: is required by GCC but not VC++
      typename btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>::const_iterator
      insert(const typename btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>::value_type& value)
      {
        return btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>::m_insert_non_unique(
          value);
      }

      template <class InputIterator>
      void insert(InputIterator first, InputIterator last)
      {
        for (; first != last; ++first)
          btree_base<Key,btree_map_base<Key,T,Comp>,Traits,Comp>::m_insert_non_unique(
            *first);
      }
    };


  } // namespace btree
} // namespace boost

#endif  // BOOST_BTREE_MAP_HPP
