//  boost/btree/detail/btree_bases.hpp  ------------------------------------------------//

//  Copyright Beman Dawes 2000, 2006, 2010, 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  See http://www.boost.org/libs/btree for documentation.

#ifndef BOOST_BTREE_BASES_HPP
#define BOOST_BTREE_BASES_HPP

#include <boost/iterator/iterator_facade.hpp>
#include <boost/noncopyable.hpp>
#include <boost/btree/detail/buffer_manager.hpp>
#include <boost/assert.hpp>
#include <boost/static_assert.hpp>
#include <cstddef>     // for size_t
#include <cstring>
#include <cassert>
#include <utility>
#include <iterator>
#include <functional>  // for less, binary_function
#include <algorithm>
#include <ostream>
#include <stdexcept>

/*

  TODO:

  * flags for key_varies and mapped_varies added, but not being set or used yet.
    key and mapped size no longer set to -1 to indicate variable length. 
  
  * btree_unit_test.cpp: need test of erase(itr1, itr2).
  
  * btree_unit_test.cpp: review tests that are commented out.

  * Should header() be part of the public interface?
      - Add individual get, and where appropriate, set, functions.
      - Move header file to detail.

  * For multi-containers, add a test case of a deep tree with all the same key. Then
    test erasing various elements.

  * Either add code to align mapped() or add a requirement that PID, K does not
    require alignment.

  * Add a function (apply(key, mapped_value)?) that inserts if key not present,
    updates if present.

  * The commented out logging in binary_file.cpp was very useful. (1) move it to header
    and (2) apply only when BOOST_BINARY_FILE_LOG is defined. This implies adding m_ to
    the actual binary_file.cpp implementation names.

  * Preload option currently is just passed on to binary_file, which reads the entire file.
    That preloads the O/S cache, but does nothing for the btree cache. Should also iterate
    over the btree to preload the btree cache.

  * Should (some) constructors, open, have max_cache_size argument?

  * Verify, document, that a max_cache_size(-1) is "all".

  * Problem: if key_type or mapped type require 64-bit alignment on some machines, but
    not on others (for example, 32-bit gcc builds), would need to artificially force
    64-bit alignment to ensure data portability with endian traits. Is an alignment trait
    needed?

  * Search all files for TODO, clear problems if possible, otherwise add to this list.

  * buffer_manager is using an intrusive set (buffers_type) for page_id lookup. An
    intrusive unordered_set might be more appropriate.

  * bt_time does not actually implement -html option except for -stl

  * bt_time should use high_res clock, perhaps in addition to cpu times?

  * When a command line argument is supposed to have a numeric value appended, check
    is_digit(). If no value appended, use strcmp, NOT strncmp. See bt_time for examples.

  * It isn't at all clear that btree_base should expose manager() and header().
    Instead provide observer functions that call the equivalent manager() and header()
    observer functions.

  * Non-member btree_* functions not implemented yet. See line 2190 or thereabouts.

  * An insert_packed() function could avoid searching and cache thrashing by hanging onto
    an iterator between calls. Check pack optimization applies (last page, etc) and verify
    new element > previous element (>= if non-unique), then just tack on the end; see
    m_branch_insert. Consider whether or not this could be combined with an insert-with-
    hint function or insert-after function.

  * Bulk load: If file_size(source) <= max_memory: just load, sort, and insert:-)

  * If an erase causes the tree size to become 0, pack optimization should be reenabled.
    (That may require redoing sequence of inserts in btree_unit_test section that says
    "add enough elements to force branch node splits"

  * mapped() is provided for set. Is that desirable?

  * N3657 identifies cases where support for heterogeneous comparisons can cause problems
    with overagressive templates. Figure out some tests to see if this is a problem with
    btree::, value_compare, or branch_compare, and add enable_if's per N3657 if needed.

  * Should size_type erase(const key_type& k); be template <class K> ?

  * operator unspecified-bool-type, operator!


*/

namespace boost
{
namespace btree
{

//--------------------------------------------------------------------------------------//
//                             general support functions                                //
//--------------------------------------------------------------------------------------//

//  append
//
//  important optimization side-effect: resulting tree will be packed; at each level, all
//  nodes except the last will have as many elements as will fit on the node

template <class Btree>
void append(const Btree& from, const Btree& to)
{
  BOOST_ASSERT_MSG(from.is_open(), "append() requires 'from' btree be open");
  BOOST_ASSERT_MSG(to.is_open(), "append() requires 'to' btree be open");
  for (typename Btree::iterator it = from.begin(); it != from.end(); ++it)
    to.insert(*it);
}


//--------------------------------------------------------------------------------------//
//                                class btree_set_base                                  //
//--------------------------------------------------------------------------------------//

template <class Key, class Traits, class Compare>
class btree_set_base
{
public:
  typedef typename Traits::node_id_type     node_id_type;
  typedef typename Traits::node_size_type   node_size_type;
  typedef typename Traits::node_level_type  node_level_type;
  typedef Key                               value_type;
  typedef Key const                         iterator_value_type;
  typedef Key                               mapped_type;
  typedef Traits                            traits_type;
  typedef Compare                           compare_type;
  typedef compare_type                      value_compare;

  const Key&          key(const value_type& v) const   // really handy, so expose
    {return v;}
  const mapped_type&  mapped(const value_type& v) const
    {return v;}
};

//--------------------------------------------------------------------------------------//
//                               class btree_map_base                                   //
//--------------------------------------------------------------------------------------//

template <class Key, class T, class Traits, class Compare>
class btree_map_base
{
public:
  typedef typename Traits::node_id_type     node_id_type;
  typedef typename Traits::node_size_type   node_size_type;
  typedef typename Traits::node_level_type  node_level_type;
  typedef std::pair<const Key, T>           value_type;
  typedef std::pair<const Key, T>           iterator_value_type;
  typedef T                                 mapped_type;
  typedef Traits                            traits_type;
  typedef Compare                           compare_type;

  const Key&  key(const value_type& v) const  // really handy, so expose
    {return v.first;}
  const T&    mapped(const value_type& v) const
    {return v.second;}

  class value_compare
  {
  public:
    value_compare() {}
    value_compare(compare_type comp) : m_comp(comp) {}
    bool operator()(const value_type& x, const value_type& y) const
      { return m_comp(x.first, y.first); }
    template <class K>
    bool operator()(const value_type& x, const K& y) const
      { return m_comp(x.first, y); }
    template <class K>
    bool operator()(const K& x, const value_type& y) const
      { return m_comp(x, y.first); }
  private:
    compare_type    m_comp;
  };

};

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                                 class btree_base                                     //
//                                                                                      //
//--------------------------------------------------------------------------------------//

/*

  This class implements a B+tree.
  
  There is no leaf node linked-list; a tree-walk provides equivalent functionality.

  Valid-chain-to-root invariant:

    Iterators contain a (smart) btree_node_ptr to the leaf node that contains the
    element being pointed to, and a (dumb) pointer to the element itself.
    Leaf and branch nodes contain a btree_node_ptr to the parent node in the tree, and
    a (dumb) branch_value_type* to the parent element itself.

    This chain, from iterator to root, is valid as long as the iterator is valid. Any
    operations that create an iterator must establish this chain, and any operations
    that advance the iterator forward or backward must maintain the validity of the
    chain.

  To facilitate emplace semantics, internal leaf inserts insert the key only, and if
  sizeof(key_type) is less than sizeof(value_type) (i.e. it is a map rather than a set)
  leave a space of the mapped_type. The map calling functions are responsible for
  actually emplacing/inserting the mapped_type contents.

  erase() invalidate iterators pointing to element beyond the erased element, but does
  not invalidate iterators pointing to elements prior to the erased element. This is a
  property of B-trees in general, not something specific to this implementation.

*/

template  <class Key,   // requires memcpyable type without pointers or references
           class Base>  // btree_map_base or btree_set_base

class btree_base : public Base, private noncopyable
{
private:
  class btree_node;
  class btree_node_ptr;
  class btree_data;
  class leaf_data;
  class branch_data;
  class leaf_node;
  class branch_node;
  template <class T>
    class iterator_type;

//--------------------------------------------------------------------------------------//
//                                public interface                                      //
//--------------------------------------------------------------------------------------//

public:
  // types:
  typedef Key                                   key_type;
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

  // for sets, these are the same type; for maps they are different types
  typedef iterator_type<
    typename Base::iterator_value_type>         iterator;
  typedef iterator_type<value_type const>       const_iterator;

  typedef std::reverse_iterator<iterator>       reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  typedef typename Base::traits_type            traits_type;
  typedef typename Base::node_id_type           node_id_type;
  typedef typename Base::node_size_type         node_size_type;
  typedef typename Base::node_level_type        node_level_type;

  // construct/destroy:

  // TODO: why aren't these protected?
  btree_base();
  btree_base(const boost::filesystem::path& p, flags::bitmask flgs, uint64_t signature,
    const compare_type& comp, std::size_t node_sz);
  ~btree_base();

