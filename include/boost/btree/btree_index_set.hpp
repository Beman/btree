//  boost/btree/btree_index_set.hpp.hpp  -----------------------------------------------//

//  Copyright Beman Dawes 2000, 2006, 2010, 2013

//  Distributed under the Boost Software License, Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_BTREE_INDEX_SET_HPP
#define BOOST_BTREE_INDEX_SET_HPP

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable : 4996)   // equivalent to -D_SCL_SECURE_NO_WARNINGS
#endif

#include <boost/filesystem.hpp>
#include <boost/btree/index_helpers.hpp>
#include <boost/btree/btree_set.hpp>
#include <boost/btree/detail/index_bases.hpp>
#include <boost/assert.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_pointer.hpp>

namespace boost
{
namespace btree
{

//--------------------------------------------------------------------------------------//
//                             class btree_index_set                                    //
//--------------------------------------------------------------------------------------//

template <class Key,    // requires memcpyable type without pointers or references
          class BtreeTraits = btree::default_traits,
          class Compare = btree::less>
class btree_index_set
  : public index_base<index_set_base<Key,BtreeTraits,Compare> >
{
private:
  typedef typename
    btree::index_base<index_set_base<Key,BtreeTraits,Compare> >  base;
protected:
  typedef
    typename base::index_position_type   index_position_type;
public:
  typedef Key                            key_type;
  typedef Key                            value_type;

  typedef BtreeTraits                    btree_traits;
  //typedef IndexTraits                    index_traits;

  typedef Compare                        key_compare;
  typedef Compare                        value_compare;

  typedef typename base::size_type       size_type;
  typedef typename base::reference       reference;
  typedef typename base::iterator        iterator;
  typedef typename base::const_iterator  const_iterator;

  typedef typename base::file_type       file_type;
  typedef typename base::file_ptr_type   file_ptr_type;
  typedef typename base::file_size_type  file_size_type;
  typedef typename base::file_position   file_position;

  typedef boost::filesystem::path        path_type;


  btree_index_set() : base() {}

  btree_index_set(const path_type& index_pth,
            const path_type& file_pth,
            flags::bitmask flgs = flags::read_only,
            uint64_t sig = -1,  // for existing files, must match creation signature
            const Compare& comp = Compare(),
            std::size_t node_sz = default_node_size)  // node_sz ignored if existing file
  {
    base::open(index_pth, file_pth, flgs, sig, comp, node_sz);
  }

  btree_index_set(const path_type& index_pth,
            file_ptr_type flat_file,            
            flags::bitmask flgs = flags::read_only,
            uint64_t sig = -1,  // for existing files, must match creation signature
            const Compare& comp = Compare(),
            std::size_t node_sz = default_node_size)  // node_sz ignored if existing file
  {
    base::open(index_pth, flat_file,flgs, sig, comp, node_sz);
  }

  btree_index_set(const path_type& base_path,
            flags::bitmask flgs = flags::read_only,
            uint64_t sig = -1,  // for existing files, must match creation signature
            const Compare& comp = Compare(),
            std::size_t node_sz = default_node_size)  // node_sz ignored if existing file
  {
    open(base_path, flgs, sig, comp, node_sz);
  }

  void open(const path_type& index_pth,
            const path_type& file_pth,
            flags::bitmask flgs = flags::read_only,
            uint64_t sig = -1,  // for existing files, must match creation signature
            const Compare& comp = Compare(),
            std::size_t node_sz = default_node_size)  // node_sz ignored if existing file
  {
    base::open(index_pth, file_pth, flgs, sig, comp, node_sz);
  }

  void open(const path_type& index_pth,
            file_ptr_type flat_file,            
            flags::bitmask flgs = flags::read_only,
            uint64_t sig = -1,  // for existing files, must match creation signature
            const Compare& comp = Compare(),
            std::size_t node_sz = default_node_size)  // node_sz ignored if existing file
  {
    base::open(index_pth, flat_file, flgs, sig, comp, node_sz);
  }

  void open(const path_type& base_path,
            flags::bitmask flgs = flags::read_only,
            uint64_t sig = -1,  // for existing files, must match creation signature
            const Compare& comp = Compare(),
            std::size_t node_sz = default_node_size)  // node_sz ignored if existing file
  {
    path_type index_pth(base_path);
    index_pth += ".ndx";
    path_type file_pth(base_path);
    file_pth += ".dat";
    base::open(index_pth, file_pth, flgs, sig, comp, node_sz);
  }

  //  modifiers

  file_position push_back(const key_type& x)
  // Effects: unconditional push_back into file(); index unaffected
  {
    file_position pos = base::file()->file_size();
    std::size_t element_sz = IndexTraits<Key>::size(x);
    base::file()->increment_file_size(element_sz);
    IndexTraits<Key>::build_flat_element(x, base::file()->template data<char>() + pos,
      element_sz);
    return pos;
  }

  std::pair<const_iterator, bool>
    insert_file_position(file_position pos)
  {
    BOOST_ASSERT((base::flags() & flags::read_only) == 0);
    std::pair<typename base::index_type::const_iterator, bool>
      result(base::m_index_btree.insert(index_position_type(pos)));
    return std::pair<const_iterator, bool>(
      const_iterator(result.first, base::file()), result.second);
  }

  std::pair<const_iterator, bool>
    insert(const value_type& value)
  //  Effects: if !find(k) then insert_file_position(push_back(value));
  {
    BOOST_ASSERT((base::flags() & flags::read_only) == 0);
    if (base::find(value) == base::end())
    {
      std::pair<const_iterator, bool> result(insert_file_position(push_back(value)));
      BOOST_ASSERT(result.second);
      return result;
    }
    return std::pair<const_iterator, bool>(const_iterator(), false);
  }
};

//--------------------------------------------------------------------------------------//
//                            class btree_index_multiset                                //
//--------------------------------------------------------------------------------------//

template <class Key,    // requires memcpyable type without pointers or references
          class BtreeTraits = btree::default_traits,
          class Compare = btree::less>
class btree_index_multiset
  : public index_base<index_multiset_base<Key,BtreeTraits,Compare> >
{
private:
  typedef typename
    btree::index_base<index_multiset_base<Key,BtreeTraits,Compare> >  base;
public:
  typedef Key                            key_type;
  typedef Key                            value_type;

  typedef BtreeTraits                    btree_traits;
  //typedef IndexTraits                    index_traits;

  typedef Compare                        key_compare;
  typedef Compare                        value_compare;

  typedef typename base::size_type       size_type;
  typedef typename base::reference       reference;
  typedef typename base::iterator        iterator;
  typedef typename base::const_iterator  const_iterator;
  typedef typename base::index_position_type       index_position_type;

  typedef boost::filesystem::path        path;
  typedef typename base::file_type       file_type;
  typedef typename base::file_ptr_type   file_ptr_type;
  typedef typename base::file_size_type  file_size_type;
  typedef typename base::file_position   file_position;


  btree_index_multiset() : base() {}

  btree_index_multiset(const path& index_pth,
            const path& file_pth,
            flags::bitmask flgs = flags::read_only,
            uint64_t sig = -1,  // for existing files, must match creation signature
            const Compare& comp = Compare(),
            std::size_t node_sz = default_node_size)  // node_sz ignored if existing file
  {
    base::open(index_pth, file_pth, flgs, sig, comp, node_sz);
  }

  btree_index_multiset(const path& index_pth,
            file_ptr_type flat_file,            
            flags::bitmask flgs = flags::read_only,
            uint64_t sig = -1,  // for existing files, must match creation signature
            const Compare& comp = Compare(),
            std::size_t node_sz = default_node_size)  // node_sz ignored if existing file
  {
    base::open(index_pth, flat_file,flgs, sig, comp, node_sz);
  }

  void open(const path& index_pth,
            const path& file_pth,
            flags::bitmask flgs = flags::read_only,
            uint64_t sig = -1,  // for existing files, must match creation signature
            const Compare& comp = Compare(),
            std::size_t node_sz = default_node_size)  // node_sz ignored if existing file
  {
    base::open(index_pth, file_pth, flgs, sig, comp, node_sz);
  }

  void open(const path& index_pth,
            file_ptr_type flat_file,            
            flags::bitmask flgs = flags::read_only,
            uint64_t sig = -1,  // for existing files, must match creation signature
            const Compare& comp = Compare(),
            std::size_t node_sz = default_node_size)  // node_sz ignored if existing file
  {
    base::open(index_pth, flat_file, flgs, sig, comp, node_sz);
  }

  //  modifiers

  file_position push_back(const key_type& x)
  // Effects: unconditional push_back into file(); index unaffected
  {
    file_position pos = base::file()->file_size();
    std::size_t element_sz = IndexTraits<Key>::size(x);
    base::file()->increment_file_size(element_sz);
    IndexTraits<Key>::build_flat_element(x, base::file()->template data<char>() + pos,
      element_sz);
    return pos;
  }

  const_iterator insert_file_position(file_position pos)
  {
    BOOST_ASSERT((base::flags() & flags::read_only) == 0);
    typename base::index_type::const_iterator
      result(base::m_index_btree.insert(index_position_type(pos)));
    return const_iterator(result, base::file());
  }

  const_iterator insert(const value_type& value)
  {
    BOOST_ASSERT((base::flags() & flags::read_only) == 0);
    return insert_file_position(push_back(value));
  }
};

  } // namespace btree
} // namespace boost

#ifdef _MSC_VER
#  pragma warning(pop) 
#endif

#endif  // BOOST_BTREE_INDEX_SET_HPP
