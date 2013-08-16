//  boost/btree/detail/index_bases.hpp  ------------------------------------------------//

//  Copyright Beman Dawes 2000, 2006, 2010, 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  See http://www.boost.org/libs/btree for documentation.

#ifndef BOOST_BTREE_INDEX_BASES_HPP
#define BOOST_BTREE_INDEX_BASES_HPP

#include <boost/btree/helpers.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/btree/mmff.hpp>
#include <boost/btree/set.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/assert.hpp>

/*

  TODO:

  *  Implement maps.

  *  Get map_example2 working. Depends on index_map.

  *  Add emplace()

  *  template <class K> operations fuctions per C++14.

  *  verify dereferencing the end iterator assert fires correctly.

  *  sets, maps, missing close(). Might need an argument that says what to close. The index?
     the flat file? Both?

  *  Rename in index_helpers.hpp to index_traits.hpp?

  *  Reserve should round up to memory map page size boundary.

*/

namespace boost
{
namespace btree
{
namespace detail
{
  template <class T, class Pos, class Compare, class IndexTraits>
    class indirect_compare;
}

//--------------------------------------------------------------------------------------//
//                                class index_set_base                                  //
//--------------------------------------------------------------------------------------//

template <class Key, class BtreeTraits, class Compare, class IndexTraits>
class index_set_base
{
public:
  typedef Key                                    key_type;
  typedef Key                                    value_type;
  typedef Key                                    mapped_type;
  typedef BtreeTraits                            btree_traits;
  typedef Compare                                compare_type;
  typedef compare_type                           value_compare;
  typedef typename BtreeTraits::node_id_type     node_id_type;
  typedef typename BtreeTraits::node_size_type   node_size_type;
  typedef typename BtreeTraits::node_level_type  node_level_type;
  typedef typename IndexTraits::reference        reference;                

  typedef IndexTraits                            index_traits;
  typedef typename index_traits::index_key       index_key;  // i.e. position in flat file
  typedef detail::indirect_compare<key_type,
    index_key, compare_type, index_traits>       index_compare_type;

  //  the following is the only difference between index_set_base and index_multiset_base
  typedef typename
    btree::btree_set<index_key, btree_traits,
      index_compare_type>                        index_type;
};

//--------------------------------------------------------------------------------------//
//                             class index_multiset_base                                //
//--------------------------------------------------------------------------------------//

template <class Key, class BtreeTraits, class Compare, class IndexTraits>
class index_multiset_base
{
public:
  typedef Key                                    key_type;
  typedef Key                                    value_type;
  typedef Key                                    mapped_type;
  typedef BtreeTraits                            btree_traits;
  typedef Compare                                compare_type;
  typedef compare_type                           value_compare;
  typedef typename BtreeTraits::node_id_type     node_id_type;
  typedef typename BtreeTraits::node_size_type   node_size_type;
  typedef typename BtreeTraits::node_level_type  node_level_type;
  typedef typename IndexTraits::reference        reference;                

  typedef IndexTraits                            index_traits;
  typedef typename index_traits::index_key       index_key;  // i.e. position in flat file
  typedef detail::indirect_compare<key_type,
    index_key, compare_type, index_traits>       index_compare_type;

  //  the following is the only difference between index_set_base and index_multiset_base
  typedef typename
    btree::btree_multiset<index_key, btree_traits,
      index_compare_type>                        index_type;
};

////--------------------------------------------------------------------------------------//
////                               class index_map_base                                   //
////--------------------------------------------------------------------------------------//
//
//template <class Key, class T, class BtreeTraits>
//class index_map_base
//{
//public:
//  typedef typename BtreeTraits::node_id_type     node_id_type;
//  typedef typename BtreeTraits::node_size_type   node_size_type;
//  typedef typename BtreeTraits::node_level_type  node_level_type;
//  typedef std::pair<const Key, T>           value_type;
//  typedef T                                 mapped_type;
//  typedef BtreeTraits                            btree_traits;
//  typedef typename BtreeTraits::compare_type     compare_type;
//
//  const Key&  key(const value_type& v) const  // really handy, so expose
//    {return v.first;}
//  const T&    mapped(const value_type& v) const
//    {return v.second;}
//
//  class value_compare
//  {
//  public:
//    value_compare() {}
//    value_compare(compare_type comp) : m_comp(comp) {}
//    bool operator()(const value_type& x, const value_type& y) const
//      { return m_comp(x.first, y.first); }
//    template <class K>
//    bool operator()(const value_type& x, const K& y) const
//      { return m_comp(x.first, y); }
//    template <class K>
//    bool operator()(const K& x, const value_type& y) const
//      { return m_comp(x, y.first); }
//  private:
//    compare_type    m_comp;
//  };
//
//};

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                                 class index_base                                     //
//                                                                                      //
//--------------------------------------------------------------------------------------//


template <class Base>  // index_map_base or index_set_base
class index_base : public Base, private noncopyable
{
public:
  template <class T, class Reference>
    class iterator_type;

//--------------------------------------------------------------------------------------//
//                                public interface                                      //
//--------------------------------------------------------------------------------------//

public:
  // types:
  typedef typename Base::key_type               key_type;
  typedef typename Base::value_type             value_type;
  typedef typename Base::mapped_type            mapped_type;
  typedef typename Base::compare_type           compare_type;
  typedef typename Base::compare_type           key_compare;
  typedef typename Base::value_compare          value_compare; 
  typedef boost::uint64_t                       size_type;

