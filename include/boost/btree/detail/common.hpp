//  boost/btree/detail/common.hpp  -----------------------------------------------------//

//  Copyright Beman Dawes 2000, 2006, 2010

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  See http://www.boost.org/libs/btree for documentation.

#ifndef BOOST_BTREE_COMMON_HPP
#define BOOST_BTREE_COMMON_HPP

#include <boost/iterator/iterator_facade.hpp>
#include <boost/noncopyable.hpp>
#include <boost/btree/detail/buffer_manager.hpp>
#include <boost/assert.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/if.hpp>
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


  * btree_unit_test.cpp: move erase tests out of insert test.

  * Upgrade m_update() to allow new dynamic size different from old dynamic size

  * Add static_assert Key, T are is_trivially_copyable

  * Should header() be part of the public interface?
      - Add individual get, and where appropriate, set, functions.
      - Move header file to detail.

  * For multi-containers, add a test case of a deep tree with all the same key. Then
    test erasing various elements.

  * Pack optimization should apply to branches too.

  * Either add code to align mapped() or add a requirement that PID, K does not
    require alignment.

  * Add a function (apply(key, mapped_value)?) that inserts if key not present,
    updates if present.

  * Header should contain uuid for value_type; used to check existing file is being
    opened with the right template parameters.

  * The commented out logging in binary_file.cpp was very useful. (1) move it to header
    and (2) apply only when BOOST_BINARY_FILE_LOG is defined. This implies adding m_ to
    the actual binary_file.cpp implementation names.

  * Preload option currently is just passed on to binary_file, which reads the entire file.
    That preloads the O/S cache, but does nothing for the btree cache. Should also iterate
    over the btree to preload the btree cache.

  * Should (some) constructors, open, have max_cache_size argument?

  * Verify, document, that a max_cache_size(-1) is "all".

*/

namespace boost
{
namespace btree
{

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                   Helpers for variable length keys and/or data                       //
//                                                                                      //
//--------------------------------------------------------------------------------------//

//--------------------------------- map_value ------------------------------------------//

// TODO: either add code to align mapped() or add a requirement that T2 does not
// require alignment.

template <class T1, class T2>
class map_value
{
public:
  const T1& key() const   { return *reinterpret_cast<const T1*>(this); }
  const T2& mapped_value() const
  {
    return *reinterpret_cast<const T2*>(reinterpret_cast<const char*>(this)
             + dynamic_size(key()));
  }
  std::size_t size() const
  {
    //std::cout << " dynamic_size key " << dynamic_size(key())
    //  << ", mapped_value " << dynamic_size(mapped_value()) << std::endl;
    return dynamic_size(key()) + dynamic_size(mapped_value());
  }
};

template <class T1, class T2>
std::ostream& operator<<(std::ostream& os, const map_value<T1, T2>& x)
{
  os << x.key() << ',' << x.mapped_value();
  return os;
}

template <class T1, class T2>
inline std::size_t dynamic_size(const map_value<T1, T2>& x) {return x.size();}

//--------------------------------------------------------------------------------------//
//                             general support functions                                //
//--------------------------------------------------------------------------------------//

//  less function object class

template <class T> struct less
{
  typedef T first_argument_type;
  typedef T second_argument_type;
  typedef bool result_type;
  bool operator()(const T& x, const T& y) const
  {
//    std::cout << "*** " << x << " < " << y << " is " << (x < y) << std::endl;
    return x < y;
  }
};

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

template <class Key, class Comp>
class btree_set_base
{
public:
  typedef Key   value_type;
  typedef Key   mapped_type;
  typedef Comp  value_compare;

  const Key& key(const value_type& v) const {return v;}  // really handy, so expose

  static std::size_t key_size() { return -1; }
  static std::size_t mapped_size() { return -1; }

protected:
  void m_memcpy_value(value_type* dest, const Key* k, std::size_t key_sz,
    const Key*, std::size_t)
  {
    std::memcpy(dest, k, key_sz);
  }
};

//--------------------------------------------------------------------------------------//
//                               class btree_map_base                                   //
//--------------------------------------------------------------------------------------//

template <class Key, class T, class Comp>
class btree_map_base
{
public:
  typedef map_value<Key, T>  value_type;
  typedef T                              mapped_type;

  const Key& key(const value_type& v) const  // really handy, so expose
    {return v.key();}

  static std::size_t key_size() { return -1; }
  static std::size_t mapped_size() { return -1; }

  class value_compare
  {
  public:
    value_compare() {}
    value_compare(Comp comp) : m_comp(comp) {}
    bool operator()(const value_type& x, const value_type& y) const
      { return m_comp(x.key(), y.key()); }
    bool operator()(const value_type& x, const Key& y) const
      { return m_comp(x.key(), y); }
    bool operator()(const Key& x, const value_type& y) const
      { return m_comp(x, y.key()); }
  private:
    Comp    m_comp;
  };

protected:
  void m_memcpy_value(value_type* dest, const Key* k, std::size_t key_sz,
    const T* mapped_v, std::size_t mapped_sz)
  {
    std::memcpy(dest, k, key_sz);
    std::memcpy(reinterpret_cast<char*>(dest) + key_sz, mapped_v, mapped_sz);
  }

};

namespace detail
{
  //--------------------------------- branch_value -------------------------------------//

  template <class PID, class K>
  class branch_value
  {
  public:
    PID&  node_id() {return *reinterpret_cast<PID*>(this); }
    K&  key()
    {
      return *reinterpret_cast<K*>(reinterpret_cast<char*>(this)
               + sizeof(PID));
    }
    const K&  key() const
    {
      return *reinterpret_cast<const K*>(reinterpret_cast<const char*>(this)
               + sizeof(PID));
    }
    std::size_t size() const
    { 
      return sizeof(PID) + dynamic_size(key());
    }
  };
}  // namespace detail

template <class PID, class K>
std::size_t dynamic_size(const detail::branch_value<PID, K>& v) {return v.size();}

namespace detail
{

  //-------------------------------- dynamic_iterator ----------------------------------//
  //
  //  Raw pointers to elements on a node won't work as iterators if the elements are
  //  variable length. dynamic_iterator provides the correct semantics.

  template <class T>
  class dynamic_iterator
    : public boost::iterator_facade<dynamic_iterator<T>, T, bidirectional_traversal_tag>
  {
  public:
    dynamic_iterator() : m_ptr(0), m_begin(0)
    {
      //std::cout << "dynamic_iterator ctor 1\n";
    } 
    explicit dynamic_iterator(T* ptr) : m_ptr(ptr)
    {
      //std::cout << "dynamic_iterator ctor 2\n";
      m_begin = ptr;
    }
    dynamic_iterator(T* ptr, std::size_t sz)
      : m_ptr(reinterpret_cast<T*>(reinterpret_cast<char*>(ptr) + sz))
    {
      //std::cout << "dynamic_iterator ctor 3\n";
      m_begin = ptr;
    }

    bool operator<(const dynamic_iterator& rhs) const
    {
      BOOST_ASSERT(m_begin == rhs.m_begin);  // on the same node
      return m_ptr < rhs.m_ptr;
    }

    void advance_by_size(std::size_t max_sz)
    {
      BOOST_ASSERT(max_sz);
      T* prior;
      for (std::size_t sz = 0; sz <= max_sz; sz += dynamic_size(*m_ptr)) 
      {
        prior = m_ptr;
        increment();
      }
      m_ptr = prior;
    }

  private:
    friend class boost::iterator_core_access;

    T* m_ptr;    // the pointer
    T* m_begin;  // the begin pointer for decrements

    T& dereference() const  { return *m_ptr; }
 
    bool equal(const dynamic_iterator& rhs) const
    { 
      return m_ptr == rhs.m_ptr;
    }

    void increment()
    {
      //std::cout << "***inc dynamic_size " << dynamic_size(*m_ptr) << std::endl;
      m_ptr = reinterpret_cast<T*>(reinterpret_cast<char*>(m_ptr)
        + dynamic_size(*m_ptr));
    }

    void decrement()
    {
      BOOST_ASSERT_MSG(m_ptr != m_begin, "internal logic error; decrement of begin");
      T* prior = m_begin;
      T* cur = m_begin;
      for(;;)
      {
        cur = reinterpret_cast<T*>(reinterpret_cast<char*>(cur) + dynamic_size(*cur));
        if (cur == m_ptr) break;
        prior = cur;
      }
      m_ptr = prior;
    }

    void advance(std::ptrdiff_t n)
    {
      if (n > 0)
        for (; n; --n)
          increment();
      else if (n < 0)
        for (; n; ++n)
          decrement();
    }

    std::ptrdiff_t distance_to(const dynamic_iterator& rhs) const
    {
      BOOST_ASSERT(false);   // is this function ever used?
      return 0;
    }

  };

