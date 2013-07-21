//  boost/btree/index.hpp  -------------------------------------------------------------//

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                       index for a memory-mapped flat-file                            //
//                                                                                      //
//--------------------------------------------------------------------------------------//

/*

  Proof-of-concept; not intended for actual use.

  TODO list:

  * For fixed-length data, should be possible to traffic in ids rather than positions.

  * Since file() exposes the flat file, do we need file_size(), file_path(), etc?
    Yes, since file_type should be documented as an implementation supplied type.
  
  * Need multi- version

  * Knows-own-size types like a C style (const char*) null terminated string should work.
    Add test case, support header.

*/

#ifndef BOOST_BTREE_INDEX_HPP
#define BOOST_BTREE_INDEX_HPP

#include <boost/btree/helpers.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/btree/mmff.hpp>
#include <boost/btree/set.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/assert.hpp>

namespace boost
{
namespace btree
{
  namespace detail
  {
    template <class T, class Pos, class Comp>
      class indirect_compare;
  }

//--------------------------------------------------------------------------------------//
//                                types and traits                                      //
//--------------------------------------------------------------------------------------//

  typedef boost::btree::extendible_mapped_file::position_type position_type;

  template <class Traits>
  struct big_index_traits
    : public Traits
  {
    typedef endian::big_uint48un_t  index_position_type;
    typedef endian::big_uint16un_t  file_element_size_type; 
  };

  typedef big_index_traits<btree::default_traits> default_index_traits;

//--------------------------------------------------------------------------------------//
//                                   btree_index                                        //
//--------------------------------------------------------------------------------------//

template <class T,          // shall be trivially copyable type; see std 3.9 [basic.types]
          class Traits = default_index_traits,
          class Comp = btree::less<T> >
class btree_index
{
  template <class VT>
  class iterator_type;
public:
  typedef T                                      value_type;
  typedef T                                      key_type;
  typedef T                                      mapped_type;

  typedef iterator_type<const value_type>        iterator;
  typedef iterator                               const_iterator;
  typedef std::pair<const_iterator,
    const_iterator>                              const_iterator_range;

  typedef typename Traits::index_position_type   index_position_type;
  typedef boost::filesystem::path                path_type;

  typedef detail::indirect_compare<T,
    index_position_type, Comp>                   index_compare_type;
  typedef typename
    btree_set<index_position_type, Traits,
      index_compare_type>                        index_type;
  typedef typename index_type::size_type         index_size_type;

  typedef boost::btree::extendible_mapped_file   file_type;
  typedef boost::shared_ptr<file_type>           file_ptr_type;
  typedef file_type::size_type                   file_size_type;

  btree_index() {}

  btree_index(const path_type& file_pth,
            file_size_type reserv,
            const path_type& index_pth,
            flags::bitmask flgs = flags::read_only,
            uint64_t sig = -1,  // for existing files, must match creation signature
            std::size_t node_sz = default_node_size,  // ignored if existing file
            const Comp& comp = Comp())
  {
    open(file_pth, reserv, index_pth, flgs, sig, node_sz, comp);
  }

  btree_index(file_ptr_type flat_file,            
            const path_type& index_pth,
            flags::bitmask flgs = flags::read_only,
            uint64_t sig = -1,  // for existing files, must match creation signature
            std::size_t node_sz = default_node_size,  // ignored if existing file
            const Comp& comp = Comp())
  {
    open(flat_file, index_pth, flgs, sig, node_sz, comp);
  }

  void open(const path_type& file_pth,
            file_size_type reserv,
            const path_type& index_pth,
            flags::bitmask flgs = flags::read_only,
            uint64_t sig = -1,  // for existing files, must match creation signature
            std::size_t node_sz = default_node_size,  // ignored if existing file
            const Comp& comp = Comp())
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
            const Comp& comp = Comp())
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
  bool              index_empty() const   {return m_set.empty();}
  path_type         index_path() const    {return m_set.path();}
  index_size_type   index_size() const    {return m_set.size();}

  file_ptr_type     file() const          {return m_file;}
  path_type         file_path() const     {BOOST_ASSERT(m_file);
                                           return m_file->path();}
  file_size_type    file_size() const     {BOOST_ASSERT(m_file);
                                           return m_file->file_size();}
  file_size_type    file_reserve() const  {BOOST_ASSERT(m_file);
                                           return m_file->reserve();}
  //  modifiers

  btree::position_type push_back(const value_type& value)
  // Effects: unconditional push_back into file(); index unaffected
  {
    return file()->push_back(value);
  }

  std::pair<const_iterator, bool>
    insert_position(btree::position_type pos)
  {
    std::pair<index_type::const_iterator, bool>
      result(m_set.insert(index_position_type(pos)));
    return std::pair<const_iterator, bool>(
      const_iterator(result.first, file()), result.second);
  }

  std::pair<const_iterator, bool>
    insert(const value_type& value)
  //  Effects: if !find(k) then insert_position(push_back(value));
  {
    if (find(value) == end())
    {
      std::pair<index_type::const_iterator, bool> result;
      result = insert_position(push_back(value));
      BOOST_ASSERT(result.second);
      return std::pair<const_iterator, bool>(
        const_iterator(result.first, file()), true);
    }
    return std::pair<const_iterator, bool>(const_iterator(), false);
  }

  // operations

  btree::position_type  position(iterator itr) const;
  const_iterator       find(const key_type& k) const;

  const_iterator        find(const key_type& k) const;
  size_type             count(const key_type& k) const;

  const_iterator        lower_bound(const key_type& k) const;
  const_iterator        upper_bound(const key_type& k) const;

  const_iterator_range  equal_range(const key_type& k) const
                          { return std::make_pair(lower_bound(k), upper_bound(k)); }

private:
  index_type      m_set;
  file_ptr_type   m_file;   // shared_ptr to flat file shared with other indexes
  Comp            m_comp;

  //------------------------------------------------------------------------------------//
  //                                  iterator_type                                     //
  //------------------------------------------------------------------------------------//
 
  template <class T>
  class iterator_type
    : public boost::iterator_facade<iterator_type<T>, T, bidirectional_traversal_tag>
  {
  public:
    typedef typename index_type::iterator    index_iterator_type;
    typedef typename btree_index::file_type  file_type;

    iterator_type() {}  // constructs the end iterator

    iterator_type(index_iterator_type itr, const file_ptr_type& file)
      : m_index_iterator(itr), m_file(file.get()) {}

  private:
    friend class boost::iterator_core_access;

    index_iterator_type   m_index_iterator;
    file_type*            m_file;

    T& dereference() const
    { 
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

};
  

namespace detail
{
  // TODO: Pos needs to be a distinct type so no ambiguity arises if Key and
  // file_position_type happen to be the same type

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
      return m_comp(*reinterpret_cast<const Key*>(m_file->const_data<char>()+lhs, rhs));
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

#endif  BOOST_BTREE_INDEX_HPP
