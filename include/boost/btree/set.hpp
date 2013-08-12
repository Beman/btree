//  boost/btree/set.hpp  ---------------------------------------------------------------//

//  Copyright Beman Dawes 2000, 2006, 2010, 2013

//  Distributed under the Boost Software License, Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt

//  See http://www.boost.org/libs/btree for documentation.

#ifndef BOOST_BTREE_SET_HPP
#define BOOST_BTREE_SET_HPP

#include <boost/config.hpp>
#include <boost/cstdint.hpp>
#include <boost/btree/header.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/btree/detail/btree_bases.hpp> // common to all 4 btree_* containers
#include <boost/assert.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_pointer.hpp>

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
//                                class btree_set                                       //
//--------------------------------------------------------------------------------------//

    template <class Key,    // shall be trivially copyable type; see std 3.9 [basic.types]
              class Traits = default_traits,
              class Comp = btree::less>
    class btree_set
      : public btree_base<Key, btree_set_base<Key,Traits,Comp> >
    {
    public:
      typedef typename
        btree_base<Key, btree_set_base<Key,Traits,Comp> >::value_type      value_type;
      typedef typename
        btree_base<Key, btree_set_base<Key,Traits,Comp> >::const_iterator  const_iterator;
      typedef typename
        btree_base<Key, btree_set_base<Key,Traits,Comp> >::iterator        iterator;

      BOOST_STATIC_ASSERT_MSG( !boost::is_pointer<Key>::value,
        "Key must not be a pointer type");

      // <Key,Traits> is required by GCC but not by VC++
      explicit btree_set()
        : btree_base<Key,btree_set_base<Key,Traits,Comp> >() {}

      explicit btree_set(const boost::filesystem::path& p,
        flags::bitmask flgs = flags::read_only,
        uint64_t sig = -1,  // for existing files, must match signature from creation
        const Comp& comp = Comp(),
        std::size_t node_sz = default_node_size)  // node_sz ignored if existing file
        : btree_base<Key,btree_set_base<Key,Traits,Comp> >(p,
            flags::open_flags(flgs) | flags::key_only | flags::unique,
            sig, comp, node_sz) {}

      template <class InputIterator>
      btree_set(InputIterator begin, InputIterator end,
        const boost::filesystem::path& p,
        flags::bitmask flgs = flags::read_only,
        uint64_t sig = -1,  // for existing files, must match signature from creation
        const Comp& comp = Comp(),
        std::size_t node_sz = default_node_size)  // node_sz ignored if existing file
      : btree_base<Key,btree_set_base<Key,Traits,Comp> >(p,
          flags::open_flags(flgs) | flags::key_only | flags::unique,
          sig, comp, node_sz)
      {
        for (; begin != end; ++begin)
        {
          btree_base<Key,btree_set_base<Key,Traits,Comp> >::m_insert_unique(
            *begin, *begin);
        }
      }

     ~btree_set()
      {
        try {btree_base<Key, btree_set_base<Key,Traits,Comp> >::close();}
        catch (...) {}
      }

 
      void open(const boost::filesystem::path& p,
        flags::bitmask flgs = flags::read_only,
        uint64_t sig = -1,  // for existing files, must match signature from creation
        const Comp& comp = Comp(),
        std::size_t node_sz = default_node_size)  // node_sz ignored if existing file
      {
        btree_base<Key,btree_set_base<Key,Traits,Comp> >::m_open(p,
          flags::open_flags(flgs) | flags::key_only | flags::unique, sig,
          comp, node_sz);
      }

      //  emplace() special case not requiring c++0x support
      std::pair<const_iterator, bool>
      emplace(const value_type& value)
      {
        return btree_base<Key,btree_set_base<Key,Traits,Comp> >::m_insert_unique(
          value);
      }

      std::pair<const_iterator, bool>
      insert(const value_type& value)
      {
        return btree_base<Key,btree_set_base<Key,Traits,Comp> >::m_insert_unique(
          value);
      }

      template <class InputIterator>
      void insert(InputIterator begin, InputIterator end)
      {
        for (; begin != end; ++begin)
        {
          btree_base<Key,btree_set_base<Key,Traits,Comp> >::m_insert_unique(
            *begin);
        }
      }
    };