  //-------------------------------- pointer_iterator ----------------------------------//
  //
  //  Raw pointer wrapper providing the same interface as dynamic_pointer, but as
  //  a random access rather than bidirctional iterator. It is used when T is not a
  //  dynamic-size type.

  template <class T>
  class pointer_iterator
    : public boost::iterator_facade<pointer_iterator<T>, T, random_access_traversal_tag>
  {
  public:
    pointer_iterator() : m_ptr(0) {} 
    explicit pointer_iterator(T* ptr) : m_ptr(ptr) {}
    pointer_iterator(T* ptr, std::size_t sz)
      : m_ptr(reinterpret_cast<T*>(reinterpret_cast<char*>(ptr) + sz)) {}

    bool operator<(const pointer_iterator& rhs) const  {return m_ptr < rhs.m_ptr;}

    void advance_by_size(std::size_t max_sz)
    {
      BOOST_ASSERT(max_sz);
      std::ptrdiff_t n = max_sz / btree::dynamic_size(*m_ptr);
      m_ptr = reinterpret_cast<T*>(reinterpret_cast<char*>(m_ptr)
        + n * static_cast<std::ptrdiff_t>(btree::dynamic_size(*m_ptr)));
    }

  private:
    friend class boost::iterator_core_access;

    T* m_ptr;    // the pointer

    T& dereference() const  { return *m_ptr; }
 
    bool equal(const pointer_iterator& rhs) const {return m_ptr == rhs.m_ptr;}

    void increment()
    {
      //std::cout << "***inc  " <<  btree::dynamic_size(*m_ptr) << '\n';
      m_ptr = reinterpret_cast<T*>(reinterpret_cast<char*>(m_ptr)
        + btree::dynamic_size(*m_ptr));
    }
    void decrement()
    {
      m_ptr = reinterpret_cast<T*>(reinterpret_cast<char*>(m_ptr)
        - btree::dynamic_size(*m_ptr));
    }
    void advance(std::ptrdiff_t n)
    {
      m_ptr = reinterpret_cast<T*>(reinterpret_cast<char*>(m_ptr)
        + n * static_cast<std::ptrdiff_t>(btree::dynamic_size(*m_ptr)));
    }

    std::ptrdiff_t distance_to(const pointer_iterator& rhs) const
    {
      return std::distance(reinterpret_cast<const char*>(m_ptr),
        reinterpret_cast<const char*>(rhs.m_ptr))
        / static_cast<std::ptrdiff_t>(btree::dynamic_size(*m_ptr));
    }

  };

}  // namespace detail

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                                 class btree_base                                    //
//                                                                                      //
//                  a B+ tree with leaf-sequence doubly-linked list                     //
//                                                                                      //
//--------------------------------------------------------------------------------------//

template  <class Key,
           class Base,  // btree_map_base or btree_set_base
           class Traits,
           class Comp>

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
  typedef Comp                                  key_compare;
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

  typedef typename Traits::node_id_type         node_id_type;
  typedef typename Traits::node_size_type       node_size_type;
  typedef typename Traits::node_level_type      node_level_type;

  // TODO: why are these being exposed:
  typedef value_type                            leaf_value_type;
  typedef typename boost::mpl::or_<
    typename boost::btree::has_dynamic_size<key_type>::type,
    typename boost::btree::has_dynamic_size<mapped_type>::type
             >::type                            leaf_iterator_selector;
  typedef typename boost::mpl::if_<leaf_iterator_selector,
             detail::dynamic_iterator<leaf_value_type>,
             detail::pointer_iterator<leaf_value_type>  
             >::type                            leaf_iterator;

  // construct/destroy:

  // TODO: why are these being exposed:
  btree_base(const Comp& comp);
  btree_base(const boost::filesystem::path& p, flags::bitmask flgs, std::size_t node_sz,
             const Comp& comp);
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

  const buffer_manager&
                     manager() const        { return m_mgr; }
  bool               is_open() const        { return m_mgr.is_open(); }
  const filesystem::path&
                     file_path() const      { return m_mgr.file_path(); }
  bool               read_only() const      { return m_read_only; }
  const header_page& header() const         {BOOST_ASSERT(is_open());
                                              return m_hdr;
                                            }
  void   dump_dot(std::ostream& os) const; // dump tree using Graphviz dot format

  // capacity:

  bool          empty() const               { return !size(); }
  size_type     size() const                { return m_hdr.element_count(); }
  //size_type     max_size() const            { return ; }
  std::size_t   node_size() const           { return m_mgr.data_size(); }
  std::size_t   max_cache_size() const      { return m_mgr.max_cache_size(); }
  void          max_cache_size(std::size_t m) {m_mgr.max_cache_size(m);}

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

  const_iterator     erase(const_iterator position);
  size_type          erase(const key_type& k);
  const_iterator     erase(const_iterator first, const_iterator last);
  void               clear();

  // observers:

  key_compare        key_comp() const       { return m_comp; }
  value_compare      value_comp() const     { return m_value_comp; }

  // operations:

  const_iterator     find(const key_type& k) const;
  size_type          count(const key_type& k) const;

  const_iterator     lower_bound(const key_type& k) const;
  const_iterator     upper_bound(const key_type& k) const;

  const_iterator_range  equal_range(const key_type& k) const
                            { return std::make_pair(lower_bound(k), upper_bound(k)); }

//--------------------------------------------------------------------------------------//
//                                private data members                                  //
//--------------------------------------------------------------------------------------//

private:

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

  btree::header_page m_hdr;

  std::size_t        m_max_leaf_size;
  std::size_t        m_max_branch_size;

  bool               m_read_only;
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
    bool             is_branch() const     {return m_level > 0 && m_level < 0xFFFEU;}
    std::size_t      size() const          {return m_size;}  // std::size_t is correct!
    void             size(std::size_t sz)  {m_size = sz;}    // ditto

//  private:
    node_level_type  m_level;        // leaf: 0, branches: distance from leaf,
                                     // header: 0xFFFF, free node list entry: 0xFFFE
    node_size_type   m_size;         // size in bytes of elements on node
  };
  
  //------------------------ leaf data formats and operations --------------------------//

 class leaf_data : public btree_data
  {
    friend class btree_base;
  public:
    leaf_iterator  begin()      { return leaf_iterator(m_value);}
    leaf_iterator  end()        { return leaf_iterator(m_value, size()); }

    //  offsetof() macro won't work for all value types, so compute by hand
    static std::size_t value_offset()
    {
      static leaf_data dummy;
      static std::size_t off
        = reinterpret_cast<char*>(&dummy.m_value) - reinterpret_cast<char*>(&dummy);
      return off;
    }

    //  private:
    leaf_value_type  m_value[1];
  };

  std::size_t char_distance(const void* from, const void* to)
  {
    return reinterpret_cast<const char*>(to) - reinterpret_cast<const char*>(from);
  }

  char* char_ptr(void* from)
  {
    return reinterpret_cast<char*>(from);
  }

  //------------------------ branch data formats and operations ------------------------//

