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

  *  template <class K> operations fuctions per C++14.

  *  verify dereferencing the end iterator assert fires correctly.

  *  The flat_adapter<string_view> specialization is space efficient, but does not support
     embedded nulls and is time inefficient for very long strings. Users might want
     trade-offs, so alternate flavors should be available. This might serve as a poster
     child for use cases where the type alone isn't sufficient to determine which
     flat_adapter to use. Consider adding a template parameter (two for maps)
     at the end that defaults to (renamed) default_flat_adapter<Key>.

  *  Rename in index_helpers.hpp to index_traits.hpp?

*/

namespace boost
{
namespace btree
{

//--------------------------------------------------------------------------------------//
//                                class index_set_base                                  //
//--------------------------------------------------------------------------------------//

template <class Key, class Traits, class Comp, class NdxTraits>
class index_set_base
{
public:
  typedef Key                               key_type;
  typedef key_type                          value_type;
  typedef key_type                          mapped_type;
  typedef Traits                            btree_traits;
  typedef Comp                              compare_type;
  typedef compare_type                      value_compare;
  typedef NdxTraits                         index_traits;
  typedef typename Traits::node_id_type     node_id_type;
  typedef typename Traits::node_size_type   node_size_type;
  typedef typename Traits::node_level_type  node_level_type;
};

////--------------------------------------------------------------------------------------//
////                               class index_map_base                                   //
////--------------------------------------------------------------------------------------//
//
//template <class Key, class T, class Traits>
//class index_map_base
//{
//public:
//  typedef typename Traits::node_id_type     node_id_type;
//  typedef typename Traits::node_size_type   node_size_type;
//  typedef typename Traits::node_level_type  node_level_type;
//  typedef std::pair<const Key, T>           value_type;
//  typedef T                                 mapped_type;
//  typedef Traits                            btree_traits;
//  typedef typename Traits::compare_type     compare_type;
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

  namespace detail
  {
    template <class T, class Pos, class Comp>
      class indirect_compare;
  }

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                                 class index_base                                     //
//                                                                                      //
//--------------------------------------------------------------------------------------//


template <class Base>  // index_map_base or index_set_base
class index_base : public Base, private noncopyable
{
private:
  template <class T>
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
  typedef value_type&                           reference;
  typedef const value_type&                     const_reference;
  typedef boost::uint64_t                       size_type;
  typedef value_type*                           pointer;
  typedef const value_type*                     const_pointer;

  typedef iterator_type<const value_type>       iterator;
  typedef iterator                              const_iterator;

  typedef std::reverse_iterator<iterator>       reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
  typedef std::pair<const_iterator, const_iterator>
                                                const_iterator_range;

  typedef typename Base::btree_traits           btree_traits;
  typedef typename Base::node_id_type           node_id_type;
  typedef typename Base::node_size_type         node_size_type;
  typedef typename Base::node_level_type        node_level_type;

  typedef typename Base::index_traits           index_traits;


  typedef boost::filesystem::path               path_type;

  typedef boost::btree::extendible_mapped_file  file_type;
  typedef boost::shared_ptr<file_type>          file_ptr_type;
  typedef file_type::size_type                  file_size_type;
  typedef file_type::position_type              file_position;

  typedef typename index_traits::index_key      index_key;

  typedef detail::indirect_compare<key_type,
    index_key, compare_type>                    index_compare_type;
  typedef typename
    btree_set<index_key, btree_traits,
      index_compare_type>                       index_type;
  typedef typename index_type::size_type        index_size_type;


protected:
  index_type      m_set;
  file_ptr_type   m_file;   // shared_ptr to flat file shared with other indexes
  compare_type    m_comp;

public:
  index_base() {} 

  //  opens

  void open(const path_type& file_pth,
            file_size_type reserv,
            const path_type& index_pth,
            flags::bitmask flgs = flags::read_only,
            uint64_t sig = -1,  // for existing files, must match creation signature
            std::size_t node_sz = default_node_size,  // ignored if existing file
            const compare_type& comp = compare_type())
  {
    BOOST_ASSERT(!m_set.is_open());
    BOOST_ASSERT(!m_file.get());
    m_file.reset(new file_type);
    m_file->open(file_pth, flgs, reserv);
    open(m_file, index_pth, flgs, sig, node_sz, comp);
  }

