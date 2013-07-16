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

*/

#ifndef BOOST_BTREE_INDEX_HPP
#define BOOST_BTREE_INDEX_HPP

#include <boost/btree/helpers.hpp>
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
  template <class T>
    class iterator_type;

public:
  typedef typename Traits::file_position_type  file_position_type;

private:
  btree_set<file_position_type, Traits, Comp>  m_set;
  extendible_mapped_file*                      m_file;
  Comp                                         m_comp;
public:
  typedef typename btree_set<file_position_type, Traits, Comp>::size_type size_type;

  void open(extendible_mapped_file& flat_file,            
            const boost::filesystem::path& index_path,
            flags::bitmask flgs = flags::read_only,
            uint64_t signature = -1,  // for existing files, must match creation signature
            std::size_t node_sz = default_node_size,  // ignored if existing file
            const Comp& comp = Comp())
  {
    BOOST_ASSERT(flat_file.is_open());
    m_comp = comp;
    m_set.open(index_path, flgs, signature, node_sz,
      detail::indirect_compare<Key, file_position_type, Comp>(m_comp, m_file));
  }

  bool       is_open() const            {return m_set.is_open();}
  bool       empty() const              {return m_set.empty();}
  size_type  size() const               {return m_set.size();}

};
  

namespace detail
{
  // TODO: Pos needs to be a private type so no ambiguity arises if Key and
  // file_position_type happen to be the same type

  template <class Key, class Pos, class Comp>
  class indirect_compare
  {
    Comp                           m_comp;
    const extendible_mapped_file&  m_file;

  public:
    indirect_compare(Comp comp, const extendible_mapped_file& flat_file)
      : m_comp(comp), m_file(flat_file) {}

    bool operator()(const Key& lhs, Pos rhs) const
    {
      return m_comp(lhs,
        *reinterpret_cast<const Key*>(m_file.const_data()+rhs));
    }
    bool operator()(Pos lhs, const Key& rhs) const
    {
      return m_comp(*reinterpret_cast<const Key*>(m_file.const_data()+lhs, rhs));
    }
    bool operator()(Pos lhs, Pos rhs) const
    {
      return m_comp(*reinterpret_cast<const Key*>(m_file.const_data()+lhs),
        *reinterpret_cast<const Key*>(m_file.const_data()+rhs));
    }

  };

}  // namespace detail

}  // namespace btree
}  // namespace boost

#endif  BOOST_BTREE_INDEX_HPP