      //----------------------------- branch invariants ----------------------------//
      //                                                                            //
      //  Unique containers:      Pn < Kn <= Pn+1   Keys in Pn are < Kn             //
      //                                            Kn <= Keys in Pn+1              //
      //                                                                            //
      //  Non-unique containers:  Pn <= Kn <= Pn+1  Keys in Pn are <= Kn            //
      //                                            Kn <= Keys in Pn+1              //
      //----------------------------------------------------------------------------//

  typedef detail::branch_value<node_id_type, key_type>  branch_value_type;
  typedef typename boost::mpl::if_<
    typename boost::btree::has_dynamic_size<key_type>::type,
             detail::dynamic_iterator<branch_value_type>,
             detail::pointer_iterator<branch_value_type>  
             >::type                                    branch_iterator;

  class branch_data : public btree_data
  {
  public:
    branch_iterator  begin()    { return branch_iterator(m_value);}
    branch_iterator  end()      { return branch_iterator(m_value, size()); }

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
  public:
    btree_node() : buffer() {}
    btree_node(buffer::buffer_id_type id, buffer_manager& mgr)
      : buffer(id, mgr) {}

    node_id_type       node_id() const                 {return node_id_type(buffer_id());}

    btree_node*        parent()                          {return m_parent.get();}
    void               parent(btree_node_ptr p)          {m_parent = p;}
    branch_iterator    parent_element()                  {return m_parent_element;}
    void               parent_element(branch_iterator p) {m_parent_element = p;}
#   ifndef NDEBUG
    node_id_type       parent_node_id()                  {return m_parent_node_id;}
    void               parent_node_id(node_id_type id)   {m_parent_node_id = id;}
#   endif

    leaf_data&         leaf()       {return *reinterpret_cast<leaf_data*>(buffer::data());}
    const leaf_data&   leaf() const {return *reinterpret_cast<const leaf_data*>(buffer::data());}
    branch_data&       branch()     {return *reinterpret_cast<branch_data*>(buffer::data());}
    unsigned           level() const         {return leaf().m_level;}
    void               level(unsigned lv)    {leaf().m_level = lv;}
    bool               is_leaf() const       {return leaf().is_leaf();}
    bool               is_branch() const     {return leaf().is_branch();}
    std::size_t        size() const          {return leaf().m_size;}  // std::size_t is correct!
    void               size(std::size_t sz)  {leaf().m_size = sz;}    // ditto
    bool               empty() const         {return leaf().m_size == 0;}

    btree_node_ptr     next_node()  // return next node at current level
    {
      if (!parent())              // if this is the root, there is no next node
        return btree_node_ptr();

      btree_node_ptr   par(m_parent);
      branch_iterator  par_element(m_parent_element);

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

      btree_node_ptr np(manager()->read(par_element->node_id()));
      np->parent(par);
      np->parent_element(par_element);
#     ifndef NDEBUG
      np->parent_node_id(par->node_id());
#     endif
      return np;
    }

    btree_node_ptr     prior_node()  // return next node at current level
    {
      if (!parent())              // if this is the root, there is no next node
        return btree_node_ptr();

      btree_node_ptr   par(m_parent);
      branch_iterator  par_element(m_parent_element);

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

      btree_node_ptr np(manager()->read(par_element->node_id()));
      np->parent(par);
      np->parent_element(par_element);
#     ifndef NDEBUG
      np->parent_node_id(par->node_id());
#     endif
      return np;
    }

  private:
    btree_node_ptr     m_parent;          // by definition, the parent is a branch node.
    branch_iterator    m_parent_element;
# ifndef NDEBUG
    node_id_type       m_parent_node_id;  // allows assert that m_parent has not been
                                          // overwritten by faultylogic
# endif
  };

  //-------------------------------  btree_node_ptr  -----------------------------------//

  class btree_node_ptr : public buffer_ptr
  {
  public:

    btree_node_ptr() : buffer_ptr() {}
    btree_node_ptr(btree_node& p) : buffer_ptr(p) {}
    btree_node_ptr(buffer& p) : buffer_ptr(p) {}
    btree_node_ptr(const btree_node_ptr& r) : buffer_ptr(r) {} 
    btree_node_ptr(const buffer_ptr& r) : buffer_ptr(r) {}
    btree_node_ptr& operator=(const btree_node_ptr& r)
    {
      btree_node_ptr(r).swap(*this);  // correct for self-assignment
      return *this;
    }
    btree_node_ptr& operator=(const buffer_ptr& r)
    {
      btree_node_ptr(r).swap(*this);  // correct for self-assignment
      return *this;
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

  public:
    iterator_type(): m_element(0) {}
    iterator_type(buffer_ptr p, leaf_iterator e)
      : m_node(static_cast<typename btree_base::btree_node_ptr>(p)),
        m_element(e) {}

  private:
    iterator_type(buffer_ptr p)  // used solely to setup the end iterator
      : m_node(static_cast<typename btree_base::btree_node_ptr>(p)),
        m_element(0) {}

    friend class boost::iterator_core_access;
    friend class btree_base;
   
    typename btree_base::btree_node_ptr  m_node; 
    typename btree_base::leaf_iterator   m_element;  // 0 for end iterator

    T& dereference() const  { return *m_element; }
 
    bool equal(const iterator_type& rhs) const
    {
      return m_element == rhs.m_element
        // check node_id() in case node memory has been reused
        && m_node->node_id() == rhs.m_node->node_id();
    }

    void increment();
    void decrement();
  };

//--------------------------------------------------------------------------------------//
//                             protected member functions                               //
//--------------------------------------------------------------------------------------//
protected:

  std::pair<const_iterator, bool>
    m_insert_unique(const key_type& k, const mapped_type& mv);

  const_iterator
    m_insert_non_unique(const key_type& k, const mapped_type& mv);
  // Remark: Insert after any elements with equivalent keys, per C++ standard

  iterator m_update(iterator itr, const mapped_type& mv);

  void m_open(const boost::filesystem::path& p, flags::bitmask flgs, std::size_t node_sz);

//--------------------------------------------------------------------------------------//
//                              private member functions                                //
//--------------------------------------------------------------------------------------//
private:

  static buffer* m_node_alloc(buffer::buffer_id_type np_id, buffer_manager& mgr)
  { return new btree_node(np_id, mgr); }

  void m_read_header()
  {
    m_mgr.seek(0);
    m_mgr.binary_file::read(m_hdr, sizeof(btree::header_page));
    m_hdr.endian_flip_if_needed();
  }

  void m_write_header()
  {
    m_mgr.seek(0);
    m_hdr.endian_flip_if_needed();
    m_mgr.binary_file::write(&m_hdr, sizeof(btree::header_page));
    m_hdr.endian_flip_if_needed();
  }

  iterator m_special_lower_bound(const key_type& k) const;
  // returned iterator::m_element is the insertion point, and thus may be the 
  // past-the-end leaf_iterator for iterator::m_node
  // postcondition: parent pointers are set, all the way up the chain to the root

  iterator m_special_upper_bound(const key_type& k) const;
  // returned iterator::m_element is the insertion point, and thus may be the 
  // past-the-end leaf_iterator for iterator::m_node
  // postcondition: parent pointers are set, all the way up the chain to the root

  btree_node_ptr m_new_node(boost::uint16_t lv);
  void  m_new_root();
  const_iterator m_leaf_insert(iterator insert_iter, const key_type& key,
    const mapped_type& mapped_value);
  void  m_branch_insert(btree_node* np, branch_iterator element,
    const key_type& k, node_id_type id);

iterator m_sub_tree_begin(node_id_type id);
iterator m_erase_branch_value(btree_node* np, branch_iterator value, node_id_type erasee);
  void  m_free_node(btree_node* np)
  {
    np->needs_write(true);
    np->level(0xFFFE);
    np->size(0);
    np->branch().begin()->node_id() = node_id_type(m_hdr.free_node_list_head_id());
    m_hdr.free_node_list_head_id(np->node_id());
  }

  //-------------------------------- branch_compare ------------------------------------//