  typedef typename Base::reference              reference;
  typedef iterator_type<value_type, reference>  iterator;
  typedef iterator                              const_iterator;

  typedef std::reverse_iterator<iterator>       reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
  typedef std::pair<const_iterator, const_iterator>
                                                const_iterator_range;

  typedef typename Base::btree_traits           btree_traits;
  typedef typename Base::node_id_type           node_id_type;
  typedef typename Base::node_size_type         node_size_type;
  typedef typename Base::node_level_type        node_level_type;

  typedef boost::filesystem::path               path_type;

  typedef boost::btree::extendible_mapped_file  file_type;
  typedef boost::shared_ptr<file_type>          file_ptr_type;
  typedef file_type::size_type                  file_size_type;
  typedef file_type::position_type              file_position;

  typedef typename Base::index_traits           index_traits;
  typedef typename index_traits::index_key      index_key;  // i.e. position in flat file

  typedef typename Base::index_compare_type     index_compare_type;

  typedef typename Base::index_type             index_type;

  typedef typename index_type::size_type        index_size_type;


protected:
  index_type      m_index_btree;
  file_ptr_type   m_file;          // shared_ptr to flat file; shared with other indexes
  compare_type    m_comp;

public:
  index_base() {} 

  //  opens

  void open(const path_type& index_pth,
            const path_type& file_pth,
            flags::bitmask flgs = flags::read_only,
            uint64_t sig = -1,  // for existing files, must match creation signature
            const compare_type& comp = compare_type(),
            std::size_t node_sz = default_node_size)  // node_sz ignored if existing file
  {
    BOOST_ASSERT(!m_index_btree.is_open());
    BOOST_ASSERT(!m_file.get());
    m_file.reset(new file_type);
    m_file->open(file_pth, flgs, reserve_default(flgs));
    open(index_pth, m_file, flgs, sig, comp, node_sz);
  }

  void open(const path_type& index_pth,
            file_ptr_type flat_file,            
            flags::bitmask flgs = flags::read_only,
            uint64_t sig = -1,  // for existing files, must match creation signature
            const compare_type& comp = compare_type(),
            std::size_t node_sz = default_node_size)  // node_sz ignored if existing file
  {
    BOOST_ASSERT(!m_index_btree.is_open());
    BOOST_ASSERT(flat_file->is_open());
    m_file = flat_file;
    m_comp = comp;
    m_index_btree.open(index_pth, flgs, sig,
      index_compare_type(m_comp, m_file.get()), node_sz);
  }

  //  modifiers
  iterator erase(const_iterator itr)
  {
    typename index_type::const_iterator result(m_index_btree.erase(itr.m_index_iterator));
    return result == m_index_btree.cend()
      ? end()
      : iterator(result, m_file);
  }
  size_type erase(const key_type& k)
  {
    size_type count = 0;
    const_iterator itr = lower_bound(k);
    
    while (itr != end() && !m_index_btree.key_comp()(k, *itr.m_index_iterator))
    {
      ++count;
      itr = erase(itr);
    }
    return count;
  }
  iterator erase(const_iterator first, const_iterator last)
  {
    return iterator(m_index_btree.erase(first.m_index_iterator, last.m_index_iterator),
      m_file);
  }

  //  iterators