//--------------------------------------------------------------------------------------//
//                              class btree_multiset                                    //
//--------------------------------------------------------------------------------------//

    template <class Key,    // shall be trivially copyable type; see std 3.9 [basic.types]
              class Traits = default_traits,
              class Comp = btree::less>              
    class btree_multiset
      : public btree_base<Key, btree_set_base<Key,Traits,Comp> >
    {
    public:
      typedef typename
        btree_base<Key, btree_set_base<Key,Traits,Comp> >::value_type      value_type;
      typedef typename
        btree_base<Key, btree_set_base<Key,Traits,Comp> >::const_iterator  const_iterator;
      typedef typename
        btree_base<Key, btree_set_base<Key,Traits,Comp> >::iterator        iterator;

      BOOST_STATIC_ASSERT_MSG( !boost::is_pointer<Key>::value,
        "Key must not be a pointer type");

      // <Key,Traits> is required by GCC but not by VC++
      explicit btree_multiset()
        : btree_base<Key,btree_set_base<Key,Traits,Comp> >() {}

      explicit btree_multiset(const boost::filesystem::path& p,
        flags::bitmask flgs = flags::read_only,
        uint64_t sig = -1,  // for existing files, must match signature from creation
        const Comp& comp = Comp(),
        std::size_t node_sz = default_node_size)  // ignored if existing file
        : btree_base<Key,btree_set_base<Key,Traits,Comp> >(p,
            flags::open_flags(flgs) | flags::key_only, sig, comp, node_sz) {}

      template <class InputIterator>
      btree_multiset(InputIterator begin, InputIterator end,
        const boost::filesystem::path& p,
        flags::bitmask flgs = flags::read_only,
        uint64_t sig = -1,  // for existing files, must match signature from creation
        const Comp& comp = Comp(),
        std::size_t node_sz = default_node_size)  // ignored if existing file
      : btree_base<Key,btree_set_base<Key,Traits,Comp> >(p,
          flags::open_flags(flgs) | flags::key_only, sig, comp, node_sz)
      {
        for (; begin != end; ++begin)
        {
          btree_base<Key,btree_set_base<Key,Traits,Comp> >::m_insert_non_unique(
            *begin, *begin);
        }
      }

     ~btree_multiset()
      {
        try {btree_base<Key, btree_set_base<Key,Traits,Comp> >::close();}
        catch (...) {}
      }

      void open(const boost::filesystem::path& p,
        flags::bitmask flgs = flags::read_only,
        uint64_t sig = -1,  // for existing files, must match signature from creation
        const Comp& comp = Comp(), 
        std::size_t node_sz = default_node_size)  // node_sz ignored if existing file
      {
         btree_base<Key,btree_set_base<Key,Traits,Comp> >::m_open(p,
          flags::open_flags(flgs) | flags::key_only, sig, comp, node_sz);
      }

      //  emplace(const Key&) special case not requiring c++0x support
      const_iterator emplace(const value_type& value)
      {
        return
          btree_base<Key,btree_set_base<Key,Traits,Comp> >::m_insert_non_unique(
            value);
      }

      const_iterator insert(const value_type& value)
      {
        return 
          btree_base<Key,btree_set_base<Key,Traits,Comp> >::m_insert_non_unique(
            value);
      }

      template <class InputIterator>
      void insert(InputIterator begin, InputIterator end)
      {
        for (; begin != end; ++begin) 
        {
          btree_base<Key,btree_set_base<Key,Traits,Comp> >::m_insert_non_unique(
            *begin);
        }
      }
    };

  } // namespace btree
} // namespace boost

#endif  // BOOST_BTREE_SET_HPP