  class branch_compare
//    : public std::binary_function<branch_value_type, key_type, bool>
  {
    friend class btree_base;
  protected:
    Comp    m_comp;
    branch_compare() {}
    branch_compare(Comp comp) : m_comp(comp) {}
  public:
   bool operator()(const branch_value_type& x, const branch_value_type& y) const
      {return m_comp(x.key(), y.key());}
   bool operator()(const Key& x, const branch_value_type& y) const
      {return m_comp(x, y.key());}
   bool operator()(const branch_value_type& x, const Key& y) const
      {return m_comp(x.key(), y);}
  };

  //------------------------ comparison function objects -------------------------------//

  //  The standard library mandates key_compare and value_compare types.
  //  The implementation also needs to compare a value_type to a key_type, and a
  //  branch_value_type to a key_type and visa versa.

  key_compare               m_comp;
  value_compare             m_value_comp;
  branch_compare            m_branch_comp;

  branch_compare
    branch_comp() const { return m_branch_comp; }

};  // class btree_base


//--------------------------------------------------------------------------------------//
//                              non-member operator <<                                  //
//--------------------------------------------------------------------------------------//

template <class Key, class Base, class Traits, class Comp>   
std::ostream& operator<<(std::ostream& os,
  const btree_base<Key,Base,Traits,Comp>& bt)
{
  os << "B+ tree \"" << bt.file_path().string() << "\"\n"
     << bt.header().element_count() << " records\n"  
     << bt.header().node_size() << " node size\n"  
     << bt.header().node_count() << " node count\n"  
     << bt.header().root_node_id() << " root node id\n"  
     << bt.header().root_level()+1 << " levels in tree\n"
     << "User supplied string: \"" << bt.header().user_c_str() << "\"\n"
  ;
  return os;
}

//--------------------------------------------------------------------------------------//
//                          class btree_base implementation                             //
//--------------------------------------------------------------------------------------//

//------------------------------ construct without open --------------------------------//

template <class Key, class Base, class Traits, class Comp>
btree_base<Key,Base,Traits,Comp>::btree_base(const Comp& comp)
  : m_mgr(m_node_alloc), m_comp(comp), m_value_comp(comp), m_branch_comp(comp)
{ 
  m_mgr.owner(this);

  // set up the end iterator
  m_end_node.manager(&m_mgr);
  m_end_iterator = const_iterator(buffer_ptr(m_end_node));
}

//------------------------------- construct with open ----------------------------------//

template <class Key, class Base, class Traits, class Comp>
btree_base<Key,Base,Traits,Comp>::btree_base(const boost::filesystem::path& p,
  flags::bitmask flgs, std::size_t node_sz, const Comp& comp)
  : m_mgr(m_node_alloc), m_comp(comp), m_value_comp(comp), m_branch_comp(comp)
{ 
  m_mgr.owner(this);

  // set up the end iterator
  m_end_node.manager(&m_mgr);
  m_end_iterator = const_iterator(buffer_ptr(m_end_node));

  // open the file and set up data members
  m_open(p, flgs, node_sz);
}

//----------------------------------- destructor ---------------------------------------//

template <class Key, class Base, class Traits, class Comp>
btree_base<Key,Base,Traits,Comp>::~btree_base()
{
  try { close(); }
  catch (...) {}
}

//------------------------------------- close ------------------------------------------//

template <class Key, class Base, class Traits, class Comp>
void btree_base<Key,Base,Traits,Comp>::close()
{
  if (is_open())
  {
    flush();
    m_mgr.close();
  }
}

//-------------------------------------- open ------------------------------------------//

template <class Key, class Base, class Traits, class Comp>
void
btree_base<Key,Base,Traits,Comp>::m_open(const boost::filesystem::path& p,
  flags::bitmask flgs, std::size_t node_sz) 
{
  BOOST_ASSERT(!is_open());
  BOOST_ASSERT(node_sz >= sizeof(btree::header_page));

  oflag::bitmask open_flags = oflag::in;
  if (flgs & flags::read_write)
    open_flags |= oflag::out;
  if (flgs & flags::truncate)
    open_flags |= oflag::out | oflag::truncate;
  if (flgs & flags::preload)
    open_flags |= oflag::preload;

  m_read_only = (open_flags & oflag::out) == 0;
  m_ok_to_pack = true;
  m_max_leaf_size = node_sz - leaf_data::value_offset();
  m_max_branch_size = node_sz - branch_data::value_offset();

  if (m_mgr.open(p, open_flags, btree::default_max_cache_nodes, node_sz))
  { // existing non-truncated file
    m_read_header();
    if (!m_hdr.marker_ok())
      BOOST_BTREE_THROW(std::runtime_error(file_path().string()+" isn't a btree"));
    if (m_hdr.big_endian() != (Traits::header_endianness == endian::order::big))
      BOOST_BTREE_THROW(std::runtime_error(file_path().string()+" has wrong endianness"));
    m_mgr.data_size(m_hdr.node_size());
    m_root = m_mgr.read(m_hdr.root_node_id());
  }
  else
  { // new or truncated file
    m_hdr.clear();
    m_hdr.big_endian(Traits::header_endianness == endian::order::big);
    m_hdr.flags(flgs & ~(btree::flags::read_write | btree::flags::truncate));
    m_hdr.splash_c_str("boost.org btree");
    m_hdr.user_c_str("");
    m_hdr.node_size(node_sz);
    m_hdr.key_size(Base::key_size());
    m_hdr.mapped_size(Base::mapped_size());
    m_hdr.increment_node_count();  // i.e. the header itself
    m_mgr.new_buffer();  // force a buffer write, thus zeroing the header for its full size
    flush();  // writes buffer and header

    // set up an empty leaf as the initial root
    m_root = m_mgr.new_buffer();
    m_root->needs_write(true);
    m_hdr.increment_node_count();
    BOOST_ASSERT(m_root->node_id() == 1);
    m_hdr.root_node_id(m_root->node_id());
    m_hdr.first_node_id(m_root->node_id());
    m_hdr.last_node_id(m_root->node_id());
    m_root->level(0);
    m_root->size(0);
  }
//  m_set_max_cache_nodes();
}

//------------------------------------- clear() ----------------------------------------//

template <class Key, class Base, class Traits, class Comp>
void
btree_base<Key,Base,Traits,Comp>::clear()
{
  BOOST_ASSERT_MSG(is_open(), "can't clear() unopen btree");

  manager().clear_write_needed();
  m_hdr.element_count(0);
  m_hdr.root_node_id(1);
  m_hdr.first_node_id(1);
  m_hdr.last_node_id(1);
  m_hdr.root_level(0);
  m_hdr.node_count(0);
  m_hdr.free_node_list_head_id(0);

  m_mgr.close();

}

//------------------------------------- begin() ----------------------------------------//

template <class Key, class Base, class Traits, class Comp>
typename btree_base<Key,Base,Traits,Comp>::const_iterator
btree_base<Key,Base,Traits,Comp>::begin() const
{
  BOOST_ASSERT_MSG(is_open(), "begin() on unopen btree");
  if (empty())
    return end();
  btree_node_ptr np = m_root;

  // work down the tree until a leaf is reached
  while (np->is_branch())
  {
    // create the child->parent list
    btree_node_ptr child_np = m_mgr.read(np->branch().begin()->node_id());
    child_np->parent(np);
    child_np->parent_element(np->branch().begin());
#   ifndef NDEBUG
    child_np->parent_node_id(np->node_id());
#   endif

    np = child_np;
  }

  return const_iterator(np, np->leaf().begin());
}

//-------------------------------------- last() ---------------------------------------//

template <class Key, class Base, class Traits, class Comp>
typename btree_base<Key,Base,Traits,Comp>::const_iterator
btree_base<Key,Base,Traits,Comp>::last() const
{
  BOOST_ASSERT_MSG(is_open(), "last() on unopen btree");
  if (empty())
    return end();
  BOOST_ASSERT(header().last_node_id());
  btree_node_ptr np(m_mgr.read(header().last_node_id()));
  BOOST_ASSERT(np->is_leaf());
  return const_iterator(np, np->leaf().end()-1);
}

//---------------------------------- m_new_node() --------------------------------------//

template <class Key, class Base, class Traits, class Comp>   
typename btree_base<Key,Base,Traits,Comp>::btree_node_ptr 
btree_base<Key,Base,Traits,Comp>::m_new_node(boost::uint16_t lv)
{
  btree_node_ptr np;
  if (m_hdr.free_node_list_head_id())
  {
    np = m_mgr.read(m_hdr.free_node_list_head_id());
    BOOST_ASSERT(np->level() == 0xFFFE);  // free node list entry
    m_hdr.free_node_list_head_id(np->branch().begin()->node_id());
  }
  else
  {
    np = m_mgr.new_buffer();
    m_hdr.increment_node_count();
    BOOST_ASSERT(m_hdr.node_count() == m_mgr.buffer_count());
  }

  np->needs_write(true);
  np->level(lv);
  np->size(0);
  return np;
}

//----------------------------------- m_new_root() -------------------------------------//

template <class Key, class Base, class Traits, class Comp>   
void
btree_base<Key,Base,Traits,Comp>::m_new_root()
{ 
  // create a new root containing only the P0 pseudo-element
  btree_node_ptr old_root = m_root;
  node_id_type old_root_id(m_root->node_id());
  m_hdr.increment_root_level();
//  m_set_max_cache_nodes();
  m_root = m_new_node(m_hdr.root_level());
  m_hdr.root_node_id(m_root->node_id());
  m_root->branch().begin()->node_id() = old_root_id;
  m_root->size(0);  // the end pseudo-element doesn't count as an element
  m_root->parent(btree_node_ptr());  
  m_root->parent_element(branch_iterator());
  old_root->parent(m_root);
  old_root->parent_element(m_root->branch().begin());
# ifndef NDEBUG
  m_root->parent_node_id(node_id_type(0));
  old_root->parent_node_id(m_root->node_id());
# endif
}

//---------------------------------- m_leaf_insert() -----------------------------------//

template <class Key, class Base, class Traits, class Comp>   
typename btree_base<Key,Base,Traits,Comp>::const_iterator
btree_base<Key,Base,Traits,Comp>::m_leaf_insert(iterator insert_iter,
  const key_type& key_, const mapped_type& mapped_value_)
{
  std::size_t          key_size= dynamic_size(key_);
  std::size_t          mapped_size = (header().flags() & btree::flags::key_only)
    ? 0
    : dynamic_size(mapped_value_);
  std::size_t          value_size = key_size + mapped_size;
  //std::cout << "***insert key_size " << key_size << " value_size " << value_size << std::endl;
  btree_node_ptr       np = insert_iter.m_node;
  leaf_iterator        insert_begin = insert_iter.m_element;
  btree_node_ptr       np2;
  
  BOOST_ASSERT_MSG(np->is_leaf(), "internal error");
  BOOST_ASSERT_MSG(np->size() <= m_max_leaf_size, "internal error");

  m_hdr.increment_element_count();
  np->needs_write(true);

  if (np->size() + value_size > m_max_leaf_size)  // no room on node?
  {
    //  no room on node, so node must be split

    if (np->level() == m_hdr.root_level()) // splitting the root?
      m_new_root();  // create a new root
    
    np2 = m_new_node(np->level());  // create the new node 

    // ck pack conditions now, since leaf seq list update may chg header().last_node_id()
    if (m_ok_to_pack
        && (insert_begin != np->leaf().end() || np->node_id() != header().last_node_id()))
      m_ok_to_pack = false;  // conditions for pack optimization not met

    if (np->node_id() == header().last_node_id())
      m_hdr.last_node_id(np2->node_id());

    // apply pack optimization if applicable
    if (m_ok_to_pack)  // have all inserts been ordered and no erases occurred?
    {
      // pack optimization: instead of splitting np, just put value alone on np2
      this->m_memcpy_value(&*np2->leaf().begin(), &key_, key_size, &mapped_value_, mapped_size);  // insert value
      np2->size(value_size);
      BOOST_ASSERT(np->parent()->node_id() == np->parent_node_id()); // max_cache_size logic OK?
      m_branch_insert(np->parent(), np->parent_element(),
        this->key(*np2->leaf().begin()), np2->node_id());
      return const_iterator(np2, np2->leaf().begin());
    }

    // split node np by moving half the elements, by size, to node p2
    leaf_iterator split_begin(np->leaf().begin());
    split_begin.advance_by_size(np->leaf().size() / 2);
    ++split_begin; // for leaves, prefer more aggressive split begin
    std::size_t split_sz = char_distance(&*split_begin, &*np->leaf().end());

    // TODO: if the insert point will fall on the new node, it would be faster to
    // copy the portion before the insert point, copy the value being inserted, and
    // finally copy the portion after the insert point. However, that's a fair amount of
    // additional code for something that only happens on half of all leaf splits on average.

    std::memcpy(&*np2->leaf().begin(), &*split_begin, split_sz);
    np2->size(split_sz);
    std::memset(&*split_begin, 0,                         // zero unused space to make
      char_distance(&*split_begin, &*np->leaf().end()));  //  file dumps easier to read
    np->size(np->size() - split_sz);

    // adjust np and insert_begin if they now fall on the new node due to the split
    if (&*split_begin < &*insert_begin)
    {
      np = np2;
      insert_begin = leaf_iterator(&*np->leaf().begin(),
        char_distance(&*split_begin, &*insert_begin));  
    }
  }

  //  insert value into np at insert_begin
//std::cout << "np size " << np->size()
//          << " insert_begin " << &*insert_begin
//          << " begin() " << &*np->leaf().begin()
//          << " end() " << &*np->leaf().end()
//          << " char_distance " << char_distance(&*insert_begin, &*np->leaf().end())
//          << std::endl;
  BOOST_ASSERT(&*insert_begin >= &*np->leaf().begin());
  BOOST_ASSERT(&*insert_begin <= &*np->leaf().end());
  std::memmove(char_ptr(&*insert_begin) + value_size,
    &*insert_begin, char_distance(&*insert_begin, &*np->leaf().end()));  // make room
  this->m_memcpy_value(&*insert_begin, &key_, key_size, &mapped_value_, mapped_size);  // insert value
  np->size(np->size() + value_size);
//std::cout << "np size " << np->size() << std::endl;

  // if there is a new node, its initial key and node_id are inserted into parent
  if (np2)
  {
    BOOST_ASSERT(insert_iter.m_node->parent()->node_id() \
      == insert_iter.m_node->parent_node_id()); // max_cache_size logic OK?
    m_branch_insert(insert_iter.m_node->parent(),
      insert_iter.m_node->parent_element(),
      this->key(*np2->leaf().begin()), np2->node_id());
  }

//std::cout << "***insert done" << std::endl;
  return const_iterator(np, insert_begin);
}

//---------------------------------- m_branch_insert() ---------------------------------//

//  This logic is very similar to the m_leaf_insert() logic, so it might seem they should
//  combined into a single function. This was done in the 25 year-old C language function
//  that the current function is based on, and it was very messy, hard to understand, and
//  bug prone. The initial C++ attempt tried the a single function approach, and seemed
//  subject to those same ills. The two function approach has been much less troublesome
//  right from the start.

template <class Key, class Base, class Traits, class Comp>   
void
btree_base<Key,Base,Traits,Comp>::m_branch_insert(
  btree_node* np1, branch_iterator element, const key_type& k, node_id_type id) 
{
  //std::cout << "branch insert key " << k << ", id " << id << std::endl;

  std::size_t       k_size = dynamic_size(k);
  std::size_t       insert_size = k_size + sizeof(node_id_type);
  btree_node*       np = np1;
  key_type*         insert_begin = &element->key();
  btree_node_ptr    np2;

  BOOST_ASSERT(np->is_branch());
  BOOST_ASSERT(np->size() <= m_max_branch_size);

  np->needs_write(true);

  if (np->size() + insert_size
                 + sizeof(node_id_type)  // NOTE WELL: size() doesn't include
                                         // size of the end pseudo-element node_id
    > m_max_branch_size)  // no room on node?
  {
    //  no room on node, so node must be split
    //std::cout << "Splitting branch\n";

    if (np->level() == m_hdr.root_level()) // splitting the root?
      m_new_root();  // create a new root
    
    np2 = m_new_node(np->level());  // create the new node

    // split node np by moving half the elements, by size, to node p2

    branch_iterator unsplit_end(np->branch().begin());
    unsplit_end.advance_by_size(np->branch().size() / 2);
    branch_iterator split_begin(unsplit_end+1);
    std::size_t split_sz = char_distance(&*split_begin, char_ptr(&*np->branch().end()) 
      + sizeof(node_id_type));  // include the end pseudo-element node_id
    BOOST_ASSERT(split_sz > sizeof(node_id_type));

    // TODO: if the insert point will fall on the new node, it would be faster to
    // copy the portion before the insert point, copy the value being inserted, and
    // finally copy the portion after the insert point. However, that's a fair amount of
    // additional code for something that only happens on half of all branch splits on average.

    //// if the insert will be at the start just copy everything to the proper location
    //if (split_begin == insert_begin)
    //{
    //  np2->branch().P0 = id;
    //  std::memcpy(&*np2->branch().begin(), &*split_begin, split_sz); // move split values to new node
    //  np2->size(split_sz);
    //  std::memset(&*split_begin, 0,  // zero unused space to make file dumps easier to read
    //    (np->branch().end() - split_begin) * sizeof(branch_value_type)); 
    //  np->size(np->size() - split_sz);
    //  BOOST_ASSERT(np->parent()->node_id() == np->parent_node_id()); // max_cache_size logic OK?
    //  m_branch_insert(np->parent(), np->parent_element()+1, k, np2->node_id());
    //  return;
    //}

    // copy the split elements, including the pseudo-end element, to p2
    BOOST_ASSERT(char_ptr(&*np2->branch().begin())+split_sz
      <= char_ptr(&*np2->branch().begin())+m_max_branch_size);
    std::memcpy(&*np2->branch().begin(), &*split_begin, split_sz);  // include end pseudo element
    np2->size(split_sz - sizeof(node_id_type));  // exclude end pseudo element from size

    BOOST_ASSERT(np->parent()->node_id() == np->parent_node_id()); // max_cache_size logic OK?

    // promote the key from the original node's new end pseudo element to the parent branch node
    m_branch_insert(np->parent(), np->parent_element(), unsplit_end->key(), np2->node_id());

    // finalize work on the original node
    std::memset(&unsplit_end->key(), 0,  // zero unused space to make file dumps easier to read
      char_distance(&unsplit_end->key(), &np->branch().end()->key())); 
    np->size(char_distance(&*np->branch().begin(), &*unsplit_end));

    // adjust np and insert_begin if they now fall on the new node due to the split
    if (&*split_begin <= &*element)
    {
      np = np2.get();
      insert_begin = reinterpret_cast<key_type*>(char_ptr(&np2->branch().begin()->key())
        + char_distance(&split_begin->key(), insert_begin));
    }
  }

  //  insert k, id, into np at insert_begin

//std::cout << "node size " << np->size()
//          << " insert size " << insert_size
//          << " k_size " << k_size
//          << " insert_begin " << insert_begin
//          << " begin() " << &*np->branch().begin()
//          << " char_distance " << char_distance(insert_begin, &np->branch().end()->key())
//          << std::endl;
  BOOST_ASSERT(insert_begin >= &np->branch().begin()->key());
  BOOST_ASSERT(insert_begin <= &np->branch().end()->key());
  BOOST_ASSERT(char_ptr(insert_begin) + insert_size            // start of memmove
    + char_distance(insert_begin, &np->branch().end()->key())  // + size of memmove
    <= char_ptr(&*np->branch().begin()) + m_max_branch_size);
  std::memmove(char_ptr(insert_begin) + insert_size,
    insert_begin, char_distance(insert_begin, &np->branch().end()->key()));  // make room
  BOOST_ASSERT(char_ptr(insert_begin) + k_size + sizeof(node_id_type)
    <= char_ptr(&*np->branch().begin())+m_max_branch_size);
  std::memcpy(insert_begin, &k, k_size);  // insert k
  std::memcpy(char_ptr(insert_begin) + k_size, &id, sizeof(node_id_type));
  np->size(np->size() + insert_size);

#ifndef NDEBUG
  if (m_hdr.flags() & btree::flags::unique)
  {
    branch_iterator cur = np->branch().begin();
    const key_type* prev_key = &cur->key();
    ++cur;
    for(; cur != np->branch().end(); ++cur)
    {
      BOOST_ASSERT(key_comp()(*prev_key, cur->key()));
      prev_key = &cur->key();
    }
  }
#endif
}

//------------------------------------- erase() ----------------------------------------//

template <class Key, class Base, class Traits, class Comp>   
typename btree_base<Key,Base,Traits,Comp>::const_iterator
btree_base<Key,Base,Traits,Comp>::erase(const_iterator pos)
{
  BOOST_ASSERT_MSG(is_open(), "erase() on unopen btree");
  BOOST_ASSERT_MSG(!read_only(), "erase() on read only btree");
  BOOST_ASSERT_MSG(pos != end(), "erase() on end iterator");
  BOOST_ASSERT(pos.m_node);
  BOOST_ASSERT(pos.m_node->is_leaf());
  BOOST_ASSERT(&*pos.m_element < &*pos.m_node->leaf().end());
  BOOST_ASSERT(&*pos.m_element >= &*pos.m_node->leaf().begin());

  m_ok_to_pack = false;  // TODO: is this too conservative?
  pos.m_node->needs_write(true);
  m_hdr.decrement_element_count();

  //key_type nxt_key;  // save next key to be able to find() iterator to be returned
  //const_iterator nxt(pos);
  //++nxt;
  //if (nxt != end())
  //  nxt_key = key(*nxt);

  if (pos.m_node->node_id() != m_root->node_id()  // not root?
    && (pos.m_node->size() == dynamic_size(*pos.m_node->leaf().begin())))  // only 1 element on node?
  {
    // erase a single value leaf node that is not the root

    BOOST_ASSERT(pos.m_node->parent()->node_id() \
      == pos.m_node->parent_node_id()); // max_cache_size logic OK?

    if (pos.m_node->node_id() == header().first_node_id())
    {
      btree_node_ptr nxt_node(pos.m_node->next_node());
      BOOST_ASSERT(nxt_node);
      m_hdr.first_node_id(nxt_node->node_id());
    }
    if (pos.m_node->node_id() == header().last_node_id())
    {
      btree_node_ptr prr_node(pos.m_node->prior_node());
      BOOST_ASSERT(prr_node);
      m_hdr.last_node_id(prr_node->node_id());
    }

    const_iterator nxt = m_erase_branch_value(pos.m_node->parent(),
      pos.m_node->parent_element(), pos.m_node->node_id());

    m_free_node(pos.m_node.get());  // add node to free node list
    return nxt;
  }
  else
  {
    // erase an element from a leaf with multiple elements or erase the only element
    // on a leaf that is also the root; these use the same logic because they do not remove
    // the node from the tree.

    value_type* erase_point = &*pos.m_element;
    std::size_t erase_sz = dynamic_size(*erase_point);
    std::size_t move_sz = char_ptr(&*pos.m_node->leaf().end())
      - (char_ptr(erase_point) + erase_sz); 
    std::memmove(erase_point, char_ptr(erase_point) + erase_sz, move_sz);
    pos.m_node->size(pos.m_node->size() - erase_sz);
    std::memset(&*pos.m_node->leaf().end(), 0, erase_sz);

    if (pos.m_element != pos.m_node->leaf().end())
      return pos;
    btree_node_ptr next_node(pos.m_node->next_node());
    return !next_node ? end() : iterator(next_node, next_node->leaf().begin());
  }
}

//----------------------------------- m_sub_tree_begin() -------------------------------//

template <class Key, class Base, class Traits, class Comp>
typename btree_base<Key,Base,Traits,Comp>::iterator
btree_base<Key,Base,Traits,Comp>::m_sub_tree_begin(node_id_type id)
{
  if (empty())
    return end();
  btree_node_ptr np =  m_mgr.read(id);

  // work down the tree until a leaf is reached
  while (np->is_branch())
  {
    // create the child->parent list
    btree_node_ptr child_np = m_mgr.read(np->branch().begin()->node_id());
    child_np->parent(np);
    child_np->parent_element(np->branch().begin());
#   ifndef NDEBUG
    child_np->parent_node_id(np->node_id());
#   endif

    np = child_np;
  }

  return iterator(np, np->leaf().begin());
}

//------------------------------ m_erase_branch_value() --------------------------------//

template <class Key, class Base, class Traits, class Comp>
typename btree_base<Key,Base,Traits,Comp>::iterator
btree_base<Key,Base,Traits,Comp>::m_erase_branch_value(
  btree_node* np, branch_iterator element, node_id_type erasee)
{
  BOOST_ASSERT(np->is_branch());
  BOOST_ASSERT(&*element >= &*np->branch().begin());
  BOOST_ASSERT(&*element <= &*np->branch().end());  // equal to end if pseudo-element only
  BOOST_ASSERT(erasee == element->node_id());

  if (np->empty()) // end pseudo-element only element on node?
                   // i.e. after the erase, the entire sub-tree will be empty
  {
    BOOST_ASSERT(np->level() != header().root_level());
    BOOST_ASSERT(np->parent()->node_id() == np->parent_node_id()); // max_cache_size logic OK?
    iterator nxt = m_erase_branch_value(np->parent(),
      np->parent_element(), np->node_id()); // erase parent value pointing to np
    m_free_node(np); // move node to free node list
    return nxt;
  }
  else
  {
    void* erase_point;

    node_id_type next_id (element == np->branch().end() ? 0 : (element+1)->node_id());
  
    if (element != np->branch().begin())
    {
      --element;
      erase_point = &element->key();
    }
    else
    {
      erase_point = &element->node_id();
    }
    std::size_t erase_sz = dynamic_size(element->key()) + sizeof(node_id_type); 
    std::size_t move_sz = char_distance(
      char_ptr(erase_point)+erase_sz, &np->branch().end()->key());
    std::memmove(erase_point, char_ptr(erase_point) + erase_sz, move_sz);
    np->size(np->size() - erase_sz);
    std::memset(char_ptr(&*np->branch().end()) + sizeof(node_id_type), 0, erase_sz);
    np->needs_write(true);

    //  set up the return iterator
    if (!next_id)
    {
      btree_node_ptr next_np(np->next_node());
      if (!!next_np)
        next_id = next_np->branch().begin()->node_id();
    }
    iterator next_itr (next_id ? m_sub_tree_begin(next_id) : end());

    //  recursively free the root node if it is now empty, promoting the end
    //  pseudo element to be the new root
    while (np->level()   // not the leaf (which can happen if iteration reaches leaf)
      && np->branch().begin() == np->branch().end()  // node empty except for P0
      && np->level() == header().root_level())   // node is the root
    {
      // make the end pseudo-element the new root and then free this node
      m_hdr.root_node_id(np->branch().end()->node_id());
      m_hdr.decrement_root_level();
      m_root = m_mgr.read(header().root_node_id());
      m_root->parent(btree_node_ptr());
      m_root->parent_element(branch_iterator());
      m_free_node(np); // move node to free node list
      np = m_root.get();
    }
    return next_itr;
  }
}

template <class Key, class Base, class Traits, class Comp>   
typename btree_base<Key,Base,Traits,Comp>::size_type
btree_base<Key,Base,Traits,Comp>::erase(const key_type& k)
{
  BOOST_ASSERT_MSG(is_open(), "erase() on unopen btree");
  BOOST_ASSERT_MSG(!read_only(), "erase() on read only btree");
  size_type count = 0;
  const_iterator it = lower_bound(k);
    
  while (it != end() && !key_comp()(k, this->key(*it)))
  {
    ++count;
    it = erase(it);
  }
  return count;
}

template <class Key, class Base, class Traits, class Comp>   
typename btree_base<Key,Base,Traits,Comp>::const_iterator 
btree_base<Key,Base,Traits,Comp>::erase(const_iterator first, const_iterator last)
{
  BOOST_ASSERT_MSG(is_open(), "erase() on unopen btree");
  BOOST_ASSERT_MSG(!read_only(), "erase() on read only btree");
  // caution: last must be revalidated when on the same node as first
  while (first != last)
  {
    if (last != end() && first.m_node == last.m_node)
    {
      BOOST_ASSERT(first.m_element < last.m_element);
      --last;  // revalidate in anticipation of erasing a prior element on same node
    }
    first = erase(first);
  }
  return last;
}

//--------------------------------- m_insert_unique() ----------------------------------//

template <class Key, class Base, class Traits, class Comp>   
std::pair<typename btree_base<Key,Base,Traits,Comp>::const_iterator, bool>
btree_base<Key,Base,Traits,Comp>::m_insert_unique(const key_type& k,
  const mapped_type& mv)
{
  BOOST_ASSERT_MSG(is_open(), "insert() on unopen btree");
  BOOST_ASSERT_MSG(!read_only(), "insert() on read only btree");
  //BOOST_ASSERT_MSG(dynamic_size(k) + (&k != &mv ? dynamic_size(mv) : 0)
  //  < node_size()/3, "insert() value size too large for node size");
  iterator insert_point = m_special_lower_bound(k);

  bool is_unique = insert_point.m_element == insert_point.m_node->leaf().end()
                || key_comp()(k, this->key(*insert_point))
                || key_comp()(this->key(*insert_point), k);

  if (is_unique)
    return std::pair<const_iterator, bool>(m_leaf_insert(insert_point, k, mv), true);

  return std::pair<const_iterator, bool>(
    const_iterator(insert_point.m_node, insert_point.m_element), false); 
}

//------------------------------- m_insert_non_unique() --------------------------------//

template <class Key, class Base, class Traits, class Comp>   
inline typename btree_base<Key,Base,Traits,Comp>::const_iterator
btree_base<Key,Base,Traits,Comp>::m_insert_non_unique(const key_type& k,
  const mapped_type& mv)
{
  BOOST_ASSERT_MSG(is_open(), "insert() on unopen btree");
  BOOST_ASSERT_MSG(!read_only(), "insert() on read only btree");
  //BOOST_ASSERT_MSG(dynamic_size(k) + (&k != &mv ? dynamic_size(mv) : 0)
  //  < node_size()/3, "insert() value size too large for node size");
  iterator insert_point = m_special_upper_bound(k);
  return m_leaf_insert(insert_point, k, mv);
}

//----------------------------------- m_update() ---------------------------------------//

template <class Key, class Base, class Traits, class Comp>   
typename btree_base<Key,Base,Traits,Comp>::iterator
btree_base<Key,Base,Traits,Comp>::m_update(iterator itr,
  const mapped_type& new_mapped_value)
{
  BOOST_ASSERT_MSG(is_open(), "update() on unopen btree");
  BOOST_ASSERT_MSG(!read_only(), "update() on read only btree");
  BOOST_ASSERT_MSG(dynamic_size(new_mapped_value) == dynamic_size(itr->mapped_value()),
    "update() size of the new mapped value not equal size of the old mapped value");
  itr.m_node->needs_write(true);
  std::memcpy(const_cast<mapped_type*>(&itr->mapped_value()),
    &new_mapped_value, dynamic_size(new_mapped_value));
  return itr;
}

//----------------------------- m_special_lower_bound() --------------------------------//

template <class Key, class Base, class Traits, class Comp>   
typename btree_base<Key,Base,Traits,Comp>::iterator
btree_base<Key,Base,Traits,Comp>::m_special_lower_bound(const key_type& k) const
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
    branch_iterator low
      = std::lower_bound(np->branch().begin(), np->branch().end(), k, branch_comp());

