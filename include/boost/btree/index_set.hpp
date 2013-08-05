//  boost/btree/index_set.hpp  ---------------------------------------------------------//

//  Copyright Beman Dawes 2000, 2006, 2010, 2013

//  Distributed under the Boost Software License, Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_BTREE_INDEX_SET_HPP
#define BOOST_BTREE_INDEX_SET_HPP

#include <boost/filesystem.hpp>
#include <boost/btree/index_helpers.hpp>
#include <boost/btree/set.hpp>
#include <boost/btree/detail/index_bases.hpp>
#include <boost/assert.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_pointer.hpp>

namespace boost
{
namespace btree
{

//--------------------------------------------------------------------------------------//
//                                 class index_set                                      //
//--------------------------------------------------------------------------------------//

template <class Key,    // shall be trivially copyable type; see std 3.9 [basic.types]
          class BtreeTraits = btree::default_traits,
          class Compare = btree::less,
          class IndexTraits = btree::default_index_traits<Key> >
class index_set
  : public index_base<index_set_base<Key,BtreeTraits,Compare,IndexTraits> >
{
private:
  typedef typename
    index_base<index_set_base<Key,BtreeTraits,Compare,IndexTraits> >  base;
public:
  typedef Key                            key_type;
  typedef Key                            value_type;

  typedef BtreeTraits                    btree_traits;
  typedef IndexTraits                    index_traits;

  typedef Compare                        key_compare;
  typedef Compare                        value_compare;

  typedef typename base::size_type       size_type;
  typedef typename base::reference       reference;
  typedef typename base::iterator        iterator;
  typedef typename base::const_iterator  const_iterator;
  typedef typename base::file_position   file_position;
  typedef typename base::index_key       index_key;

  index_set()
    : index_base<index_set_base<Key,BtreeTraits,Compare,IndexTraits> >() {}

  index_set(const path_type& file_pth,
            file_size_type reserv,
            const path_type& index_pth,
            flags::bitmask flgs = flags::read_only,
            uint64_t sig = -1,  // for existing files, must match creation signature
            std::size_t node_sz = default_node_size,  // ignored if existing file
            const Compare& comp = Compare())
  {
    index_base<index_set_base<Key,BtreeTraits,Compare,IndexTraits> >::
    open(file_pth, reserv, index_pth, flgs, sig, node_sz, comp);
  }

  index_set(file_ptr_type flat_file,            
            const path_type& index_pth,
            flags::bitmask flgs = flags::read_only,
            uint64_t sig = -1,  // for existing files, must match creation signature
            std::size_t node_sz = default_node_size,  // ignored if existing file
            const Compare& comp = Compare())
  {
    index_base<index_set_base<Key,BtreeTraits,Compare,IndexTraits> >::
    open(flat_file, index_pth, flgs, sig, node_sz, comp);
  }

  void open(const path_type& file_pth,
            file_size_type reserv,
            const path_type& index_pth,
            flags::bitmask flgs = flags::read_only,
            uint64_t sig = -1,  // for existing files, must match creation signature
            std::size_t node_sz = default_node_size,  // ignored if existing file
            const Compare& comp = Compare())
  {
    index_base<index_set_base<Key,BtreeTraits,Compare,IndexTraits> >::
    open(file_pth, reserv, index_pth, flgs, sig, node_sz, comp);
  }

  void open(file_ptr_type flat_file,            
            const path_type& index_pth,
            flags::bitmask flgs = flags::read_only,
            uint64_t sig = -1,  // for existing files, must match creation signature
            std::size_t node_sz = default_node_size,  // ignored if existing file
            const Compare& comp = Compare())
  {
    index_base<index_set_base<Key,BtreeTraits,Compare,IndexTraits> >::
    open(flat_file, index_pth, flgs, sig, node_sz, comp);
  }

  //  modifiers

  file_position push_back(const key_type& x)
  // Effects: unconditional push_back into file(); index unaffected
  {
    file_position pos = file()->file_size();
    std::size_t element_sz = index_traits::flat_size(x);
    file()->increment_file_size(element_sz);
    index_traits::build_flat_element(x, file()->data<char>()+pos, element_sz);
    return pos;
  }

  std::pair<const_iterator, bool>
    insert_file_position(file_position pos)
  {
    BOOST_ASSERT(!read_only());
    std::pair<index_type::const_iterator, bool>
      result(m_set.insert(index_key(pos)));
    return std::pair<const_iterator, bool>(
      const_iterator(result.first, file()), result.second);
  }

  std::pair<const_iterator, bool>
    insert(const value_type& value)
  //  Effects: if !find(k) then insert_file_position(push_back(value));
  {
    BOOST_ASSERT(!read_only());
    if (find(value) == end())
    {
      std::pair<const_iterator, bool> result(insert_file_position(push_back(value)));
      BOOST_ASSERT(result.second);
      return result;
    }
    return std::pair<const_iterator, bool>(const_iterator(), false);
  }
};

////--------------------------------------------------------------------------------------//
////                              class index_multiset                                    //
////--------------------------------------------------------------------------------------//
//
//    template <class Key,    // shall be trivially copyable type; see std 3.9 [basic.types]
//              class BtreeTraits = index_traits<btree::default_node_traits, btree::less> >              
//    class index_multiset
//      : public index_base<Key, index_set_base<Key,BtreeTraits> >
//    {
//    public:
//      typedef typename
//        index_base<Key, index_set_base<Key,BtreeTraits> >::value_type      value_type;
//      typedef typename
//        index_base<Key, index_set_base<Key,BtreeTraits> >::const_iterator  const_iterator;
//      typedef typename
//        index_base<Key, index_set_base<Key,BtreeTraits> >::iterator        iterator;
//
//      BOOST_STATIC_ASSERT_MSG( !boost::is_pointer<Key>::value,
//        "Key must not be a pointer type");
//
//      // <Key,BtreeTraits> is required by GCC but not by VC++
//      explicit index_multiset()
//        : index_base<Key,index_set_base<Key,BtreeTraits> >() {}
//
//      explicit index_multiset(const boost::filesystem::path& p,
//        flags::bitmask flgs = flags::read_only,
//        uint64_t sig = -1,  // for existing files, must match signature from creation
//        std::size_t node_sz = default_node_size,  // ignored if existing file
//        const BtreeTraits& traits = BtreeTraits())
//        : index_base<Key,index_set_base<Key,BtreeTraits> >(p,
//            flags::open_flags(flgs) | flags::key_only, sig, node_sz, traits) {}
//
//      template <class InputIterator>
//      index_multiset(InputIterator begin, InputIterator end,
//        const boost::filesystem::path& p,
//        flags::bitmask flgs = flags::read_only,
//        uint64_t sig = -1,  // for existing files, must match signature from creation
//        std::size_t node_sz = default_node_size,  // ignored if existing file
//        const BtreeTraits& traits = BtreeTraits())
//      : index_base<Key,index_set_base<Key,BtreeTraits> >(p,
//          flags::open_flags(flgs) | flags::key_only, sig, node_sz, traits)
//      {
//        for (; begin != end; ++begin)
//        {
//          index_base<Key,index_set_base<Key,BtreeTraits> >::m_insert_non_unique(
//            *begin, *begin);
//        }
//      }
//
//      void open(const boost::filesystem::path& p,
//        flags::bitmask flgs = flags::read_only,
//        uint64_t sig = -1,  // for existing files, must match signature from creation
//        std::size_t node_sz = default_node_size) // node_sz ignored if existing file
//      {
//         index_base<Key,index_set_base<Key,BtreeTraits> >::m_open(p,
//          flags::open_flags(flgs) | flags::key_only, sig, node_sz);
//      }
//
//      //  emplace(const Key&) special case not requiring c++0x support
//      const_iterator emplace(const value_type& value)
//      {
//        return
//          index_base<Key,index_set_base<Key,BtreeTraits> >::m_insert_non_unique(
//            value);
//      }
//
//      const_iterator insert(const value_type& value)
//      {
//        return 
//          index_base<Key,index_set_base<Key,BtreeTraits> >::m_insert_non_unique(
//            value);
//      }
//
//      template <class InputIterator>
//      void insert(InputIterator begin, InputIterator end)
//      {
//        for (; begin != end; ++begin) 
//        {
//          index_base<Key,index_set_base<Key,BtreeTraits> >::m_insert_non_unique(
//            *begin);
//        }
//      }
//    };

  } // namespace btree
} // namespace boost

#endif  // BOOST_BTREE_INDEX_SET_HPP