  //  file operations:

  void flush()                              {
                                              BOOST_ASSERT(is_open());
                                              if (m_mgr.flush())
                                                m_write_header();
                                            }
  void close();

  // TODO: operator unspecified-bool-type, operator!
  
  // iterators:
    //   Non-const overloads are not provided because of the need to explicitly know
    //   when an update, if allowed, will occur. See map and multimap update() function.

  const_iterator     begin() const;
  const_iterator     end() const            { return m_end_iterator; }
  const_iterator     last() const;
  const_reverse_iterator
                     rbegin() const         { return reverse_iterator(cend()); }     
  const_reverse_iterator
                     rend() const           { return reverse_iterator(cbegin()); }

  const_iterator     cbegin() const         { return begin(); }
  const_iterator     cend() const           { return end(); }
  const_reverse_iterator
                     crbegin() const        { return reverse_iterator(cend()); }     
  const_reverse_iterator                    
                     crend() const          { return reverse_iterator(cbegin()); }

  // observers:

  bool               is_open() const        { return m_mgr.is_open(); }
  const filesystem::path&
                     path() const           { return m_mgr.path(); }
  flags::bitmask     flags() const          { return m_flags; }

  // TODO: why are these two exposed? See main TODO list above.
  const buffer_manager&
                     manager() const        { return m_mgr; }
  const header_page& header() const         { BOOST_ASSERT(is_open());
                                              return m_hdr; }
  key_compare        key_comp() const       { return m_comp; }
  value_compare      value_comp() const     { return m_value_comp; }

  // capacity:

  bool          empty() const               { return !size(); }
  size_type     size() const                { return m_hdr.element_count(); }
  size_type     max_size() const            { return -1; }
  bool          ok_to_pack() const          { return m_ok_to_pack; }
  std::size_t   node_size() const           { BOOST_ASSERT(is_open());
                                              return m_mgr.data_size(); }
  std::size_t   max_cache_size() const      { BOOST_ASSERT(is_open());
                                              return m_mgr.max_cache_size(); }
  void          max_cache_size(std::size_t m)  // -1 indicates unlimited
  { 
    BOOST_ASSERT(is_open());
    m_mgr.max_cache_size(m >= (header().levels()+1) ? m : (header().levels()+1));
  }
  void          max_cache_megabytes(std::size_t mb)
  { 
    BOOST_ASSERT(is_open());
    m_mgr.max_cache_size((mb*1048576)/node_size());
  }

  //  The following element access functions are not provided. Returning references is
  //  far too dangerous, since the memory pointed to would be in a node buffer that can
  //  overwritten by other activity, including calls to const functions. Access via
  //  iterators doesn't suffer the same since iterators contain reference counted smart
  //  pointers to the node's memory.
  //
  //  T&        operator[](const key_type& k);
  //  const T&  at(const key_type& k) const;
  //  T&        operator[](key_type&& k);
  //  T&        at(const key_type& k);


  // modifiers:

  //template <class... Args>
  //  pair<iterator, bool> emplace(Args&&... args);
  //template <class... Args>
  //  iterator emplace_hint(const_iterator position, Args&&... args);

  //template <class P>
  //iterator insert(const_iterator position, P&&);
  //void insert(initializer_list<value_type>);

  //  const_iterator is returned because of the need to explicitly know when an update,
  //  if allowed, will occur. See map and multimap update() function.
  const_iterator     erase(const_iterator position);
  size_type          erase(const key_type& k);
  const_iterator     erase(const_iterator first, const_iterator last);

  void               clear();

  // operations:
  //   Types K and Key are required to be key_comp() comparable
  //   Non-const overloads are not provided because of the need to explicitly know
  //   when an update, if allowed, will occur. See map and multimap update() function.
 
  template <class K>
    const_iterator   find(const K& k) const;
  const_iterator     find(const Key& k) const          {return find<Key>(k);}

  template <class K>
    size_type        count(const K& k) const;
  size_type          count(const Key& k) const         {return count<Key>(k);}

  template <class K>
    const_iterator   lower_bound(const K& k) const;
  const_iterator     lower_bound(const Key& k) const   {return lower_bound<Key>(k);}

  template <class K>
    const_iterator   upper_bound(const K& k) const;
  const_iterator     upper_bound(const Key& k) const   {return upper_bound<Key>(k);}

  template <class K>
    std::pair<const_iterator, const_iterator>
      equal_range(const K& k) const
        {return std::make_pair(lower_bound(k), upper_bound(k));}
  std::pair<const_iterator, const_iterator>
    equal_range(const Key& k) const
      {return std::make_pair(lower_bound<Key>(k), upper_bound<Key>(k));}

//------------------------------  inspect leaf-to-root  --------------------------------//

  bool inspect_leaf_to_root(std::ostream& os, const const_iterator& itr)
    // Returns: true if no errors detected
{
  btree_node* np = itr.node().get();
  for (;
       np->level() < header().root_level();
       np = np->parent().get())
  {
    if (!np->parent())
    {
      os  << "error: no parent() for " << np->node_id()
          << ", yet level=" << np->level()
          << ", use count=" << np->use_count()
          << ", levels=" << header().levels()
          << ", cache size=" << manager().buffers_in_memory()
          << ", cache active=" << manager().buffers_in_memory()
                                    - manager().buffers_available()
          << ", cache avail=" << manager().buffers_available()
          << ", branch pages =" << header().branch_node_count()
          << ", leaf pages =" << header().leaf_node_count()
          << ", size=" << size()
          << std::endl;
      manager().dump_buffers(os);
      manager().dump_available_buffers(os);
      return false;
    }
  }
  if (np->parent())
  {
    os  << "error: root " << np->node_id() << " has parent\n" 
        << "  level=" << np->level()
        << ", use count=" << np->use_count()
        << ", levels=" << header().levels()
        << ", cache size=" << manager().buffers_in_memory()
        << ", cache active=" << manager().buffers_in_memory()
                                  - manager().buffers_available()
        << ", cache avail=" << manager().buffers_available()
        << ", branch pages =" << header().branch_node_count()
        << ", leaf pages =" << header().leaf_node_count()
        << ", size=" << size()
        << std::endl;
    manager().dump_buffers(os);
    manager().dump_available_buffers(os);
    return false;
  }

  return true;
}

//--------------------------------------------------------------------------------------//
//                                private data members                                  //
//--------------------------------------------------------------------------------------//

private:

  //  The standard library mandates key_compare and value_compare types.
  //  The implementation also needs to compare a value_type to a key_type, and a
  //  branch_value_type to a key_type and visa versa.

  key_compare        m_comp;
  value_compare      m_value_comp;

  //  The following is defined at the end of the btree_base definition because of some
  //  complex ordering dependencies that I don't really understand:
  //     branch_compare  m_branch_comp;

  mutable
    buffer_manager   m_mgr;

  btree_node_ptr     m_root;  // invariant: there is always at least one leaf,
                              // possibly empty, in the tree, and thus there is
                              // always a root. If the tree has only one leaf
                              // node, that node is the root

  //  end iterator mechanism: needed so that decrement of end() is implementable
  buffer             m_end_node;  // end iterators point to this node, providing
                                  // access to "this" via buffer::manager() 
  const_iterator     m_end_iterator;

  header_page        m_hdr;

  std::size_t        m_max_leaf_elements;
  std::size_t        m_max_branch_elements;

  flags::bitmask     m_flags;
  bool               m_ok_to_pack;  // true while all inserts ordered and no erases
                                               

//--------------------------------------------------------------------------------------//
//                                private nested classes                                //
//--------------------------------------------------------------------------------------//

  //------------------------ disk data formats and operations --------------------------//

  //  Pages hold a sequences of elements, plus administrivia. See details below.

  class btree_data
  {
  public:
    unsigned         level() const         {return m_level;}
    void             level(unsigned lv)    {m_level = lv;}
    bool             is_leaf() const       {return m_level == 0;}
    bool             is_branch() const     {return m_level > 0 && m_level != 0xFF;}
    std::size_t      size() const          {return m_size;}  // std::size_t is correct!
    void             size(std::size_t sz)  {m_size
                                             = static_cast<uint_least32_t>(sz);}  // ditto

//  private:
    node_level_type  m_level;    // leaf: 0, branches: distance from leaf,
                                 // free node list entry: 0xFF
    node_size_type   m_size;     // # of elements; on branches excludes end pseudo-element
  };
  
  //------------------------ leaf data formats and operations --------------------------//

  class leaf_data : public btree_data
  {
    friend class btree_base;
  public:
    value_type*  begin()      {return m_value;}
    value_type*  end()        {return &m_value[btree_data::size()];}

    //  offsetof() macro won't work for all value types, so compute by hand
    static std::size_t value_offset()
    {
      static leaf_data dummy;
      static std::size_t off
        = reinterpret_cast<char*>(&dummy.m_value) - reinterpret_cast<char*>(&dummy);
      return off;
    }