    if ((header().flags() & btree::flags::unique)
      && low != np->branch().end()
      && !key_comp()(k, low->key())) // if k isn't less that low->key(), it is equal
      ++low;                         // and so must be incremented; this follows from
                                     // the branch node invariant for unique containers

    // create the child->parent list
    btree_node_ptr child_np = m_mgr.read(low->node_id());
    child_np->parent(np);
    child_np->parent_element(low);
#   ifndef NDEBUG
    child_np->parent_node_id(np->node_id());
#   endif

    np = child_np;
  }

  //  search leaf
  leaf_iterator low
    = std::lower_bound(np->leaf().begin(), np->leaf().end(), k, value_comp());

  return iterator(np, low);
}

//---------------------------------- lower_bound() -------------------------------------//

template <class Key, class Base, class Traits, class Comp>   
typename btree_base<Key,Base,Traits,Comp>::const_iterator
btree_base<Key,Base,Traits,Comp>::lower_bound(const key_type& k) const
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

template <class Key, class Base, class Traits, class Comp>   
typename btree_base<Key,Base,Traits,Comp>::iterator
btree_base<Key,Base,Traits,Comp>::m_special_upper_bound(const key_type& k) const
{
  btree_node_ptr np = m_root;

  // search branches down the tree until a leaf is reached
  while (np->is_branch())
  {
    branch_iterator up
      = std::upper_bound(np->branch().begin(), np->branch().end(), k, branch_comp());

    // create the child->parent list
    btree_node_ptr child_np = m_mgr.read(up->node_id());
    child_np->parent(np);
    child_np->parent_element(up);
#   ifndef NDEBUG
    child_np->parent_node_id(np->node_id());
#   endif

    np = child_np;
  }

  //  search leaf
  leaf_iterator up
    = std::upper_bound(np->leaf().begin(), np->leaf().end(), k, value_comp());

  return iterator(np, up);
}