  iterator begin()              {return iterator(m_index_btree.begin(), file());}
  const_iterator begin() const  {return const_iterator(m_index_btree.begin(), file());}
  iterator end()                {return iterator(m_index_btree.end(), file());}
  const_iterator end() const    {return const_iterator(m_index_btree.end(), file());}

  //  observers
  bool              is_open() const       {BOOST_ASSERT(!m_index_btree.is_open()
                                             || m_file->is_open());
                                           return m_index_btree.is_open();}
  flags::bitmask    flags() const         {return m_index_btree.flags();}
  bool              index_empty() const   {return m_index_btree.empty();}
  path_type         index_path() const    {return m_index_btree.path();}
  index_size_type   index_size() const    {return m_index_btree.size();}

  file_ptr_type     file() const          {return m_file;}
  path_type         file_path() const     {BOOST_ASSERT(m_file);
                                           return m_file->path();}
  file_size_type    file_size() const     {BOOST_ASSERT(m_file);
                                           return m_file->file_size();}
  file_size_type    file_reserve() const  {BOOST_ASSERT(m_file);
                                           return m_file->reserve();}

  // operations

  file_position     position(iterator itr) const;
  //  Returns: The offset in the flat file of the element pointed to by itr

  const_iterator    find(const key_type& k) const 
                                           {return const_iterator(m_index_btree.find(k),
                                              m_file);}
 
  size_type         count(const key_type& k) const  {return m_index_btree.count(k);}

  const_iterator    lower_bound(const key_type& k) const
                                    {return const_iterator(m_index_btree.lower_bound(k),
                                      m_file);}

  const_iterator    upper_bound(const key_type& k) const
                                    {return const_iterator(m_index_btree.upper_bound(k),
                                      m_file);}
  const_iterator_range
                    equal_range(const key_type& k) const
                                  {return std::make_pair(lower_bound(k), upper_bound(k));}

  //------------------------------------------------------------------------------------//
  //                                  iterator_type                                     //
  //------------------------------------------------------------------------------------//
 
  template <class T, class Reference>
  class iterator_type
    : public boost::iterator_facade<iterator_type<T, Reference>, T,
        bidirectional_traversal_tag, Reference>
  {
  public:
    typedef typename index_type::iterator   index_iterator_type;
    typedef typename index_base::file_type  file_type;

    iterator_type() {}  // constructs the end iterator

    iterator_type(index_iterator_type itr, const file_ptr_type& file)
      : m_index_iterator(itr), m_file(file.get()) {}

  private:
    friend class boost::iterator_core_access;
    friend class index_base;

    index_iterator_type   m_index_iterator;
    file_type*            m_file;  // 0 for end iterator

    Reference dereference() const
    { 
      BOOST_ASSERT_MSG(m_file, "btree index attempt to dereference end iterator");
//      std::cout << "**** " << *m_index_iterator << std::endl;
      return index_traits::make_reference(m_file->const_data<char>() + *m_index_iterator);
    }
 
    bool equal(const iterator_type& rhs) const
    {
      BOOST_ASSERT(m_file == rhs.m_file);
      return m_index_iterator == rhs.m_index_iterator;
    }

    void increment() { ++m_index_iterator; }
    void decrement() { --m_index_iterator; }
  };

};  // class index_base

namespace detail
{
  // TODO: Pos needs to be a distinct type so no ambiguity arises if Key and
  // file_position happen to be the same type. Or else use the heterogenous type
  // approach now used in the std library

  template <class Key, class Pos, class Compare, class IndexTraits>
  class indirect_compare
  {
    Compare                        m_comp;
    extendible_mapped_file*        m_file;

  public:

    indirect_compare(){}
    indirect_compare(Compare comp, extendible_mapped_file* flat_file)
      : m_comp(comp), m_file(flat_file) {}

    bool operator()(const Key& lhs, Pos rhs) const
    {
      return m_comp(lhs,
        IndexTraits::make_reference(m_file->const_data<char>()+rhs));
    }
    bool operator()(Pos lhs, const Key& rhs) const
    {
      return m_comp(IndexTraits::make_reference(m_file->const_data<char>()+lhs), rhs);
    }
    bool operator()(Pos lhs, Pos rhs) const
    {
      return m_comp(IndexTraits::make_reference(m_file->const_data<char>()+lhs),
        IndexTraits::make_reference(m_file->const_data<char>()+rhs));
    }

  };

}  // namespace detail
}  // namespace btree
}  // namespace boost

#endif // BOOST_BTREE_INDEX_BASES_HPP