    //  private:
    value_type  m_value[1];
  };

  //std::size_t char_distance(const void* from, const void* to)
  //{
  //  return reinterpret_cast<const char*>(to) - reinterpret_cast<const char*>(from);
  //}

  //char* char_ptr(void* from)
  //{
  //  return reinterpret_cast<char*>(from);
  //}

  //------------------------ branch data formats and operations ------------------------//

      //----------------------------- branch invariants ----------------------------//
      //                                                                            //
      //  Unique containers:      Pn < Kn <= Pn+1   Keys in Pn are < Kn             //
      //                                            Kn <= Keys in Pn+1              //
      //                                                                            //
      //  Non-unique containers:  Pn <= Kn <= Pn+1  Keys in Pn are <= Kn            //
      //                                            Kn <= Keys in Pn+1              //
      //----------------------------------------------------------------------------//

  struct branch_value_type
  {
    branch_value_type() {}
    branch_value_type(Key& k, node_id_type id) : node_id(id), key(k) {}

    node_id_type  node_id;   // branch insert/erase depend on this
    Key           key;       //  data member ordering
  };

  class branch_data : public btree_data
  {
  public:
    branch_value_type*  begin()    {return m_value;}

    // HEADS UP: end() branch_value_type* is dereferenceable, although only the node_id
    // is present. This because a B-Tree branch page has one more child id than the
    // number of keys. See the branch invariants above.
    branch_value_type*  end()      {return &m_value[btree_data::size()];}

    //  offsetof() macro won't work for all branch_value_type's, so compute by hand
    static std::size_t value_offset()
    {
      static branch_data dummy;
      static std::size_t off
        = reinterpret_cast<char*>(&dummy.m_value) - reinterpret_cast<char*>(&dummy);
      return off;
    }

//  private:
    branch_value_type  m_value[1];
  };

  class branch_node;

  //----------------------------------- btree_node -------------------------------------//

  class btree_node : public buffer
  {
  private:
    mutable btree_node_ptr      m_parent;     // by definition, parent is a branch node
    mutable branch_value_type*  m_parent_element;
# ifndef NDEBUG
    node_id_type       m_parent_node_id;  // allows assert that m_parent has not been
                                          // overwritten by faulty logic
# endif

  public:
    btree_node() : buffer() {}
    btree_node(buffer::buffer_id_type id, buffer_manager& mgr)
      : buffer(id, mgr) {}

    node_id_type       node_id() const                 {return node_id_type(buffer_id());}

    btree_node_ptr&    parent()                          {return m_parent;}
//    const btree_node_ptr&  parent() const                    {return m_parent;}
    void               parent(btree_node_ptr p)          {m_parent = p;}
    void               parent_reset()                    {m_parent.reset();}
    branch_value_type* parent_element()                  {return m_parent_element;}
    void               parent_element(branch_value_type* p) {m_parent_element = p;}
#   ifndef NDEBUG
    node_id_type       parent_node_id()                  {return m_parent_node_id;}
    void               parent_node_id(node_id_type id)   {m_parent_node_id = id;}
#   endif

    const btree_base&  owner() const
    {
      BOOST_ASSERT(buffer::manager());
      return *reinterpret_cast<const btree_base*>(buffer::manager()->owner());
    }
    leaf_data&         leaf()       {return *reinterpret_cast<leaf_data*>(buffer::data());}
    const leaf_data&   leaf() const {return *reinterpret_cast<const leaf_data*>(buffer::data());}
    branch_data&       branch()     {return *reinterpret_cast<branch_data*>(buffer::data());}
    unsigned           level() const         {return leaf().m_level;}
    void               level(unsigned lv)    {leaf().m_level = lv;}
    bool               is_leaf() const       {return leaf().is_leaf();}
    bool               is_branch() const     {return leaf().is_branch();}
    std::size_t        size() const          {return leaf().m_size;}  // std::size_t correct!
    void               size(std::size_t sz)  {leaf().m_size           // ditto
                                               = static_cast<uint_least32_t>(sz);}    
    bool               empty() const         {return leaf().m_size == 0;}

    btree_node_ptr     next_node()  // return next node at current level while
                                    // maintaining the child to parent chain
    {
      if (!parent())              // if this is the root, there is no next node
      {
        BOOST_ASSERT(level() == owner().header().root_level());
//  TODO: should be able to write "owner().root_level()"
        return btree_node_ptr();
      }

      btree_node_ptr   par(m_parent);
      branch_value_type*  par_element(m_parent_element);

      if (par_element != par->branch().end())
      {
        ++par_element;
      }
      else
      {
        par = parent()->next_node();
        if (!par)
          return par;
        par_element = par->branch().begin();
      }

      btree_node_ptr np(manager()->read(par_element->node_id));

      // maintain the child to parent chain
      np->parent(par);
      np->parent_element(par_element);
#     ifndef NDEBUG
      np->parent_node_id(par->node_id());
#     endif
      return np;
    }

    btree_node_ptr     prior_node()  // return prior node at current level while
                                     // maintaining the child to parent chain
    {
      if (!parent())              // if this is the root, there is no prior node
      {
        //if (level() != owner().header().root_level())
        //{
        //  std::cout << "id=" << node_id() << ", "
        //       << int(level()) << " " << int(owner().header().root_level()) << std::endl;
        //}
        BOOST_ASSERT(level()
          == owner().header().root_level());  // possibly broken leaf-to-root chain
        return btree_node_ptr();
      }

      btree_node_ptr   par(m_parent);
      branch_value_type*  par_element(m_parent_element);

      if (par_element != par->branch().begin())
      {
        --par_element;
      }
      else
      {
        par = parent()->prior_node();
        if (!par)
          return par;
        par_element = par->branch().end();
      }

      btree_node_ptr np(manager()->read(par_element->node_id));

      // maintain the child to parent chain
      np->parent(par);
      np->parent_element(par_element);
#     ifndef NDEBUG
      np->parent_node_id(par->node_id());
#     endif
      return np;
    }
  };

  //-------------------------------  btree_node_ptr  -----------------------------------//

  //  smart pointer; maintains buffer's use count and clear leaf-to-root parent chain
  //  when use count goes to 0.
  class btree_node_ptr : public buffer_ptr  
  {
  public:

    btree_node_ptr() : buffer_ptr() {}
    btree_node_ptr(btree_node& p) : buffer_ptr(p) {}
    btree_node_ptr(buffer& p) : buffer_ptr(p) {}
    btree_node_ptr(const btree_node_ptr& r) : buffer_ptr(r) {} 
    btree_node_ptr(const buffer_ptr& r) : buffer_ptr(r) {}

   ~btree_node_ptr()
    {
      reset();  // reset handles parent chain if present
    }

    btree_node_ptr& operator=(const btree_node_ptr& r)
    {
      btree_node_ptr tmp(r);  // if there is self-assignment, will ++ *this
      if (m_ptr && use_count() == 1)
        reset();  // reset handles parent chain if present 
      tmp.swap(*this);
      return *this;
    }

    btree_node_ptr& operator=(const buffer_ptr& r)
    {
      btree_node_ptr tmp(r);  // if there is self-assignment, will ++ *this
      if (m_ptr && use_count() == 1)
        reset();  // reset handles parent chain if present 
      tmp.swap(*this);
      return *this;
    }

    void reset()
    {
      if (m_ptr)
      {
        if (use_count() == 1     // buffer_ptr::reset will set use_count() to 0
          && get()->parent()     // && there is a parent()
          && get()->node_id() != static_cast<node_id_type>(-1)) // && not dummy
        {
          // reset the parent pointer
          btree_node* p(get());
          buffer_ptr::reset();   // reset the child before the parent
          p->parent_reset();     // so that the parent is more recently used
        }
        else
        {
          buffer_ptr::reset();
        }
      }
    }

    btree_node* get() const {return static_cast<btree_node*>(m_ptr);}
    btree_node& operator*() const
    {
      BOOST_ASSERT(m_ptr);
      return *static_cast<btree_node*>(m_ptr);
    }
    btree_node* operator->() const
    {
      BOOST_ASSERT(m_ptr);
      return static_cast<btree_node*>(m_ptr);
    }
  };

  //------------------------------------------------------------------------------------//
  //                                  iterator_type                                     //
  //------------------------------------------------------------------------------------//
 