//---------------------------------- upper_bound() -------------------------------------//

template <class Key, class Base, class Traits, class Comp>   
typename btree_base<Key,Base,Traits,Comp>::const_iterator
btree_base<Key,Base,Traits,Comp>::upper_bound(const key_type& k) const
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

template <class Key, class Base, class Traits, class Comp>   
typename btree_base<Key,Base,Traits,Comp>::const_iterator
btree_base<Key,Base,Traits,Comp>::find(const key_type& k) const
{
  BOOST_ASSERT_MSG(is_open(), "find() on unopen btree");
  const_iterator low = lower_bound(k);
  return (low != end() && !key_comp()(k, this->key(*low)))
    ? low
    : end();
}

//------------------------------------ count() -----------------------------------------//

template <class Key, class Base, class Traits, class Comp>   
typename btree_base<Key,Base,Traits,Comp>::size_type
btree_base<Key,Base,Traits,Comp>::count(const key_type& k) const
{
  BOOST_ASSERT_MSG(is_open(), "lower_bound() on unopen btree");
  size_type count = 0;

  for (const_iterator it = lower_bound(k);
        it != end() && !key_comp()(k, this->key(*it));
        ++it) { ++count; } 

  return count;
}

//----------------------------------- dump_dot -----------------------------------------//

