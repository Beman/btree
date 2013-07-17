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
    

*/

#ifndef BOOST_BTREE_INDEX_HPP
#define BOOST_BTREE_INDEX_HPP

#include <boost/btree/helpers.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/btree/mmff.hpp>
#include <boost/btree/set.hpp>
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
//                                   btree_index                                        //
//--------------------------------------------------------------------------------------//

template <class Key, class Traits = default_traits,
          class Comp = btree::less<Key> >
class btree_index
{
public:
  typedef typename Traits::file_position_type    file_position_type;  // in index
  typedef boost::filesystem::path                path_type;

  typedef detail::indirect_compare<Key,
    file_position_type, Comp>                    index_compare_type;
  typedef typename
    btree_set<file_position_type, Traits,
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
    m_comp = comp;
    m_set.open(index_pth, flgs, sig, node_sz,
      index_compare_type(m_comp, m_file.get()));
  }

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


private:
  index_type      m_set;
  file_ptr_type   m_file;   // shared by all indices to this flat file
  Comp            m_comp;
};
  

namespace detail
{
  // TODO: Pos needs to be a private type so no ambiguity arises if Key and
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
        *reinterpret_cast<const Key*>(m_file->const_data()+rhs));
    }
    bool operator()(Pos lhs, const Key& rhs) const
    {
      return m_comp(*reinterpret_cast<const Key*>(m_file->const_data()+lhs, rhs));
    }
    bool operator()(Pos lhs, Pos rhs) const
    {
      return m_comp(*reinterpret_cast<const Key*>(m_file->const_data()+lhs),
        *reinterpret_cast<const Key*>(m_file->const_data()+rhs));
    }

  };

}  // namespace detail

}  // namespace btree
}  // namespace boost

#endif  BOOST_BTREE_INDEX_HPP