  template <class T>
  class iterator_type
    : public boost::iterator_facade<iterator_type<T>, T, bidirectional_traversal_tag>
  {

  private:
    typename btree_base::btree_node_ptr  m_node;     // 0 for uninitialized iterator,
                                                     //   == m_end_node for end iterator
    T*                                   m_element;  // 0 for end iterator

  public:
    typedef typename btree_base::btree_node_ptr::use_count_type use_count_type;

    iterator_type(): m_element(0) {}
    iterator_type(btree_node_ptr p, T* e) : m_node(p), m_element(e) {}
    iterator_type(buffer_ptr p, T* e)
      : m_node(static_cast<typename btree_base::btree_node_ptr>(p)),
        m_element(e) {}

    iterator_type(buffer_ptr p)  // used solely to setup the end iterator
      : m_node(static_cast<typename btree_base::btree_node_ptr>(p)),
        m_element(0) {}

    template <class Other>
    iterator_type(iterator_type<Other> const& other)
      : m_node(other.m_node), m_element(other.m_element) {}

    typename
    btree_base::btree_node_ptr::use_count_type use_count() const
      {return m_node->use_count();}

  private:
    friend class boost::iterator_core_access;
    friend class btree_base;
    template <class> friend class iterator_type;
   
    T& dereference() const
    { 
      BOOST_ASSERT_MSG(m_node.get(),
        "Attempt to dereference uninitialized btree iterator");
      BOOST_ASSERT_MSG(m_element,
        "Attempt to dereference end btree iterator");
      return *m_element;
    }
 
    template <class Other>
    bool equal(iterator_type<Other> const& rhs) const
    {
      BOOST_ASSERT_MSG(m_node.get(), "Attempt to compare uninitialized btree iterator");
      return m_element == rhs.m_element
        // check node_id() in case node memory has been reused
        && m_node->node_id() == rhs.m_node->node_id();
    }

    void increment();
    void decrement();

    public:
    //  TODO: make these protected? AFAICR, only special test and debug code needs them
    typedef typename btree_base::btree_node_ptr  btree_node_ptr;
    const btree_node_ptr&  node() const {return m_node;}
  };

//--------------------------------------------------------------------------------------//
//                             protected member functions                               //
//--------------------------------------------------------------------------------------//
protected:

  std::pair<const_iterator, bool>
    m_insert_unique(const key_type& k);

  const_iterator
    m_insert_non_unique(const key_type& k);
  // Remark: Insert after any elements with equivalent keys, per C++ standard

  void m_open(const boost::filesystem::path& p, flags::bitmask flgs, uint64_t signature,
              const compare_type& comp, std::size_t node_sz);

  iterator m_write_cast(const_iterator itr)
  {
    itr.m_node->needs_write(true);
    return iterator(itr.m_node, const_cast<value_type*>(itr.m_element));
  }

//--------------------------------------------------------------------------------------//
//                              private member functions                                //
//--------------------------------------------------------------------------------------//
private:
  template <class Btree>
  friend void dump_dot(std::ostream& os, const Btree& bt); 
  
  void m_close_and_throw(const std::string& msg)
  {
    if (is_open())
      close();
    BOOST_BTREE_THROW(std::runtime_error(path().string()+" "+msg));
  }

  static buffer* m_node_alloc(buffer::buffer_id_type np_id, buffer_manager& mgr)
  { return new btree_node(np_id, mgr); }

  void m_read_header()
  {
    m_mgr.seek(0);
    m_mgr.binary_file::read(m_hdr);
    m_hdr.endian_flip_if_needed();
  }

  void m_write_header()
  {
    m_mgr.seek(0);
    m_hdr.endian_flip_if_needed();
    m_mgr.binary_file::write(m_hdr);
    m_hdr.endian_flip_if_needed();
  }

  template <class K>
  const_iterator m_special_lower_bound(const K& k) const;
  // returned const_iterator::m_element is the insertion point, and thus may be the 
  // past-the-end leaf const_iterator for const_iterator::m_node
  // postcondition: parent pointers are set, all the way up the chain to the root

  template <class K> 
  const_iterator m_special_upper_bound(const K& k) const;
  // returned const_iterator::m_element is the insertion point, and thus may be the 
  // past-the-end leaf const_iterator for const_iterator::m_node
  // postcondition: parent pointers are set, all the way up the chain to the root

  btree_node_ptr m_new_node(node_level_type lv);
  void  m_new_root();

  const_iterator m_leaf_insert(const_iterator insert_iter, const key_type& k);

  void m_branch_insert(btree_node_ptr np, branch_value_type* element, const key_type& k,
    btree_node_ptr child);  // insert key, child->node_id;
                            // set child's parent, parent_element

  void m_erase_branch_value(btree_node* np, branch_value_type* value);

  void  m_free_node(btree_node* np)  // add to free node list
  {
    if (np->is_leaf())
      m_hdr.decrement_leaf_node_count();
    else
      m_hdr.decrement_branch_node_count();
    np->needs_write(true);
    np->never_free(false);
    np->level(0xFF);
    np->size(0);
    np->branch().begin()->node_id = node_id_type(m_hdr.free_node_list_head_id());
    m_hdr.free_node_list_head_id(np->node_id());
  }

  //-------------------------------- branch_compare ------------------------------------//

  class branch_compare
//    : public std::binary_function<branch_value_type, key_type, bool>
  {
    friend class btree_base;
  protected:
    compare_type    m_comp;
    branch_compare() {}
    branch_compare(compare_type comp) : m_comp(comp) {}
  public:
   bool operator()(const branch_value_type& x, const branch_value_type& y) const
      {return m_comp(x.key, y.key);}
   template <class K>
   bool operator()(const K& x, const branch_value_type& y) const
      {return m_comp(x, y.key);}
   template <class K>
   bool operator()(const branch_value_type& x, const K& y) const
      {return m_comp(x.key, y);}
  };

  //------------------------ comparison function objects -------------------------------//

  branch_compare            m_branch_comp;