template <class Key, class Base, class Traits, class Comp>   
void btree_base<Key,Base,Traits,Comp>::dump_dot(std::ostream& os) const
{
  BOOST_ASSERT_MSG(is_open(), "dump_dot() on unopen btree");
  os << "digraph btree {\nrankdir=LR;\nfontname=Courier;\n"
    "node [shape = record,margin=.1,width=.1,height=.1,fontname=Courier,style=\"filled\"];\n";

  for (unsigned int p = 1; p < header().node_count(); ++p)
  {
    btree_node_ptr np = m_mgr.read(p);

    if (np->is_leaf())
    {
      os << "node" << p << "[label = \"<f0> ";
      for (leaf_iterator it = np->leaf().begin(); it != np->leaf().end(); ++it)
      {
        if (it != np->leaf().begin())
          os << '|';
        os << *it;
      }
      os << "\",fillcolor=\"palegreen\"];\n";
   }
    else if (np->is_branch())
    {
      os << "node" << p << "[label = \"";
      int f = 0;
      branch_iterator it;
      for (it = np->branch().begin(); it != np->branch().end(); ++it)
      {
        os << "<f" << f << ">|" /*<< it->node_id() << ","*/ << it->key() << "|";
        ++f;
      }
      os << "<f" << f << ">\",fillcolor=\"lightblue\"];\n";
      f = 0;
      for (it = np->branch().begin(); it != np->branch().end(); ++it)
      {
        os << "\"node" << p << "\":f" << f << " -> \"node" << it->node_id() << "\":f0;\n";
        ++f;
      }
    os << "\"node" << p << "\":f" << f << " -> \"node" << it->node_id() << "\":f0;\n";
    }
  }

  os << "}" << std::endl;
}