  void open(file_ptr_type flat_file,            
            const path_type& index_pth,
            flags::bitmask flgs = flags::read_only,
            uint64_t sig = -1,  // for existing files, must match creation signature
            std::size_t node_sz = default_node_size,  // ignored if existing file
            const compare_type& comp = compare_type())
  {
    BOOST_ASSERT(!m_set.is_open());
    BOOST_ASSERT(flat_file->is_open());
    m_file = flat_file;
    m_comp = comp;
    m_set.open(index_pth, flgs, sig, node_sz,
      index_compare_type(m_comp, m_file.get()));
  }

  //  iterators

  iterator begin()              {return iterator(m_set.begin(), file());}
  const_iterator begin() const  {return const_iterator(m_set.begin(), file());}
  iterator end()                {return iterator(m_set.end(), file());}
  const_iterator end() const    {return const_iterator(m_set.end(), file());}

  //  observers
  bool              is_open() const       {BOOST_ASSERT(!m_set.is_open()
                                             || m_file->is_open());
                                           return m_set.is_open();}
  bool              read_only() const     {return m_set.read_only();}
  bool              index_empty() const   {return m_set.empty();}
  path_type         index_path() const    {return m_set.path();}
  index_size_type   index_size() const    {return m_set.size();}

  file_ptr_type     file() const          {return m_file;}
  path_type         file_path() const     {BOOST_ASSERT(m_file);
                                           return m_file->path();}
  file_size_type    file_size() const     {BOOST_ASSERT(m_file);
                                           return m_file->file_size();}
  file_size_type    file_reserve() const  {BOOST_ASSERT(m_file);}

  // operations

  file_position     position(iterator itr) const;
  //  Returns: The offset in the flat file of the element pointed to by itr

  const_iterator    find(const key_type& k) const 
                                           {return const_iterator(m_set.find(k), m_file);}
 
  size_type         count(const key_type& k) const  {return m_set.count(k);}

  const_iterator    lower_bound(const key_type& k) const
                                    {return const_iterator(m_set.lower_bound(k), m_file);}

  const_iterator    upper_bound(const key_type& k) const
                                    {return const_iterator(m_set.upper_bound(k), m_file);}
  const_iterator_range
                    equal_range(const key_type& k) const
                                  {return std::make_pair(lower_bound(k), upper_bound(k));}

  //------------------------------------------------------------------------------------//
  //                                  iterator_type                                     //
  //------------------------------------------------------------------------------------//
 
private:
  template <class T>
  class iterator_type
    : public boost::iterator_facade<iterator_type<T>, T, bidirectional_traversal_tag>
  {
  public:
    typedef typename index_type::iterator    index_iterator_type;
    typedef typename index_base::file_type  file_type;

    iterator_type() {}  // constructs the end iterator

    iterator_type(index_iterator_type itr, const file_ptr_type& file)
      : m_index_iterator(itr), m_file(file.get()) {}

  private:
    friend class boost::iterator_core_access;

    index_iterator_type   m_index_iterator;
    file_type*            m_file;  // 0 for end iterator

    T& dereference() const
    { 
      BOOST_ASSERT_MSG(m_file, "btree index attempt to dereference end iterator");
      return *(reinterpret_cast<T*>(m_file->const_data<char>() + *m_index_iterator));
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
  // file_position happen to be the same type

  template <class Key, class Pos, class Comp>
  class indirect_compare
  {
    Comp                           m_comp;
    extendible_mapped_file*        m_file;

  public:

    indirect_compare(){}
    indirect_compare(Comp comp, extendible_mapped_file* flat_file)
      : m_comp(comp), m_file(flat_file) {}

    bool operator()(const Key& lhs, Pos rhs) const
    {
      return m_comp(lhs,
        *reinterpret_cast<const Key*>(m_file->const_data<char>()+rhs));
    }
    bool operator()(Pos lhs, const Key& rhs) const
    {
      return m_comp(*reinterpret_cast<const Key*>(m_file->const_data<char>()+lhs), rhs);
    }
    bool operator()(Pos lhs, Pos rhs) const
    {
      return m_comp(*reinterpret_cast<const Key*>(m_file->const_data<char>()+lhs),
        *reinterpret_cast<const Key*>(m_file->const_data<char>()+rhs));
    }

  };

}  // namespace detail
}  // namespace btree
}  // namespace boost

#endif // BOOST_BTREE_INDEX_BASES_HPP
