//  boost/btree/set.hpp  ---------------------------------------------------------------//

//  Copyright Beman Dawes 2000, 2006, 2010

//  Distributed under the Boost Software License, Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt

//  See http://www.boost.org/libs/btree for documentation.

#ifndef BOOST_BTREE_SET_HPP
#define BOOST_BTREE_SET_HPP

#define BOOST_FILESYSTEM_VERSION 3

#include <boost/config.hpp>
#include <boost/cstdint.hpp>
#include <boost/btree/header.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/btree/detail/indirect_common.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_pointer.hpp>

namespace boost
{
  namespace btree
  {

//--------------------------------------------------------------------------------------//
//                             class indirect_btree_set                                 //
//--------------------------------------------------------------------------------------//

    template <class Key, class Traits = default_native_traits,
              class Comp = std::less<Key> >
    class indirect_btree_set
      : public indirect_btree_base<Key, indirect_btree_set_base<Key,Comp>, Traits, Comp>
    {
    public:

      BOOST_STATIC_ASSERT_MSG( boost::is_pointer<Key>::value, "Key must be a pointer type");

      // <Key,Comp> is required by GCC but not by VC++
      explicit indirect_btree_set(const Comp& comp = Comp())
        : indirect_btree_base<Key,indirect_btree_set_base<Key,Comp>,Traits,Comp>(comp) {}

      explicit indirect_btree_set(const boost::filesystem::path& p,
          flags::bitmask flgs = flags::read_only,
          std::size_t pg_sz = default_page_size,  // ignored if existing file
          const Comp& comp = Comp())
        : indirect_btree_base<Key,indirect_btree_set_base<Key,Comp>,Traits,Comp>(p,
            flags::user(flgs) | flags::key_only, pg_sz, comp) {}

      template <class InputIterator>
      indirect_btree_set(InputIterator first, InputIterator last,
        const boost::filesystem::path& p,
        flags::bitmask flgs = flags::read_only,
        std::size_t pg_sz = default_page_size,  // ignored if existing file
        const Comp& comp = Comp())
      : indirect_btree_base<Key,indirect_btree_set_base<Key,Comp>,Traits,Comp>(p,
          flags::user(flgs) | flags::key_only, pg_sz, comp)
      {
        for (; first != last; ++first)
          indirect_btree_base<Key,indirect_btree_set_base<Key,Comp>,Traits,Comp>::m_insert_unique(*first);
      }
 
      void open(const boost::filesystem::path& p,
        flags::bitmask flgs = flags::read_only,
        std::size_t pg_sz = default_page_size) // pg_sz ignored if existing file
      {
        indirect_btree_base<Key,indirect_btree_set_base<Key,Comp>,Traits,Comp>::m_open(p,
          flags::user(flgs) | flags::key_only, pg_sz);
      }

      // typename indirect_btree_base<Key,indirect_btree_set_base<Key,Comp>,Traits,Comp>:: is required by GCC but not VC++
      std::pair<typename indirect_btree_base<Key,indirect_btree_set_base<Key,Comp>,Traits,Comp>::const_iterator, bool>
      insert(const typename indirect_btree_base<Key,indirect_btree_set_base<Key,Comp>,Traits,Comp>::value_type& value)
      {
        return indirect_btree_base<Key,indirect_btree_set_base<Key,Comp>,Traits,Comp>::m_insert_unique(
          value);
      }

      template <class InputIterator>
      void insert(InputIterator first, InputIterator last)
      {
        for (; first != last; ++first) 
          indirect_btree_base<Key,indirect_btree_set_base<Key,Comp>,Traits,Comp>::m_insert_unique(*first);
      }
    };

//--------------------------------------------------------------------------------------//
//                              class indirect_btree_multiset                                    //
//--------------------------------------------------------------------------------------//

    template <class Key, class Traits = default_native_traits,
              class Comp = std::less<Key> >              
    class indirect_btree_multiset
      : public indirect_btree_base<Key, indirect_btree_set_base<Key,Comp>, Traits, Comp>
    {
    public:

      BOOST_STATIC_ASSERT_MSG( boost::is_pointer<Key>::value, "Key must be a pointer type");

      // <Key,Comp> is required by GCC but not by VC++
      explicit indirect_btree_multiset(const Comp& comp = Comp())
        : indirect_btree_base<Key,indirect_btree_set_base<Key,Comp>,Traits,Comp>(comp) {}

      explicit indirect_btree_multiset(const boost::filesystem::path& p,
          flags::bitmask flgs = flags::read_only,
          std::size_t pg_sz = default_page_size,  // ignored if existing file
          const Comp& comp = Comp())
        : indirect_btree_base<Key,indirect_btree_set_base<Key,Comp>,Traits,Comp>(p,
            flags::user(flgs) | flags::key_only | flags::multi, pg_sz, comp) {}

      template <class InputIterator>
      indirect_btree_multiset(InputIterator first, InputIterator last,
        const boost::filesystem::path& p,
        flags::bitmask flgs = flags::read_only,
        std::size_t pg_sz = default_page_size,  // ignored if existing file
        const Comp& comp = Comp())
      : indirect_btree_base<Key,indirect_btree_set_base<Key,Comp>,Traits,Comp>(p,
          flags::user(flgs) | flags::key_only | flags::multi, pg_sz, comp)
      {
        for (; first != last; ++first)
          indirect_btree_base<Key,indirect_btree_set_base<Key,Comp>,Traits,Comp>::m_insert_non_unique(
            *first);
      }

      void open(const boost::filesystem::path& p,
        flags::bitmask flgs = flags::read_only,
        std::size_t pg_sz = default_page_size) // pg_sz ignored if existing file
      {
         indirect_btree_base<Key,indirect_btree_set_base<Key,Comp>,Traits,Comp>::m_open(p,
          flags::user(flgs) | flags::key_only | flags::multi, pg_sz);
      }

      // typename indirect_btree_base<Key,indirect_btree_set_base<Key,Comp>,Traits,Comp>:: is required by GCC but not VC++
      typename indirect_btree_base<Key,indirect_btree_set_base<Key,Comp>,Traits,Comp>::const_iterator
      insert(const typename indirect_btree_base<Key,indirect_btree_set_base<Key,Comp>,Traits,Comp>::value_type& value)
      {
        return indirect_btree_base<Key,indirect_btree_set_base<Key,Comp>,Traits,Comp>::m_insert_non_unique(
          value);
      }

      template <class InputIterator>
      void insert(InputIterator first, InputIterator last)
      {
        for (; first != last; ++first) 
          indirect_btree_base<Key,indirect_btree_set_base<Key,Comp>,Traits,Comp>::m_insert_non_unique(
            *first);
      }
    };

  } // namespace btree
} // namespace boost

#endif  // BOOST_BTREE_SET_HPP