////  non-member functions  ----------------------------------------------------//

///  non-member functions not implemented yet
/*
template <typename Key, typename T, typename Comp>
bool operator==(const common_base<Key,T,Comp,GetKey>& x,
              const common_base<Key,T,Comp,GetKey>& y);

template <typename Key, typename T, typename Comp>
bool operator< (const common_base<Key,T,Comp,GetKey>& x,
              const common_base<Key,T,Comp,GetKey>& y);

template <typename Key, typename T, typename Comp>
bool operator!=(const common_base<Key,T,Comp,GetKey>& x,
              const common_base<Key,T,Comp,GetKey>& y);

template <typename Key, typename T, typename Comp>
bool operator> (const common_base<Key,T,Comp,GetKey>& x,
              const common_base<Key,T,Comp,GetKey>& y);

template <typename Key, typename T, typename Comp>
bool operator>=(const common_base<Key,T,Comp,GetKey>& x,
              const common_base<Key,T,Comp,GetKey>& y);

template <typename Key, typename T, typename Comp>
bool operator<=(const common_base<Key,T,Comp,GetKey>& x,
              const common_base<Key,T,Comp,GetKey>& y);

template <typename Key, typename T, typename Comp>
void swap(common_base<Key,T,Comp,GetKey>& x,
        common_base<Key,T,Comp,GetKey>& y);
*/
//--------------------------------------------------------------------------------------//

template <class Key, class Base, class Traits, class Comp>
template <class T>
void
btree_base<Key,Base,Traits,Comp>::iterator_type<T>::increment()
{
  BOOST_ASSERT_MSG(m_element != typename btree_base::leaf_iterator(0),
    "increment of end iterator"); 
  BOOST_ASSERT(m_node);
  BOOST_ASSERT(&*m_element >= &*m_node->leaf().begin());
  BOOST_ASSERT(&*m_element < &*m_node->leaf().end());

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
    *this = reinterpret_cast<const btree_base<Key,Base,Traits,Comp>*>
        (np->manager()->owner())->m_end_iterator;
  }
}

template <class Key, class Base, class Traits, class Comp>
template <class T>
void
btree_base<Key,Base,Traits,Comp>::iterator_type<T>::decrement()
{
  if (*this == reinterpret_cast<const btree_base<Key,Base,Traits,Comp>*>
        (m_node->manager()->owner())->end())
    *this = reinterpret_cast<btree_base<Key,Base,Traits,Comp>*>
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
      *this = reinterpret_cast<const btree_base<Key,Base,Traits,Comp>*>
          (np->manager()->owner())->m_end_iterator;
    }
  }
}

}  // namespace btree
}  // namespace boost

#endif // BOOST_BTREE_COMMON_HPP