  branch_compare
    branch_comp() const { return m_branch_comp; }

};  // class btree_base


//--------------------------------------------------------------------------------------//
//                              non-member operator <<                                  //
//--------------------------------------------------------------------------------------//

template <class Key, class Base>   
std::ostream& operator<<(std::ostream& os,
  const btree_base<Key,Base>& bt)
{
  BOOST_ASSERT(bt.is_open());
  os << "  element count ------------: " << bt.header().element_count() << "\n" 
     << "  node size ----------------: " << bt.header().node_size() << "\n"
     << "  levels in tree -----------: " << bt.header().root_level()+1 << "\n"
     << "  node count, inc free list-: " << bt.header().node_count() << "\n"
     << "  leaf node count ----------: " << bt.header().leaf_node_count() << "\n"
     << "  branch node count --------: " << bt.header().branch_node_count() << "\n"
     << "  node count, without free -: " << bt.header().leaf_node_count()
                                            + bt.header().branch_node_count() << "\n"
     << "  root node id -------------: " << bt.header().root_node_id() << "\n"
     << "  free node list head id ---: " << bt.header().free_node_list_head_id() << "\n"
     << "  User supplied string -----: \"" << bt.header().user_c_str() << "\"\n"
     << "  OK to pack ---------------: " << bt.ok_to_pack() << "\n"
  ;
  return os;
}

//--------------------------------------------------------------------------------------//
//                          class btree_base implementation                             //
//--------------------------------------------------------------------------------------//

//------------------------------ construct without open --------------------------------//

template <class Key, class Base>
btree_base<Key,Base>::btree_base()
  // initialize in the correct order to avoid voluminous gcc warnings:
  : m_mgr(m_node_alloc)
{ 
  m_mgr.owner(this);

  // set up the end iterator
  m_end_node.manager(&m_mgr);
  m_end_iterator = const_iterator(buffer_ptr(m_end_node));
}

//------------------------------- construct with open ----------------------------------//

template <class Key, class Base>
btree_base<Key,Base>::btree_base(const boost::filesystem::path& p,
  flags::bitmask flgs, uint64_t signature, const compare_type& comp, std::size_t node_sz)
    : m_mgr(m_node_alloc)
{ 
  m_mgr.owner(this);

  // set up the end iterator
  m_end_node.manager(&m_mgr);
  m_end_iterator = const_iterator(buffer_ptr(m_end_node));
  BOOST_ASSERT(manager().buffers_in_memory() == 0);  // verify m_end_iterator not cached

  // open the file and set up data members
  m_open(p, flgs, signature, comp, node_sz);
}

//----------------------------------- destructor ---------------------------------------//

template <class Key, class Base>
btree_base<Key,Base>::~btree_base()
{
  try { close(); }
  catch (...) {}
}

//------------------------------------- close ------------------------------------------//

template <class Key, class Base>
void btree_base<Key,Base>::close()
{
  if (is_open())
  {
    flush();
    m_mgr.close();
  }
}

//-------------------------------------- open ------------------------------------------//

template <class Key, class Base>
void
btree_base<Key,Base>::m_open(const boost::filesystem::path& p,
  flags::bitmask flgs, uint64_t signature, const compare_type& comp, std::size_t node_sz) 
{
  BOOST_ASSERT(!is_open());
  BOOST_ASSERT(node_sz >= sizeof(btree::header_page));

  m_comp = comp;         // type key_compare, which is a typedef of compare_type

  m_value_comp = comp;   // type value_compare, which is a typedef of compare_type for sets, and 
                         // its own type for maps and has a constructor from type compare_type

  m_branch_comp = comp;  // type branch_compare, which is its own type and has a
                         // constructor from compare_type
  m_flags = flgs;

  if (cache_branches_default(flgs) & flags::cache_branches)
    m_flags |= flags::cache_branches;
  if (flgs & flags::truncate)
    m_flags |= flags::read_write;  // truncate implies read_write
 
  oflag::bitmask open_flags = oflag::in;
  if (flgs & flags::read_write)
    open_flags |= oflag::out;
  if (flgs & flags::truncate)
    open_flags |= oflag::out | oflag::truncate;
  if (flgs & flags::preload)
    open_flags |= oflag::preload;

  m_ok_to_pack = true;
  m_max_leaf_elements
    = (node_sz - leaf_data::value_offset()) / sizeof(value_type);
  m_max_branch_elements
    = (node_sz - sizeof(node_id_type) - branch_data::value_offset())
      / sizeof(branch_value_type);

  if (m_mgr.open(p, open_flags, 0, node_sz))
  { // existing non-truncated file
    m_read_header();
    if (!m_hdr.marker_ok())
      m_close_and_throw("isn't a btree");
    if (m_hdr.signature() != signature)
      m_close_and_throw("signature differs");
    if (m_hdr.big_endian() != (traits_type::header_endianness == endian::order::big))
      m_close_and_throw("endianness differs");
    if ((m_hdr.flags() & flags::key_only) != (flgs & flags::key_only))
      m_close_and_throw("map/set differs");
    if ((m_hdr.flags() & flags::unique) != (flgs & flags::unique))
      m_close_and_throw("multi/non-multi differs");
    if (m_hdr.key_size() != sizeof(key_type))
      m_close_and_throw("key size differs");
    if (m_hdr.mapped_size() != sizeof(mapped_type))
      m_close_and_throw("mapped size differs");

    m_mgr.data_size(m_hdr.node_size());
    m_root = m_mgr.read(m_hdr.root_node_id());
    max_cache_size(max_cache_default(flgs, boost::filesystem::file_size(p)));
  }
  else
  { // new or truncated file
    m_hdr.clear();
    m_hdr.big_endian(traits_type::header_endianness == endian::order::big);
    m_hdr.signature(signature);
    m_hdr.flags(flags::permanent_flags(flgs));
    m_hdr.splash_c_str("boost.org btree");
    m_hdr.user_c_str("");
    m_hdr.node_size(node_sz);
    m_hdr.key_size(sizeof(key_type));
    m_hdr.mapped_size(sizeof(mapped_type));
    m_hdr.increment_node_count();  // i.e. the header itself
    m_mgr.new_buffer();   // create a buffer, thus zeroing the header for its full size
    flush();              // write the header buffer
    m_mgr.clear_cache();  //  but don't cache it

    // set up an empty leaf as the initial root
    m_root = m_mgr.new_buffer();
    flush();
    m_root->needs_write(true);
    m_hdr.increment_node_count();
    m_hdr.increment_leaf_node_count();
    BOOST_ASSERT(m_root->node_id() == 1);
    m_hdr.root_node_id(m_root->node_id());
    m_hdr.last_node_id(m_root->node_id());
    m_root->level(0);
    m_root->size(0);
    max_cache_size(max_cache_default(flgs, 0));
  }
}

//------------------------------------- clear() ----------------------------------------//

template <class Key, class Base>
void
btree_base<Key,Base>::clear()
{
  BOOST_ASSERT_MSG(is_open(), "attempt to clear() unopen btree");

  manager().clear_write_needed();
  m_hdr.element_count(0);
  m_hdr.root_node_id(1);
  m_hdr.last_node_id(1);
  m_hdr.root_level(0);
  m_hdr.node_count(0);
  m_hdr.free_node_list_head_id(0);

  m_mgr.close();
}

//------------------------------------- begin() ----------------------------------------//

template <class Key, class Base>
typename btree_base<Key,Base>::const_iterator
btree_base<Key,Base>::begin() const
{
  BOOST_ASSERT_MSG(is_open(), "begin() on unopen btree");
  if (empty())
    return end();

  // starting at the root
  btree_node_ptr np = m_root;

  // work down the tree until a leaf is reached
  while (np->is_branch())
  {
    // create the child->parent list
    btree_node_ptr child_np = m_mgr.read(np->branch().begin()->node_id);
    child_np->parent(np);
    child_np->parent_element(np->branch().begin());
#   ifndef NDEBUG
    child_np->parent_node_id(np->node_id());
#   endif

    np = child_np;
  }

  BOOST_ASSERT(np->is_leaf());
  return const_iterator(np, np->leaf().begin());
}

//-------------------------------------- last() ---------------------------------------//

template <class Key, class Base>
typename btree_base<Key,Base>::const_iterator
btree_base<Key,Base>::last() const
{
  BOOST_ASSERT_MSG(is_open(), "last() on unopen btree");
  if (empty())
    return end();

  // starting at the root
  btree_node_ptr np = m_root;

  // work down the tree until a leaf is reached
  while (np->is_branch())
  {
    // create the child->parent list
    btree_node_ptr child_np = m_mgr.read(np->branch().end()->node_id);
    child_np->parent(np);
    child_np->parent_element(np->branch().end());
#   ifndef NDEBUG
    child_np->parent_node_id(np->node_id());
#   endif

    np = child_np;
  }

  BOOST_ASSERT(np->is_leaf());
  return const_iterator(np, np->leaf().end()-1);
}

//---------------------------------- m_new_node() --------------------------------------//

template <class Key, class Base>   
typename btree_base<Key,Base>::btree_node_ptr 
btree_base<Key,Base>::m_new_node(node_level_type lv)
{
  btree_node_ptr np;
  if (m_hdr.free_node_list_head_id())
  {
    np = m_mgr.read(m_hdr.free_node_list_head_id());
    BOOST_ASSERT(np->level() == 0xFF);  // free node list entry
    m_hdr.free_node_list_head_id(np->branch().begin()->node_id);
  }
  else
  {
    np = m_mgr.new_buffer();
    m_hdr.increment_node_count();
    BOOST_ASSERT(m_hdr.node_count() == m_mgr.buffer_count());
  }

  if (lv)  // is branch
    m_hdr.increment_branch_node_count();
  else
    m_hdr.increment_leaf_node_count();

  np->needs_write(true);
  np->never_free(lv > 0 && (flags() & flags::cache_branches));
//  cout << "******* lv:" << int(lv) << " cache_branches():" << cache_branches()
//    << " never_free:" << np->never_free() << endl;;
  np->level(lv);
  np->size(0);
  np->parent_reset();     // better safe than sorry
  np->parent_element(0);   // ditto
  return np;
}

//----------------------------------- m_new_root() -------------------------------------//

template <class Key, class Base>   
void
btree_base<Key,Base>::m_new_root()
{ 
  // create a new root containing only the P0 pseudo-element
  btree_node_ptr old_root = m_root;
  node_id_type old_root_id(m_root->node_id());
  m_hdr.increment_root_level();

  //  maintain max_cache_size() > levels() invariant
  if (max_cache_size() < header().levels() + 1)
    max_cache_size(header().levels() + 1);

  m_root = m_new_node(m_hdr.root_level());
  m_hdr.root_node_id(m_root->node_id());
  m_root->branch().begin()->node_id = old_root_id;
  m_root->size(0);  // the end pseudo-element doesn't count as an element
  m_root->parent(btree_node_ptr());  
  m_root->parent_element(0);
  old_root->parent(m_root);
  old_root->parent_element(m_root->branch().begin());
# ifndef NDEBUG
  m_root->parent_node_id(node_id_type(0));
  old_root->parent_node_id(m_root->node_id());
# endif
}

//---------------------------------- m_leaf_insert() -----------------------------------//

//  The key only is inserted; for sets the key_type is the value_type so nothing further
//  is required, but for maps the map class must emplace the mapped type into the element.
//  See include/boost/btree/map.hpp for the code that handles this.

template <class Key, class Base>   
typename btree_base<Key,Base>::const_iterator
btree_base<Key,Base>::m_leaf_insert(const_iterator insert_iter, const key_type& k)
{
  //std::cout << "m_leaf_insert: " << k << std::endl;
  btree_node_ptr       np = insert_iter.node();
  value_type*          insert_begin = const_cast<value_type*>(insert_iter.m_element);
  btree_node_ptr       np2;
  
  BOOST_ASSERT_MSG(np, "internal error");
  BOOST_ASSERT_MSG(np->is_leaf(), "internal error");
  BOOST_ASSERT_MSG(np->size() <= m_max_leaf_elements, "internal error");

  m_hdr.increment_element_count();
  np->needs_write(true);

  if (np->size() == m_max_leaf_elements)  // no room on node?
  {
    //  no room on node, so node must be split

    if (np->level() == m_hdr.root_level()) // splitting the root?
      m_new_root();  // create a new root
    
    np2 = m_new_node(np->level());  // create the new node 

    // ck pack optimization now, since header().last_node_id() may change
    if (m_ok_to_pack
        && (insert_begin != np->leaf().end()
            || np->node_id() != header().last_node_id()))
      m_ok_to_pack = false;  // conditions for pack optimization not met

    // if last leaf node being split, update header
    if (np->node_id() == header().last_node_id())
      m_hdr.last_node_id(np2->node_id());

    // apply pack optimization if applicable
    if (m_ok_to_pack)
    {
      // pack optimization: instead of splitting np, just put value alone on np2
      std::memcpy(&*np2->leaf().begin(), &k, sizeof(key_type));  // insert key
      np2->size(1);
      BOOST_ASSERT(np->parent()->node_id() == np->parent_node_id()); // cache logic OK?
      m_branch_insert(np->parent(), np->parent_element(),
        this->key(*np2->leaf().begin()), np2);
      return const_iterator(np2, np2->leaf().begin());
    }

    // split node np by moving half the elements to node np2
    std::size_t split_sz = np->size() / 2;  // round down to speed copy
    BOOST_ASSERT(split_sz);
    value_type* split_begin = np->leaf().begin() + (np->size() - split_sz);

    // TODO: if the insert point will fall on the new node, it would be faster to
    // copy the portion before the insert point, copy the value being inserted, and
    // finally copy the portion after the insert point. However, that's a fair amount
    // of additional code for something that on average only happens on half of all
    // leaf splits.

    std::memcpy(np2->leaf().begin(), split_begin, split_sz * sizeof(value_type));
    np2->size(split_sz);

    // finalize work on the original node
# ifndef NDEBUG
    std::memset(split_begin, 0,                         // zero unused space to make
      (np->leaf().end()-split_begin)*sizeof(value_type));  //  file dumps easier to read
# endif
    np->size(np->size() - split_sz);

    // adjust np and insert_begin if they now fall on the new node due to the split
    if (split_begin < insert_begin)
    {
      np = np2;
      insert_begin = np->leaf().begin() + (insert_begin - split_begin);
    }
  }

  //  insert key into np at insert_begin
  BOOST_ASSERT(insert_begin >= np->leaf().begin());
  BOOST_ASSERT(insert_begin <= np->leaf().end());
  std::size_t memmove_sz = (np->leaf().end() - insert_begin) * sizeof(value_type);
  std::memmove(insert_begin+1, insert_begin, memmove_sz);  // make room
  std::memcpy(insert_begin, &k, sizeof(key_type));   // insert value
  np->size(np->size()+1);

  // if there is a new node, its initial key and node_id are inserted into parent
  if (np2)
  {
    BOOST_ASSERT(insert_iter.m_node->parent()->node_id()
      == insert_iter.m_node->parent_node_id()); // cache logic OK?
    m_branch_insert(insert_iter.m_node->parent(),
      insert_iter.m_node->parent_element(),
      this->key(*np2->leaf().begin()), np2);

    BOOST_ASSERT(np2->parent());          // m_branch_insert should have set parent
    BOOST_ASSERT(np2->parent_element() != 0);  // and parent_element
  }

  return const_iterator(np, insert_begin);
}

//---------------------------------- m_branch_insert() ---------------------------------//

//  This logic is very similar to the m_leaf_insert() logic, so it might seem they should
//  combined into a single function. This was done in the 25 year-old C language function
//  that the current function is based on, and it was very messy, hard to understand, and
//  bug prone. The initial C++ attempt tried this single function approach, and seemed
//  subject to those same ills. The two function approach has been much less troublesome
//  right from the start.

template <class Key, class Base>   
void
btree_base<Key,Base>::m_branch_insert(btree_node_ptr np,
  branch_value_type* element, const key_type& k, btree_node_ptr child) 
{
  //std::cout << "m_branch_insert into node " << np->node_id() << ", level " << np->level()
  //          << ", id " << child->node_id()
  //          << ", key " << k
  //          << std::endl;
  btree_node_ptr    np2;

  BOOST_ASSERT(np->is_branch());
  BOOST_ASSERT(np->size() <= m_max_branch_elements);

  np->needs_write(true);

  if (np->size() == m_max_branch_elements)  // no room on node?
  {
    //  no room on node, so node must be split

    if (np->level() == m_hdr.root_level()) // splitting the root?
      m_new_root();  // create a new root
    
    np2 = m_new_node(np->level());  // create the new node

    // apply pack optimization if applicable
    if (m_ok_to_pack)
    {
      // instead of splitting np, just copy child's node_id to np2
      np2->branch().begin()->node_id = child->node_id();
      //  set the child's parent and parent_element
      child->parent(np2);
      child->parent_element(np2->branch().begin()); 

      BOOST_ASSERT(np->parent()->node_id() == np->parent_node_id()); // cache logic OK?

      m_branch_insert(np->parent(), np->parent_element(), k, np2);
      return;
    }

    // split node np by moving half the elements to node p2

    std::size_t np2_sz = np->size() / 2;
    std::size_t np_sz = np->size() - np2_sz;
    np->size(np_sz - 1);  // -1 to account for end pseudo-element

    // promote the key from the new end pseudo element to the parent branch node
    m_branch_insert(np->parent(), np->parent_element(), np->branch().end()->key, np2);

    // Note: if the insert point will fall on the new node, it would be faster to
    // copy the portion before the insert point, copy the value being inserted, and
    // finally copy the portion after the insert point. However, that's a fair amount of
    // additional code for something that only happens on half of all branch splits
    // on average.

    // copy the split elements, including the pseudo-end element, to np2
    std::memcpy(np2->branch().begin(), np->branch().end() + 1,
      np2_sz * sizeof(branch_value_type) + sizeof(node_id_type));  // include end pseudo element
    np2->size(np2_sz);  // exclude end pseudo element from size

    BOOST_ASSERT(np->parent()->node_id() == np->parent_node_id()); // cache logic OK?


    // finalize work on the original node
# ifndef NDEBUG
    std::memset(&np->branch().end()->key, 0,  // zero unused space so dumps easier to read
      (m_max_branch_elements - np->size()) * sizeof(branch_value_type) - sizeof(key_type)); 
# endif

    // adjust np and insert_begin if they now fall on the new node due to the split
    if (!(element >= np->branch().begin() && element <= np->branch().end()))
    {
      std::size_t element_offset =  element - np->branch().end() - 1;
      element = np2->branch().begin() + element_offset;
      np = np2;
    }
  }  // split finished

  BOOST_ASSERT(np->size() < m_max_branch_elements);

  //  insert k, id, into np at &element->key
  BOOST_ASSERT(element >= np->branch().begin());
  BOOST_ASSERT(element <= np->branch().end());
  
  std::size_t move_sz = (np->branch().end() - element) * sizeof(branch_value_type);
  std::memmove(&(element+1)->key, &element->key, move_sz);  // make room
  std::memcpy(&element->key, &k, sizeof(key_type));         // insert k
  (element+1)->node_id = child->node_id();                  // insert node_id
  np->size(np->size() + 1);

  //  set the child's parent and parent_element
  child->parent(np);
  child->parent_element(element+1);

#ifndef NDEBUG
  if (m_hdr.flags() & btree::flags::unique)
  {
    //std::cout << "audit node " << np->node_id()
    //          << ", size " << np->size() << std::endl;
    branch_value_type* cur = np->branch().begin();
    const key_type* prev_key;
    for(; cur != np->branch().end(); ++cur)
    {
      //std::cout << "m_branch_insert audit key: " << cur->key << std::endl;
      BOOST_ASSERT(cur == np->branch().begin()
        || key_comp()(*prev_key, cur->key));
      prev_key = &cur->key;
    }
    //std::cout << " audit done" << std::endl;
  }
#endif
}

//------------------------------------- erase() ----------------------------------------//

//  erases leaf only; see m_erase_branch_value() for branches
template <class Key, class Base>   
typename btree_base<Key,Base>::const_iterator
btree_base<Key,Base>::erase(const_iterator pos)
{
  BOOST_ASSERT_MSG(is_open(), "erase() on unopen btree");
  BOOST_ASSERT_MSG((flags() & flags::read_only) == 0, "erase() on read only btree");
  BOOST_ASSERT_MSG(pos != end(), "erase() on end iterator");
  BOOST_ASSERT(pos.m_node);
  BOOST_ASSERT(pos.m_node->is_leaf());
  BOOST_ASSERT(pos.m_element < pos.m_node->leaf().end());
  BOOST_ASSERT(pos.m_element >= pos.m_node->leaf().begin());

  //std::cout << "erase " << key(*pos.m_element) << std::endl;

  m_ok_to_pack = false;  // TODO: is this too conservative?
  pos.m_node->needs_write(true);
  m_hdr.decrement_element_count();

  if (pos.m_node->node_id() != m_root->node_id()  // not root?
        && pos.m_node->size() == 1)  // 1 element node?
  {
    // erase a single value leaf node that is not the root

    BOOST_ASSERT(pos.m_node->parent()->node_id() \
      == pos.m_node->parent_node_id()); // cache logic OK?

    // Save the prior node to use below to obtain the return iterator. The prior node
    // is saved rather than the next node, because the next node pointer will be
    // invalidated by m_branch_erase_value(), while the prior node pointer will not be
    // invalidated by m_branch_erase_value().
    btree_node_ptr prior_node(pos.m_node->prior_node());  // may be null

    if (pos.m_node->node_id() == header().last_node_id())
    {
      BOOST_ASSERT(prior_node);  // logic error; assert will only fire if this leaf is
                                 // the root, but this control path should not be
                                 // taken if this leaf is the root
      m_hdr.last_node_id(prior_node->node_id());
    }

    m_erase_branch_value(pos.m_node->parent().get(), pos.m_node->parent_element());

    m_free_node(pos.m_node.get());  // add node to free node list

    if (prior_node)
    {
      btree_node_ptr next_node(prior_node->next_node());
      return next_node
        ? const_iterator(next_node, next_node->leaf().begin())
        : cend();
    }
    return cbegin();
  }
  else
  {
    // erase an element from a leaf with multiple elements or erase the only element
    // on a leaf that is also the root; these use the same logic because they do not remove
    // the node from the tree.

    value_type* element = const_cast<value_type*>(pos.m_element);
    std::size_t move_sz = (pos.m_node->leaf().end() - (element+1)) * sizeof(value_type);
    BOOST_ASSERT(element + 1 + move_sz/sizeof(value_type) <= pos.m_node->leaf().end());
    //std::cout << "moving " << move_sz << std::endl;
    std::memmove(element, element+1, move_sz);
    pos.m_node->size(pos.m_node->size() - 1);
    std::memset(pos.m_node->leaf().end(), 0, sizeof(value_type));

    if (pos.m_element != pos.m_node->leaf().end())
      return pos;
    btree_node_ptr next_node(pos.m_node->next_node());
    return !next_node ? cend() : const_iterator(next_node, next_node->leaf().begin());
  }
}

//------------------------------ m_erase_branch_value() --------------------------------//

template <class Key, class Base>
void btree_base<Key,Base>::m_erase_branch_value(
  btree_node* np, branch_value_type* element)
{
  BOOST_ASSERT(np->is_branch());
  BOOST_ASSERT(element >= np->branch().begin());
  BOOST_ASSERT(element <= np->branch().end());  // equal to end if pseudo-element only

  if (np->empty()) // end pseudo-element only element on node?
                   // i.e. after the erase, the entire sub-tree will be empty
  {
    BOOST_ASSERT(np->level() != header().root_level());
    BOOST_ASSERT(np->parent()->node_id() == np->parent_node_id()); // cache logic OK?

    m_erase_branch_value(np->parent().get(),
      np->parent_element()); // erase parent value pointing to np
    m_free_node(np); // move node to free node list
  }
  else
  { // erase element that is not on a P0-only page

    char* erase_ptr;
    std::size_t move_sz;

    if (element != np->branch().begin())
    {
      move_sz = (np->branch().end() - element) * sizeof(branch_value_type);
      --element;
      erase_ptr = reinterpret_cast<char*>(&element->key);
    }
    else
    {
      erase_ptr = reinterpret_cast<char*>(&element->node_id);
      move_sz = ((np->size() - 1 ) * sizeof(branch_value_type)) + sizeof(node_id_type);
    }

    std::memmove(erase_ptr, erase_ptr + sizeof(branch_value_type), move_sz);

    np->size(np->size() - 1);
    std::memset(reinterpret_cast<char*>(np->branch().end()) + sizeof(node_id_type),
      0, sizeof(branch_value_type));
    np->needs_write(true);

    //  recursively free the root node if it is now empty, promoting the end
    //  pseudo element node_id to be the new root
    while (np->level()   // not the leaf (which can happen if iteration reaches leaf)
      && np->branch().begin() == np->branch().end()  // node empty except for P0
      && np->level() == header().root_level())   // node is the root
    {
      // make the end pseudo-element the new root and then free this node
      np->needs_write(true);
      m_hdr.root_node_id(np->branch().end()->node_id);
      //std::cout << "new root " << header().root_node_id() << std::endl;
      m_hdr.decrement_root_level();
      m_root = m_mgr.read(header().root_node_id());
      m_root->parent(btree_node_ptr());
      m_root->parent_element(0);
      m_free_node(np); // move node to free node list
      np = m_root.get();
    }
  }
}

template <class Key, class Base>   
typename btree_base<Key,Base>::size_type
btree_base<Key,Base>::erase(const key_type& k)
{
  BOOST_ASSERT_MSG(is_open(), "erase() on unopen btree");
  BOOST_ASSERT_MSG((flags() & flags::read_only) == 0, "erase() on read only btree");
  size_type count = 0;
  const_iterator it = lower_bound(k);
    
  while (it != end() && !key_comp()(k, this->key(*it)))
  {
    ++count;
    it = erase(it);
  }
  return count;
}

template <class Key, class Base>   
typename btree_base<Key,Base>::const_iterator 
btree_base<Key,Base>::erase(const_iterator first, const_iterator last)
{
  BOOST_ASSERT_MSG(is_open(), "erase() on unopen btree");
  BOOST_ASSERT_MSG((flags() & flags::read_only) == 0, "erase() on read only btree");
  //
  while (first != last)
  {
    // last must be adjusted when on the same node as first
    if (last != end() && first.m_node == last.m_node)
    {
      BOOST_ASSERT(first.m_element <= last.m_element);
      --last;  // adjust in anticipation of erasing a prior element on same node
    }
    first = erase(first);  // this form of is essential because it does not require
                           // last have a valid leaf-to-root parent chain
  }
  return last;
}

//--------------------------------- m_insert_unique() ----------------------------------//

template <class Key, class Base>   
std::pair<typename btree_base<Key,Base>::const_iterator, bool>
btree_base<Key,Base>::m_insert_unique(const key_type& k)
{
  BOOST_ASSERT_MSG(is_open(), "insert() on unopen btree");
  BOOST_ASSERT_MSG((flags() & flags::read_only) == 0, "insert() on read only btree");
  const_iterator insert_point = m_special_lower_bound(k);

  bool is_unique = insert_point.m_element == insert_point.m_node->leaf().end()
                || key_comp()(k, this->key(*insert_point))
                || key_comp()(this->key(*insert_point), k);

  if (is_unique)
    return std::pair<const_iterator, bool>(m_leaf_insert(insert_point, k), true);

  return std::pair<const_iterator, bool>(
    const_iterator(insert_point.m_node, insert_point.m_element), false); 
}

//------------------------------- m_insert_non_unique() --------------------------------//

template <class Key, class Base>   
inline typename btree_base<Key,Base>::const_iterator
btree_base<Key,Base>::m_insert_non_unique(const key_type& k)
{
  BOOST_ASSERT_MSG(is_open(), "insert() on unopen btree");
  BOOST_ASSERT_MSG((flags() & flags::read_only) == 0, "insert() on read only btree");
  const_iterator insert_point = m_special_upper_bound(k);
  return m_leaf_insert(insert_point, k);
}

//----------------------------- m_special_lower_bound() --------------------------------//

template <class Key, class Base>
template <class K>
typename btree_base<Key,Base>::const_iterator
btree_base<Key,Base>::m_special_lower_bound(const K& k) const
//  m_special_lower_bound() differs from lower_bound() in that if the search key is not
//  present and the first key greater than the search key is the first key on a leaf
//  other than the first leaf, the returned iterator points to the end element of
//  the leaf prior to the leaf whose first element is the true lower bound.
//
//  Those semantics are useful because the returned iterator is pointing to the insertion
//  point for unique inserts and does not miss the first key in a series of non-unique
//  keys that do not begin on a node boundary.

//  Analysis; consider a branch node with these entries:
//
//     P0, K0="B", P1, K1="D", P2, K2="F", P3, ---
//     ----------  ----------  ----------  ------------------
//     element 0   element 1   element 2   end pseudo-element
//
//  Search key:  A  B  C  D  E  F  G
//  std::lower_bound  
//   element     0  0  1  1  2  2  end pseudo-element
//  child_np->   0  0  1  1  2  2  end pseudo-element
//   parent_element
//  Child node:  P0 P1 P1 P2 P2 P3 P3
{
  btree_node_ptr np = m_root;

  // search branches down the tree until a leaf is reached
  while (np->is_branch())
  {
    branch_value_type* low
      = std::lower_bound(np->branch().begin(), np->branch().end(), k, branch_comp());

    if ((header().flags() & btree::flags::unique)
      && low != np->branch().end()
      && !key_comp()(k, low->key)) // if k isn't less that low->key(), it is equal
      ++low;                         // and so must be incremented; this follows from
                                     // the branch node invariant for unique containers

    // create the child->parent list
    btree_node_ptr child_np = m_mgr.read(low->node_id);
    child_np->parent(np);
    child_np->parent_element(low);
#   ifndef NDEBUG
    child_np->parent_node_id(np->node_id());
#   endif

    np = child_np;
  }

  //  search leaf
  value_type* low
    = std::lower_bound(np->leaf().begin(), np->leaf().end(), k, value_comp());

  return const_iterator(np, low);
}

//---------------------------------- lower_bound() -------------------------------------//

template <class Key, class Base>
template <class K>
typename btree_base<Key,Base>::const_iterator
btree_base<Key,Base>::lower_bound(const K& k) const
{
  BOOST_ASSERT_MSG(is_open(), "lower_bound() on unopen btree");

  const_iterator low = m_special_lower_bound(k);

  if (low.m_element != low.m_node->leaf().end())
    return low;

  if (low.m_node->leaf().begin() == low.m_node->leaf().end())
  {
    BOOST_ASSERT(empty());
    return end();
  }

  // lower bound is first element on next node
  btree_node_ptr np = low.m_node->next_node();
  return np ? const_iterator(np, np->leaf().begin()) : end();
}

//------------------------------ m_special_upper_bound() -------------------------------//

template <class Key, class Base>
template <class K> 
typename btree_base<Key,Base>::const_iterator
btree_base<Key,Base>::m_special_upper_bound(const K& k) const
{
  btree_node_ptr np = m_root;

  // search branches down the tree until a leaf is reached
  while (np->is_branch())
  {
    branch_value_type* up
      = std::upper_bound(np->branch().begin(), np->branch().end(), k, branch_comp());

    // create the child->parent list
    btree_node_ptr child_np = m_mgr.read(up->node_id);
    child_np->parent(np);
    child_np->parent_element(up);
#   ifndef NDEBUG
    child_np->parent_node_id(np->node_id());
#   endif

    np = child_np;
  }

  //  search leaf
  value_type* up
    = std::upper_bound(np->leaf().begin(), np->leaf().end(), k, value_comp());

  return const_iterator(np, up);
}

//---------------------------------- upper_bound() -------------------------------------//

template <class Key, class Base>
template <class K> 
typename btree_base<Key,Base>::const_iterator
btree_base<Key,Base>::upper_bound(const K& k) const
{
  BOOST_ASSERT_MSG(is_open(), "upper_bound() on unopen btree");

  const_iterator up = m_special_upper_bound(k);

  if (up.m_element != up.m_node->leaf().end())
    return up;

  // upper bound is first element on next node
  btree_node_ptr np = up.m_node->next_node();
  return np ? const_iterator(np, np->leaf().begin()) : end();
}

//------------------------------------- find() -----------------------------------------//

template <class Key, class Base>
template <class K>
typename btree_base<Key,Base>::const_iterator
btree_base<Key,Base>::find(const K& k) const
{
  BOOST_ASSERT_MSG(is_open(), "find() on unopen btree");
  const_iterator low = lower_bound(k);
  return (low != end() && !key_comp()(k, this->key(*low)))
    ? low
    : end();
}

//------------------------------------ count() -----------------------------------------//

template <class Key, class Base>
template <class K> 
typename btree_base<Key,Base>::size_type
btree_base<Key,Base>::count(const K& k) const
{
  BOOST_ASSERT_MSG(is_open(), "lower_bound() on unopen btree");
  size_type count = 0;

  for (const_iterator it = lower_bound(k);
        it != end() && !key_comp()(k, this->key(*it));
        ++it) { ++count; } 

  return count;
}

////  non-member functions  ----------------------------------------------------//

// dump tree using Graphviz dot format
template <class Btree>
void   dump_dot(std::ostream& os, const Btree& bt); 


///  TODO: non-member functions not implemented yet
/*
template <typename Key, typename T, typename Compare>
bool operator==(const common_base<Key,T,GetKey>& x,
              const common_base<Key,T,GetKey>& y);

template <typename Key, typename T, typename Compare>
bool operator< (const common_base<Key,T,GetKey>& x,
              const common_base<Key,T,GetKey>& y);

template <typename Key, typename T, typename Compare>
bool operator!=(const common_base<Key,T,GetKey>& x,
              const common_base<Key,T,GetKey>& y);

template <typename Key, typename T, typename Compare>
bool operator> (const common_base<Key,T,GetKey>& x,
              const common_base<Key,T,GetKey>& y);

template <typename Key, typename T, typename Compare>
bool operator>=(const common_base<Key,T,GetKey>& x,
              const common_base<Key,T,GetKey>& y);

template <typename Key, typename T, typename Compare>
bool operator<=(const common_base<Key,T,GetKey>& x,
              const common_base<Key,T,GetKey>& y);

template <typename Key, typename T, typename Compare>
void swap(common_base<Key,T,GetKey>& x,
        common_base<Key,T,GetKey>& y);
*/

//----------------------------------- dump_dot -----------------------------------------//

template <class Btree>
void dump_dot(std::ostream& os, const Btree& bt) 
{
  BOOST_ASSERT_MSG(bt.is_open(), "dump_dot() on unopen btree");

  os << "digraph btree {\nrankdir=LR;\nfontname=Courier;\n"
    "node [shape = record,margin=.1,width=.1,height=.1,fontname=Courier"
    ",style=\"filled\"];\n";

  for (unsigned int p = 1; p < bt.header().node_count(); ++p)
  {
    typename Btree::btree_node_ptr np = bt.m_mgr.read(p);

    if (np->is_leaf())
    {
      os << "node" << p << "[label = \"<f0> " << p
         << ", use-ct=" << np->use_count()-1 << "|";
      for (typename Btree::value_type* it = np->leaf().begin();
        it != np->leaf().end(); ++it)
      {
        if (it != np->leaf().begin())
          os << '|';
        os << bt.key(*it);
      }
      os << "\",fillcolor=\"palegreen\"];\n";
    }
    else if (np->is_branch())
    {
      os << "node" << p << "[label = \"<f0> " << p
         << ", use-ct=" << np->use_count()-1 << "|";
      int f = 1;
      typename Btree::branch_value_type* it;
      for (it = np->branch().begin(); it != np->branch().end(); ++it)
      {
        os << "<f" << f << ">|" << it->key << "|";
        ++f;
      }
      os << "<f" << f << ">\",fillcolor=\"lightblue\"];\n";
      f = 1;
      for (it = np->branch().begin(); it != np->branch().end(); ++it)
      {
        os << "\"node" << p << "\":f" << f
           << " -> \"node" << it->node_id << "\":f0;\n";
        ++f;
      }
      os << "\"node" << p << "\":f" << f 
         << " -> \"node" << it->node_id << "\":f0;\n";
    }
  }

  os << "}" << std::endl;
}

//--------------------------------------------------------------------------------------//

template <class Key, class Base>
template <class T>
void
btree_base<Key,Base>::iterator_type<T>::increment()
{
  BOOST_ASSERT_MSG(m_node.get(),
    "Attempt to increment uninitialized btree iterator");
  BOOST_ASSERT_MSG(m_element, "Attempt to increment end btree iterator"); 
  BOOST_ASSERT(m_element >= m_node->leaf().begin());
  BOOST_ASSERT(m_element < m_node->leaf().end());

  if (++m_element != m_node->leaf().end())
    return;

  btree_node_ptr np(m_node);
  m_node = m_node->next_node();

  if (m_node)
  {  
    m_element = m_node->leaf().begin();
    BOOST_ASSERT(m_element != m_node->leaf().end());
  }
  else // end() reached
  {
    *this = reinterpret_cast<const btree_base<Key,Base>*>
        (np->manager()->owner())->m_end_iterator;
  }
}

template <class Key, class Base>
template <class T>
void
btree_base<Key,Base>::iterator_type<T>::decrement()
{
  BOOST_ASSERT_MSG(m_node.get(),
    "Attempt to decrement uninitialized btree iterator");
  if (*this == reinterpret_cast<const btree_base<Key,Base>*>
        (m_node->manager()->owner())->end())
    *this = reinterpret_cast<btree_base<Key,Base>*>
        (m_node->manager()->owner())->last();
  else if (m_element != m_node->leaf().begin())
    --m_element;
  else
  {
    btree_node_ptr np(m_node);
    m_node = m_node->prior_node();

    if (m_node)
    {  
      m_element = m_node->leaf().end();
      BOOST_ASSERT(m_element != m_node->leaf().begin());
      --m_element;
    }
    else // end() reached
    {
      *this = reinterpret_cast<const btree_base<Key,Base>*>
          (np->manager()->owner())->m_end_iterator;
    }
  }
}

}  // namespace btree
}  // namespace boost

#endif // BOOST_BTREE_BASES_HPP

