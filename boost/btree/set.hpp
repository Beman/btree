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
#include <boost/btree/detail/common.hpp> // common to all 4 btree_* containers
#include <boost/assert.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_pointer.hpp>

namespace boost
{
  namespace btree
  {

    //  endian traits are the default since they don't require page_id_type alignment

//--------------------------------------------------------------------------------------//
//                                class btree_set                                       //
//--------------------------------------------------------------------------------------//

    template <class Key, class Traits = default_endian_traits,
              class Comp = btree::less<Key> >
    class btree_set
      : public btree_base<Key, btree_set_base<Key,Comp>, Traits, Comp>
    {
    public:

      BOOST_STATIC_ASSERT_MSG( !boost::is_pointer<Key>::value, "Key must not be a pointer type");

      // <Key,Comp> is required by GCC but not by VC++
      explicit btree_set(const Comp& comp = Comp())
        : btree_base<Key,btree_set_base<Key,Comp>,Traits,Comp>(comp) {}

      explicit btree_set(const boost::filesystem::path& p,
          flags::bitmask flgs = flags::read_only,
          std::size_t pg_sz = default_page_size,  // ignored if existing file
          const Comp& comp = Comp())
        : btree_base<Key,btree_set_base<Key,Comp>,Traits,Comp>(p,
            flags::user(flgs) | flags::key_only | flags::unique, pg_sz, comp) {}

      template <class InputIterator>
      btree_set(InputIterator begin, InputIterator end,
        const boost::filesystem::path& p,
        flags::bitmask flgs = flags::read_only,
        std::size_t pg_sz = default_page_size,  // ignored if existing file
        const Comp& comp = Comp())
      : btree_base<Key,btree_set_base<Key,Comp>,Traits,Comp>(p,
          flags::user(flgs) | flags::key_only | flags::unique, pg_sz, comp)
      {
        for (; begin != end; ++begin)
        {
          btree_base<Key,btree_set_base<Key,Comp>,Traits,Comp>::m_insert_unique(
            *begin, *begin);
        }
      }
 
      void open(const boost::filesystem::path& p,
        flags::bitmask flgs = flags::read_only,
        std::size_t pg_sz = default_page_size) // pg_sz ignored if existing file
      {
        btree_base<Key,btree_set_base<Key,Comp>,Traits,Comp>::m_open(p,
          flags::user(flgs) | flags::key_only | flags::unique, pg_sz);
      }

      // typename btree_base<Key,btree_set_base<Key,Comp>,Traits,Comp>:: is required by GCC but not VC++
      std::pair<typename btree_base<Key,btree_set_base<Key,Comp>,Traits,Comp>::const_iterator, bool>
      insert(const typename btree_base<Key,btree_set_base<Key,Comp>,Traits,Comp>::value_type& value)
      {
        return btree_base<Key,btree_set_base<Key,Comp>,Traits,Comp>::m_insert_unique(
          value, value);
      }

      template <class InputIterator>
      void insert(InputIterator begin, InputIterator end)
      {
        for (; begin != end; ++begin)
        {
          btree_base<Key,btree_set_base<Key,Comp>,Traits,Comp>::m_insert_unique(
            *begin, *begin);
        }
      }
    };

//--------------------------------------------------------------------------------------//
//                              class btree_multiset                                    //
//--------------------------------------------------------------------------------------//

    template <class Key, class Traits = default_endian_traits,
              class Comp = btree::less<Key> >              
    class btree_multiset
      : public btree_base<Key, btree_set_base<Key,Comp>, Traits, Comp>
    {
    public:

      BOOST_STATIC_ASSERT_MSG( !boost::is_pointer<Key>::value, "Key must not be a pointer type");

      // <Key,Comp> is required by GCC but not by VC++
      explicit btree_multiset(const Comp& comp = Comp())
        : btree_base<Key,btree_set_base<Key,Comp>,Traits,Comp>(comp) {}

      explicit btree_multiset(const boost::filesystem::path& p,
          flags::bitmask flgs = flags::read_only,
          std::size_t pg_sz = default_page_size,  // ignored if existing file
          const Comp& comp = Comp())
        : btree_base<Key,btree_set_base<Key,Comp>,Traits,Comp>(p,
            flags::user(flgs) | flags::key_only, pg_sz, comp) {}

      template <class InputIterator>
      btree_multiset(InputIterator begin, InputIterator end,
        const boost::filesystem::path& p,
        flags::bitmask flgs = flags::read_only,
        std::size_t pg_sz = default_page_size,  // ignored if existing file
        const Comp& comp = Comp())
      : btree_base<Key,btree_set_base<Key,Comp>,Traits,Comp>(p,
          flags::user(flgs) | flags::key_only, pg_sz, comp)
      {
        for (; begin != end; ++begin)
        {
          btree_base<Key,btree_set_base<Key,Comp>,Traits,Comp>::m_insert_non_unique(
            *begin, *begin);
        }
      }

      void open(const boost::filesystem::path& p,
        flags::bitmask flgs = flags::read_only,
        std::size_t pg_sz = default_page_size) // pg_sz ignored if existing file
      {
         btree_base<Key,btree_set_base<Key,Comp>,Traits,Comp>::m_open(p,
          flags::user(flgs) | flags::key_only, pg_sz);
      }

      // typename btree_base<Key,btree_set_base<Key,Comp>,Traits,Comp>:: is required by GCC but not VC++
      typename btree_base<Key,btree_set_base<Key,Comp>,Traits,Comp>::const_iterator
      insert(const typename btree_base<Key,btree_set_base<Key,Comp>,Traits,Comp>::value_type& value)
      {
        return btree_base<Key,btree_set_base<Key,Comp>,Traits,Comp>::m_insert_non_unique(
          value, value);
      }

      template <class InputIterator>
      void insert(InputIterator begin, InputIterator end)
      {
        for (; begin != end; ++begin) 
        {
          btree_base<Key,btree_set_base<Key,Comp>,Traits,Comp>::m_insert_non_unique(
            *begin, *begin);
        }
      }
    };

  } // namespace btree
} // namespace boost

#endif  // BOOST_BTREE_SET_HPP
