//  boost/detail/common_base.hpp  ------------------------------------------------------//

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

  * implement emplace(). Howard speculates emplace() makes the map/multimap insert()
    key, mapped_value overload unnecessary. 

  * vbtree_unit_test.cpp: move erase tests out of insert test.

  * Add static_assert Key, T are is_trivially_copyable

  * Add check for trying to insert a value sized >= 1/4 (page size - begin)

  * map, multi_map, insert(key, mapped_value) can be confused with template insert?
    Use enable_if?

  * If not variable size, one or both internal iterators should be random access tag,
    including case where leaf is fwd, but branch is random

  * advance_by_size should dispatch on iterator type and use more efficient
    algorithm if random access (I.E. fixed size)

  * header() shouldn't be part of the public interface.
      - Add individual get, and where appropriate, set, functions.
      - Move header file to detail.

  * For multi-containers, consider branch pages with the same number of P and K entries,
    where the invariant then becomes Kn <= Keys in Pn 

  * For multi-containers, add a test case of a deep tree with all the same key. Then
    test erasing various elements.

  * Pack optimization should apply to branches too. 

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

//--------------------------------- dynamic_size ---------------------------------------//

//  Must be overloaded for any type T whose dynamic size differs from sizeof(T)
//  See http://www.gotw.ca/publications/mill17.htm,
//  Why Not Specialize: The Dimov/Abrahams Example

template <class T>
inline std::size_t dynamic_size(const T&) { return sizeof(T); }

//--------------------------------- vbtree_value ---------------------------------------//

// TODO: either add code to align mapped() or add a requirement that T2 does not
// require alignment.

template <class T1, class T2>
class vbtree_value
{
public:
  T1& key() const   { return *reinterpret_cast<const T1*>(this); }
  const T2& mapped_value() const
  {
    return *reinterpret_cast<T2*>(reinterpret_cast<const char*>(this)
             + btree::dynamic_size(key()));
  }
  std::size_t dynamic_size() const
  { 
    return btree::dynamic_size(key()) + btree::dynamic_size(mapped_value());
  }
};

template <class T1, class T2>
std::ostream& operator<<(std::ostream& os, const vbtree_value<T1, T2>& x)
{
  os << x.key() << ',' << x.mapped_value();
  return os;
}

template <class T1, class T2>
inline std::size_t dynamic_size(const vbtree_value<T1, T2>& x) {return x.dynamic_size();}

//  less function object class

template <class T> struct less
{
  typedef T first_argument_type;
  typedef T second_argument_type;
  typedef bool result_type;
  bool operator()(const T& x, const T& y) const { return x < y; }
};

//--------------------------------------------------------------------------------------//
//                             class vbtree_set_base                                    //
//--------------------------------------------------------------------------------------//

template <class Key, class Comp>
class vbtree_set_base
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
//                             class vbtree_map_base                                    //
//--------------------------------------------------------------------------------------//

template <class Key, class T, class Comp>
class vbtree_map_base
{
public:
  typedef vbtree_value<const Key, const T>  value_type;
  typedef T                                 mapped_type;

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
  //-------------------------------- dynamic_iterator ----------------------------------//
  //
  //  Pointers to elements on a page aren't useful as iterators because the elements are
  //  variable length. dynamic_iterator provides the correct semantics.

  template <class T>
  class dynamic_iterator
    : public boost::iterator_facade<dynamic_iterator<T>, T, bidirectional_traversal_tag>
  {
  public:
    dynamic_iterator() : m_ptr(0), m_begin(0) {} 
    explicit dynamic_iterator(T* ptr) : m_ptr(ptr) {m_begin = ptr;}
    dynamic_iterator(T* ptr, std::size_t sz)
      : m_ptr(reinterpret_cast<T*>(reinterpret_cast<char*>(ptr) + sz)) {m_begin = ptr;}

    bool operator<(const dynamic_iterator& rhs) const
    {
      BOOST_ASSERT(m_begin == rhs.m_begin);  // on the same page
      return m_ptr < rhs.m_ptr;
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
      m_ptr = reinterpret_cast<T*>(reinterpret_cast<char*>(m_ptr)
        + btree::dynamic_size(*m_ptr));
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

  //--------------------------------- branch_value -------------------------------------//

  // TODO: either add code to align mapped() or add a requirement that PID, K does not
  // require alignment.

  template <class PID, class K>
  class branch_value
  {
  public:
    PID&  page_id() {return *reinterpret_cast<PID*>(this); }
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
    std::size_t dynamic_size() const
    { 
      return sizeof(PID) + btree::dynamic_size(key());
    }
  };

  //------------------------------- advance_by_size ------------------------------------//

  template <class ForwardIterator>
  ForwardIterator advance_by_size(ForwardIterator begin, std::size_t max_sz)
  {
    BOOST_ASSERT(max_sz);
    ForwardIterator prior;
    for (std::size_t sz = 0; sz <= max_sz; sz += dynamic_size(*begin)) 
    {
      prior = begin;
      ++begin;
    }
    return prior;
  }

}  // namespace detail

template <class PID, class K>
std::size_t dynamic_size(const detail::branch_value<PID, K>& v) {return v.dynamic_size();}

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                                 class vbtree_base                                    //
//                                                                                      //
//                  a B+ tree with leaf-sequence doubly-linked list                     //
//                                                                                      //
//--------------------------------------------------------------------------------------//

template  <class Key,
           class Base,  // vbtree_map_base or vbtree_set_base
           class Traits,
           class Comp>

class vbtree_base : public Base, private noncopyable
{
private:
  class btree_page;
  class btree_page_ptr;
  class btree_data;
  class leaf_data;
  class branch_data;
  class leaf_page;
  class branch_page;
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

  typedef typename Traits::page_id_type         page_id_type;
  typedef typename Traits::page_size_type       page_size_type;
  typedef typename Traits::page_level_type      page_level_type;

  typedef value_type                                 leaf_value_type;
  typedef detail::dynamic_iterator<leaf_value_type>  leaf_iterator;

  // construct/destroy:

  vbtree_base(const Comp& comp);

  vbtree_base(const boost::filesystem::path& p, flags::bitmask flgs, std::size_t pg_sz,
             const Comp& comp);

  ~vbtree_base();

  //  file operations:

  void flush()                              { if (m_mgr.flush())
                                                m_write_header();
                                            }
  void close();

  // TODO: operator unspecified-bool-type, operator!
  
  // iterators:

  iterator  writable(const_iterator& iter)  {iter.m_page->m_needs_write(true);
                                              return iterator(iter.m_page, iter.m_element);
                                            }
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
  std::size_t   page_size() const           { return m_mgr.data_size(); }
  std::size_t   max_cache_pages() const     { return m_mgr.max_cache_buffers(); }
  void          max_cache_pages(std::size_t m)
                                            {
                                              m_mgr.max_cache_buffers(m);
                                              m_set_max_cache_pages();
                                            }

  //  The following element access functions are not provided. Returning references is
  //  far too dangerous, since the memory pointed to would be in a page buffer that can
  //  overwritten by other activity, including calls to const functions. Access via
  //  iterators doesn't suffer the same since iterators contain reference counted smart
  //  pointers to the page's memory.
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

  btree_page_ptr     m_root;  // invariant: there is always at least one leaf,
                              // possibly empty, in the tree, and thus there is
                              // always a root. If the tree has only one leaf
                              // page, that page is the root

  //  end iterator mechanism: needed so that decrement of end() is implementable
  buffer             m_end_page;  // end iterators point to this page, providing
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
    page_level_type  m_level;        // leaf: 0, branches: distance from leaf,
                                     // header: 0xFFFF, free page list entry: 0xFFFE
    page_size_type   m_size;         // size in bytes of elements on page
  };
  
  //------------------------ leaf data formats and operations --------------------------//

 class leaf_data : public btree_data
  {
    friend class vbtree_base;
  public:
    page_id_type   prior_page_id() const           {return m_prior_page_id;}
    void           prior_page_id(page_id_type id)  {m_prior_page_id = id;}
    page_id_type   next_page_id() const            {return m_next_page_id;}
    void           next_page_id(page_id_type id)   {m_next_page_id = id;}
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
    page_id_type     m_next_page_id;   // page sequence fwd list; 0 for end
    page_id_type     m_prior_page_id;  // page sequence bwd list; 0 for end
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
      //  Unique containers:  Pn < Kn <= Pn+1   Keys in Pn are < Kn                 //
      //                                        Kn <= Keys in Pn+1                  //
      //                                                                            //
      //  Multi containers:                                                         //
      //                                                                            //
      //----------------------------------------------------------------------------//

  typedef detail::branch_value<page_id_type, key_type>  branch_value_type;
  typedef detail::dynamic_iterator<branch_value_type>   branch_iterator;

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

  ////  Access to (itr-1)->page_id on branch pages may be expensive since it may be a
  ////  ForwardIterator, and won't work at all for access to PO given begin(). The fix
  ////  is a function that knows what to do.
  //page_id_type prior_page_id(branch_iterator itr) const
  //{
  //  // page_id_type must be unaligned || key_type must have same or stronger alignment
  //  return *reinterpret_cast<const page_id_type*>(
  //    reinterpret_cast<const char*>(&*itr) - sizeof(page_id_type));
  //}

  class branch_page;

  class btree_page : public buffer
  {
  public:
    btree_page() : buffer() {}
    btree_page(buffer::buffer_id_type id, buffer_manager& mgr)
      : buffer(id, mgr) {}

    page_id_type       page_id() const                 {return page_id_type(buffer_id());}

    //----------------------------------------------------------------------------------//
    // WARNING: The child->parent list is ephemeral, and is only valid when it has been //
    // established (by m_lower_page/m_upper_page) during insert and erase operations.   //
    // Once those operations are complete, child->parent list pointers are not valid.   //
    //----------------------------------------------------------------------------------//
    btree_page*        parent()                          {return m_parent;}
    void               parent(btree_page* p)             {m_parent = p;}
    branch_iterator    parent_element()                  {return m_parent_element;}
    void               parent_element(branch_iterator p) {m_parent_element = p;}
#   ifndef NDEBUG
    page_id_type       parent_page_id()                  {return m_parent_page_id;}
    void               parent_page_id(page_id_type id)   {m_parent_page_id = id;}
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

  private:
    btree_page*         m_parent;   // by definition, the parent is a branch page.
                                    // rationale for raw pointer: (1) elminate overhead
                                    // of revisiting pages to do btree_page_ptr::reset()
                                    // (2) allows single m_upper/m_lower_page search
                                    // function to cover both modifying and non-modifying
                                    // uses.
    branch_iterator  m_parent_element;
# ifndef NDEBUG
    page_id_type        m_parent_page_id;  // allows assert that m_parent has not been
                                           // overwritten by faulty max_cache_pages
# endif
  };

  //-------------------------------  btree_page_ptr  -----------------------------------//

  class btree_page_ptr : public buffer_ptr
  {
  public:

    btree_page_ptr() : buffer_ptr() {}
    btree_page_ptr(btree_page& p) : buffer_ptr(p) {}
    btree_page_ptr(buffer& p) : buffer_ptr(p) {}
    btree_page_ptr(const btree_page_ptr& r) : buffer_ptr(r) {} 
    btree_page_ptr(const buffer_ptr& r) : buffer_ptr(r) {}
    btree_page_ptr& operator=(const btree_page_ptr& r)
    {
      btree_page_ptr(r).swap(*this);  // correct for self-assignment
      return *this;
    }
    btree_page_ptr& operator=(const buffer_ptr& r)
    {
      btree_page_ptr(r).swap(*this);  // correct for self-assignment
      return *this;
    }
    btree_page* get() const {return static_cast<btree_page*>(m_ptr);}
    btree_page& operator*() const
    {
      BOOST_ASSERT(m_ptr);
      return *static_cast<btree_page*>(m_ptr);
    }
    btree_page* operator->() const
    {
      BOOST_ASSERT(m_ptr);
      return static_cast<btree_page*>(m_ptr);
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
      : m_page(static_cast<typename vbtree_base::btree_page_ptr>(p)),
        m_element(e) {}

  private:
    iterator_type(buffer_ptr p)  // used solely to setup the end iterator
      : m_page(static_cast<typename vbtree_base::btree_page_ptr>(p)),
        m_element(0) {}

    friend class boost::iterator_core_access;
    friend class vbtree_base;
   
    typename vbtree_base::btree_page_ptr  m_page; 
    typename vbtree_base::leaf_iterator   m_element;  // 0 for end iterator

    T& dereference() const  { return *m_element; }
 
    bool equal(const iterator_type& rhs) const
    {
      return m_element == rhs.m_element
        // check page_id() in case page memory has been reused
        && m_page->page_id() == rhs.m_page->page_id();
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

  void m_open(const boost::filesystem::path& p, flags::bitmask flgs, std::size_t pg_sz);

//--------------------------------------------------------------------------------------//
//                              private member functions                                //
//--------------------------------------------------------------------------------------//
private:

  static buffer* m_page_alloc(buffer::buffer_id_type pg_id, buffer_manager& mgr)
  { return new btree_page(pg_id, mgr); }

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

  iterator m_lower_page_bound(const key_type& k);
  // returned iterator::m_element is the insertion point, and thus may be the 
  // past-the-end leaf_iterator for iterator::m_page
  // postcondition: parent pointers are set, all the way up the chain to the root
  iterator m_upper_page_bound(const key_type& k);
  // returned iterator::m_element is the insertion point, and thus may be the 
  // past-the-end leaf_iterator for iterator::m_page
  // postcondition: parent pointers are set, all the way up the chain to the root
  btree_page_ptr m_new_page(boost::uint16_t lv);
  void  m_new_root();
  const_iterator m_leaf_insert(iterator insert_iter, const key_type& key,
    const mapped_type& mapped_value);
  void  m_branch_insert(btree_page* pg, branch_iterator element,
    const key_type& k, page_id_type id);

  struct branch_value_type;
  void  m_erase_branch_value(btree_page* pg, branch_iterator value, page_id_type erasee);
  void  m_free_page(btree_page* pg)
  {
    pg->needs_write(true);
    pg->level(0xFFFE);
    pg->size(0);
    pg->leaf().prior_page_id(page_id_type());
    pg->leaf().next_page_id(page_id_type(m_hdr.free_page_list_head_id()));
    m_hdr.free_page_list_head_id(pg->page_id());
  }

  void m_set_max_cache_pages()
  {
    // To ensure that the ephemeral child->parent list of btree_pages remains valid during
    // inserts and erases, the minimum size of max_cache_pages must be set large enough
    // that pages in the list are never overwritten.
    //
    // The worst case is believed to be when an insert is done, and every page from the leaf
    // to the root is full.

    std::size_t minimum_cache_pages =
      (m_hdr.levels()) * 2  // two pages per level to handle the splits
      + 2                   // previous and next pages accessed to update sequence list
      + 2;                  // safety margin
   
    if (m_mgr.max_cache_buffers() < minimum_cache_pages)
      m_mgr.max_cache_buffers(minimum_cache_pages);
  }

  //-------------------------------- branch_compare ------------------------------------//

  class branch_compare
//    : public std::binary_function<branch_value_type, key_type, bool>
  {
    friend class vbtree_base;
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

};  // class vbtree_base


//--------------------------------------------------------------------------------------//
//                              non-member operator <<                                  //
//--------------------------------------------------------------------------------------//

template <class Key, class Base, class Traits, class Comp>   
std::ostream& operator<<(std::ostream& os,
  const vbtree_base<Key,Base,Traits,Comp>& bt)
{
  os << "B+ tree \"" << bt.file_path().string() << "\"\n"
     << bt.header().element_count() << " records\n"  
     << bt.header().page_size() << " page size\n"  
     << bt.header().page_count() << " page count\n"  
     << bt.header().root_page_id() << " root page id\n"  
     << bt.header().root_level()+1 << " levels in tree\n"
     << "User supplied string: \"" << bt.header().user_c_str() << "\"\n"
  ;
  return os;
}

//--------------------------------------------------------------------------------------//
//                          class vbtree_base implementation                             //
//--------------------------------------------------------------------------------------//

//------------------------------ construct without open --------------------------------//

template <class Key, class Base, class Traits, class Comp>
vbtree_base<Key,Base,Traits,Comp>::vbtree_base(const Comp& comp)
  : m_mgr(m_page_alloc), m_comp(comp), m_value_comp(comp), m_branch_comp(comp)
{ 
  m_mgr.owner(this);

  // set up the end iterator
  m_end_page.manager(&m_mgr);
  m_end_iterator = const_iterator(buffer_ptr(m_end_page));
}

//------------------------------- construct with open ----------------------------------//

template <class Key, class Base, class Traits, class Comp>
vbtree_base<Key,Base,Traits,Comp>::vbtree_base(const boost::filesystem::path& p,
  flags::bitmask flgs, std::size_t pg_sz, const Comp& comp)
  : m_mgr(m_page_alloc), m_comp(comp), m_value_comp(comp), m_branch_comp(comp)
{ 
  m_mgr.owner(this);

  // set up the end iterator
  m_end_page.manager(&m_mgr);
  m_end_iterator = const_iterator(buffer_ptr(m_end_page));

  // open the file and set up data members
  m_open(p, flgs, pg_sz);
}

//----------------------------------- destructor ---------------------------------------//

template <class Key, class Base, class Traits, class Comp>
vbtree_base<Key,Base,Traits,Comp>::~vbtree_base()
{
  try { close(); }
  catch (...) {}
}

//------------------------------------- close ------------------------------------------//

template <class Key, class Base, class Traits, class Comp>
void vbtree_base<Key,Base,Traits,Comp>::close()
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
vbtree_base<Key,Base,Traits,Comp>::m_open(const boost::filesystem::path& p,
  flags::bitmask flgs, std::size_t pg_sz) 
{
  BOOST_ASSERT(!is_open());
  BOOST_ASSERT(pg_sz >= sizeof(btree::header_page));
// TODO: need max_value_type_size like the old lib95 btree
//  BOOST_ASSERT(pg_sz / sizeof(value_type) >= 3);
//  BOOST_ASSERT(pg_sz / sizeof(branch_value_type) >= 3);

  oflag::bitmask open_flags = oflag::in;
  if (flgs & flags::read_write)
    open_flags |= oflag::out;
  if (flgs & flags::truncate)
    open_flags |= oflag::out | oflag::truncate;
  if (flgs & flags::preload)
    open_flags |= oflag::preload;

  m_read_only = (open_flags & oflag::out) == 0;
  m_ok_to_pack = true;
  m_max_leaf_size = pg_sz - leaf_data::value_offset();
  m_max_branch_size = pg_sz - branch_data::value_offset();

  if (m_mgr.open(p, open_flags, btree::default_max_cache_pages, pg_sz))
  { // existing non-truncated file
    m_read_header();
    if (!m_hdr.marker_ok())
      BOOST_BTREE_THROW(std::runtime_error(file_path().string()+" isn't a btree"));
    if (m_hdr.big_endian() != (Traits::header_endianness == integer::endianness::big))
      BOOST_BTREE_THROW(std::runtime_error(file_path().string()+" has wrong endianness"));
    //if (m_hdr.key_size() != Base::key_size())
    //  BOOST_BTREE_THROW(std::runtime_error(file_path().string()+" has wrong key_type size"));
    //if (m_hdr.mapped_size() != Base::mapped_size())
    //  BOOST_BTREE_THROW(std::runtime_error(file_path().string()+" has wrong mapped_type size"));
    m_mgr.data_size(m_hdr.page_size());
    m_root = m_mgr.read(m_hdr.root_page_id());
  }
  else
  { // new or truncated file
    m_hdr.big_endian(Traits::header_endianness == integer::endianness::big);
    m_hdr.flags(flgs & ~(btree::flags::read_write | btree::flags::truncate));
    m_hdr.splash_c_str("boost.org btree");
    m_hdr.user_c_str("");
    m_hdr.page_size(pg_sz);
    m_hdr.key_size(Base::key_size());
//    BOOST_ASSERT(m_hdr.key_size() == Base::key_size());  // fails if key_type too large
    m_hdr.mapped_size(Base::mapped_size());
//    BOOST_ASSERT(m_hdr.mapped_size() == Base::mapped_size());  // fails if mapped_type too large
    m_hdr.increment_page_count();  // i.e. the header itself
    m_mgr.new_buffer();  // force a buffer write, thus zeroing the header for its full size
    flush();
    m_write_header();  // not totally necessary

    // set up an empty leaf as the initial root
    m_root = m_mgr.new_buffer();
    m_root->needs_write(true);
    m_hdr.increment_page_count();
    BOOST_ASSERT(m_root->page_id() == 1);
    m_hdr.root_page_id(m_root->page_id());
    m_hdr.first_page_id(m_root->page_id());
    m_hdr.last_page_id(m_root->page_id());
    m_root->level(0);
    m_root->size(0);
    m_root->leaf().prior_page_id(page_id_type(0));
    m_root->leaf().next_page_id(page_id_type(0));
  }
  m_set_max_cache_pages();
}

//------------------------------------- clear() ----------------------------------------//

template <class Key, class Base, class Traits, class Comp>
void
vbtree_base<Key,Base,Traits,Comp>::clear()
{
  BOOST_ASSERT_MSG(is_open(), "can't clear() unopen btree");
  for (buffer_manager::buffer_set_type::iterator itr = m_mgr.buffer_set.begin();
    itr != m_mgr.buffer_set.end();
    ++itr)
  {
    itr->needs_write(false);
  }

  m_hdr.element_count(0);
  m_hdr.root_page_id(1);
  m_hdr.first_page_id(1);
  m_hdr.last_page_id(1);
  m_hdr.root_level(0);
  m_hdr.page_count(0);
  m_hdr.free_page_list_head_id(0);


  m_mgr.close();

}

//------------------------------------- begin() ----------------------------------------//

template <class Key, class Base, class Traits, class Comp>
typename vbtree_base<Key,Base,Traits,Comp>::const_iterator
vbtree_base<Key,Base,Traits,Comp>::begin() const
{
  BOOST_ASSERT_MSG(is_open(), "begin() on unopen btree");
  if (empty())
    return end();
  BOOST_ASSERT(header().first_page_id());                     
  btree_page_ptr pg(m_mgr.read(header().first_page_id()));
  BOOST_ASSERT(pg->is_leaf());
  return const_iterator(pg, pg->leaf().begin());
}

//-------------------------------------- last() ---------------------------------------//

template <class Key, class Base, class Traits, class Comp>
typename vbtree_base<Key,Base,Traits,Comp>::const_iterator
vbtree_base<Key,Base,Traits,Comp>::last() const
{
  BOOST_ASSERT_MSG(is_open(), "last() on unopen btree");
  if (empty())
    return end();
  BOOST_ASSERT(header().last_page_id());
  btree_page_ptr pg(m_mgr.read(header().last_page_id()));
  BOOST_ASSERT(pg->is_leaf());
  return const_iterator(pg, pg->leaf().end()-1);
}

//---------------------------------- m_new_page() --------------------------------------//

template <class Key, class Base, class Traits, class Comp>   
typename vbtree_base<Key,Base,Traits,Comp>::btree_page_ptr 
vbtree_base<Key,Base,Traits,Comp>::m_new_page(boost::uint16_t lv)
{
  btree_page_ptr pg;
  if (m_hdr.free_page_list_head_id())
  {
    pg = m_mgr.read(m_hdr.free_page_list_head_id());
    BOOST_ASSERT(pg->level() == 0xFFFE);  // free page list entry
    m_hdr.free_page_list_head_id(pg->leaf().next_page_id());
  }
  else
  {
    pg = m_mgr.new_buffer();
    m_hdr.increment_page_count();
    BOOST_ASSERT(m_hdr.page_count() == m_mgr.buffer_count());
  }

  pg->needs_write(true);
  pg->level(lv);
  pg->size(0);
  return pg;
}

//----------------------------------- m_new_root() -------------------------------------//

template <class Key, class Base, class Traits, class Comp>   
void
vbtree_base<Key,Base,Traits,Comp>::m_new_root()
{ 
  // create a new root containing only the P0 pseudo-element
  btree_page_ptr old_root = m_root;
  page_id_type old_root_id(m_root->page_id());
  m_hdr.increment_root_level();
  m_set_max_cache_pages();
  m_root = m_new_page(m_hdr.root_level());
  m_hdr.root_page_id(m_root->page_id());
  m_root->branch().begin()->page_id() = old_root_id;
  m_root->size(0);  // the end pseudo-element doesn't count as an element
  // TODO: why maintain the child->parent list? By the time m_new_root() is called,
  // hasn't the need passed?
  m_root->parent(0);  
  m_root->parent_element(branch_iterator());
  old_root->parent(m_root.get());
  old_root->parent_element(m_root->branch().begin());
# ifndef NDEBUG
  m_root->parent_page_id(page_id_type(0));
  old_root->parent_page_id(m_root->page_id());
# endif
}

//---------------------------------- m_leaf_insert() -----------------------------------//

template <class Key, class Base, class Traits, class Comp>   
typename vbtree_base<Key,Base,Traits,Comp>::const_iterator
vbtree_base<Key,Base,Traits,Comp>::m_leaf_insert(iterator insert_iter,
  const key_type& key_, const mapped_type& mapped_value_)
{
  std::size_t          key_size= dynamic_size(key_);
  std::size_t          mapped_size = (header().flags() & btree::flags::key_only)
    ? 0
    : dynamic_size(mapped_value_);
  std::size_t          value_size = key_size + mapped_size;;
  btree_page_ptr       pg = insert_iter.m_page;
  leaf_iterator        insert_begin = insert_iter.m_element;
  btree_page_ptr       pg2;
  
  BOOST_ASSERT_MSG(pg->is_leaf(), "internal error");
  BOOST_ASSERT_MSG(pg->size() <= m_max_leaf_size, "internal error");

  m_hdr.increment_element_count();
  pg->needs_write(true);

  if (pg->size() + value_size > m_max_leaf_size)  // no room on page?
  {
    //  no room on page, so page must be split

    if (pg->level() == m_hdr.root_level()) // splitting the root?
      m_new_root();  // create a new root
    
    pg2 = m_new_page(pg->level());  // create the new page 

    // ck pack conditions now, since leaf seq list update may chg header().last_page_id()
    if (m_ok_to_pack
        && (insert_begin != pg->leaf().end() || pg->page_id() != header().last_page_id()))
      m_ok_to_pack = false;  // conditions for pack optimization not met

    // insert the new page into the leaf sequence lists
    pg2->leaf().prior_page_id(pg->page_id());
    pg2->leaf().next_page_id(pg->leaf().next_page_id());
    pg->leaf().next_page_id(pg2->page_id());
    if (pg2->leaf().next_page_id())
    {
      const btree_page_ptr next_page(m_mgr.read(pg2->leaf().next_page_id()));
      next_page->leaf().prior_page_id(pg2->page_id());
      next_page->needs_write(true);
    }
    else 
      m_hdr.last_page_id(pg2->page_id());

    // apply pack optimization if applicable
    if (m_ok_to_pack)  // have all inserts been ordered and no erases occurred?
    {
      // pack optimization: instead of splitting pg, just put value alone on pg2
      m_memcpy_value(&*pg2->leaf().begin(), &key_, key_size, &mapped_value_, mapped_size);  // insert value
      pg2->size(value_size);
      BOOST_ASSERT(pg->parent()->page_id() == pg->parent_page_id()); // max_cache_size logic OK?
      m_branch_insert(pg->parent(), pg->parent_element(),
        key(*pg2->leaf().begin()), pg2->page_id());
      return const_iterator(pg2, pg2->leaf().begin());
    }

    // split page pg by moving half the elements, by size, to page p2
    leaf_iterator split_begin(detail::advance_by_size(pg->leaf().begin(),
      pg->leaf().size() / 2));
    ++split_begin; // for leaves, prefer more aggressive split begin
    std::size_t split_sz = char_distance(&*split_begin, &*pg->leaf().end());

    // TODO: if the insert point will fall on the new page, it would be faster to
    // copy the portion before the insert point, copy the value being inserted, and
    // finally copy the portion after the insert point. However, that's a fair amount of
    // additional code for something that only happens on half of all leaf splits on average.

    std::memcpy(&*pg2->leaf().begin(), &*split_begin, split_sz);
    pg2->size(split_sz);
    std::memset(&*split_begin, 0,                         // zero unused space to make
      char_distance(&*split_begin, &*pg->leaf().end()));  //  file dumps easier to read
    pg->size(pg->size() - split_sz);

    // adjust pg and insert_begin if they now fall on the new page due to the split
    if (&*split_begin < &*insert_begin)
    {
      pg = pg2;
      insert_begin = leaf_iterator(&*pg->leaf().begin(),
        char_distance(&*split_begin, &*insert_begin));  
    }
  }

  //  insert value into pg at insert_begin
  BOOST_ASSERT(&*insert_begin >= &*pg->leaf().begin());
  BOOST_ASSERT(&*insert_begin <= &*pg->leaf().end());
  std::memmove(char_ptr(&*insert_begin) + value_size,
    &*insert_begin, char_distance(&*insert_begin, &*pg->leaf().end()));  // make room
  m_memcpy_value(&*insert_begin, &key_, key_size, &mapped_value_, mapped_size);  // insert value
  pg->size(pg->size() + value_size);

  // if there is a new page, its initial key and page_id are inserted into parent
  if (pg2)
  {
    BOOST_ASSERT(insert_iter.m_page->parent()->page_id() \
      == insert_iter.m_page->parent_page_id()); // max_cache_size logic OK?
    m_branch_insert(insert_iter.m_page->parent(),
      insert_iter.m_page->parent_element(),
      key(*pg2->leaf().begin()), pg2->page_id());
  }

  return const_iterator(pg, insert_begin);
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
vbtree_base<Key,Base,Traits,Comp>::m_branch_insert(
  btree_page* pg1, branch_iterator element, const key_type& k, page_id_type id) 
{
  std::size_t       k_size = dynamic_size(k);
  std::size_t       insert_size = k_size + sizeof(page_id_type);
  btree_page*       pg = pg1;
  key_type*         insert_begin = &element->key();
  btree_page_ptr    pg2;

  BOOST_ASSERT(pg->is_branch());
  BOOST_ASSERT(pg->size() <= m_max_branch_size);

  pg->needs_write(true);

  if (pg->size() + insert_size > m_max_branch_size)  // no room on page?
  {
    //  no room on page, so page must be split
std::cout << "Splitting branch\n";

    if (pg->level() == m_hdr.root_level()) // splitting the root?
      m_new_root();  // create a new root
    
    pg2 = m_new_page(pg->level());  // create the new page

    // split page pg by moving half the elements, by size, to page p2

    branch_iterator unsplit_end(detail::advance_by_size(pg->branch().begin(),
      pg->branch().size() / 2));
    branch_iterator split_begin(unsplit_end+1);
    std::size_t split_sz = char_distance(&*split_begin, char_ptr(&*pg->branch().end()) 
      + sizeof(page_id_type));  // include the end pseudo-element page_id
    BOOST_ASSERT(split_sz > sizeof(page_id_type));

    // TODO: if the insert point will fall on the new page, it would be faster to
    // copy the portion before the insert point, copy the value being inserted, and
    // finally copy the portion after the insert point. However, that's a fair amount of
    // additional code for something that only happens on half of all branch splits on average.

    //// if the insert will be at the start just copy everything to the proper location
    //if (split_begin == insert_begin)
    //{
    //  pg2->branch().P0 = id;
    //  std::memcpy(&*pg2->branch().begin(), &*split_begin, split_sz); // move split values to new page
    //  pg2->size(split_sz);
    //  std::memset(&*split_begin, 0,  // zero unused space to make file dumps easier to read
    //    (pg->branch().end() - split_begin) * sizeof(branch_value_type)); 
    //  pg->size(pg->size() - split_sz);
    //  BOOST_ASSERT(pg->parent()->page_id() == pg->parent_page_id()); // max_cache_size logic OK?
    //  m_branch_insert(pg->parent(), pg->parent_element()+1, k, pg2->page_id());
    //  return;
    //}

    // copy the split elements, including the pseudo-end element, to p2
    std::memcpy(&*pg2->branch().begin(), &*split_begin, split_sz);  // include end pseudo element
    pg2->size(split_sz - sizeof(page_id_type));  // exclude end pseudo element from size

    BOOST_ASSERT(pg->parent()->page_id() == pg->parent_page_id()); // max_cache_size logic OK?

    // promote the key from the original page's new end pseudo element to the parent branch page
    m_branch_insert(pg->parent(), pg->parent_element(), unsplit_end->key(), pg2->page_id());

    // finalize work on the original page
    std::memset(&unsplit_end->key(), 0,  // zero unused space to make file dumps easier to read
      char_distance(&unsplit_end->key(), &pg->branch().end()->key())); 
    pg->size(char_distance(&*pg->branch().begin(), &*unsplit_end));

    // adjust pg and insert_begin if they now fall on the new page due to the split
    if (&*split_begin <= &*element)
    {
      pg = pg2.get();
      insert_begin = reinterpret_cast<key_type*>(char_ptr(&pg2->branch().begin()->key())
        + char_distance(&split_begin->key(), insert_begin));
    }
  }

  //  insert k, id, into pg at insert_begin
  BOOST_ASSERT(insert_begin >= &pg->branch().begin()->key());
  BOOST_ASSERT(insert_begin <= &pg->branch().end()->key());
  std::memmove(char_ptr(insert_begin) + insert_size,
    insert_begin, char_distance(insert_begin, &pg->branch().end()->key()));  // make room
  std::memcpy(insert_begin, &k, k_size);  // insert k
  std::memcpy(char_ptr(insert_begin) + k_size, &id, sizeof(page_id_type));
  pg->size(pg->size() + insert_size);

#ifndef NDEBUG
  if (!(m_hdr.flags() & btree::flags::multi))
  {
    branch_iterator cur = pg->branch().begin();
    key_type prev_key = cur->key();
    ++cur;
    for(; cur != pg->branch().end(); ++cur)
    {
      BOOST_ASSERT(key_comp()(prev_key, cur->key()));
      prev_key = cur->key();
    }
  }
#endif
}

//------------------------------------- erase() ----------------------------------------//

template <class Key, class Base, class Traits, class Comp>   
typename vbtree_base<Key,Base,Traits,Comp>::const_iterator
vbtree_base<Key,Base,Traits,Comp>::erase(const_iterator pos)
{
  BOOST_ASSERT_MSG(is_open(), "erase() on unopen btree");
  BOOST_ASSERT_MSG(pos != end(), "erase() on end iterator");
  BOOST_ASSERT(pos.m_page);
  BOOST_ASSERT(pos.m_page->is_leaf());
  BOOST_ASSERT(&*pos.m_element < &*pos.m_page->leaf().end());
  BOOST_ASSERT(&*pos.m_element >= &*pos.m_page->leaf().begin());

  m_ok_to_pack = false;  // TODO: is this too conservative?

  if (pos.m_page->page_id() != m_root->page_id()  // not root?
    && (pos.m_page->size() == dynamic_size(*pos.m_page->leaf().begin())))  // only 1 value on page?
  {
    // erase a single value leaf page that is not the root
    
    // establish the parent chain back to the root; the parent chain must be established
    // since the page may have been reached by iteration, and also because any existing
    // parent chain might have been invalidated by inserts or erases of common ancestors.

    iterator low = m_lower_page_bound(key(*pos));
    BOOST_ASSERT(low.m_page->page_id() == pos.m_page->page_id());
    BOOST_ASSERT(low.m_element == pos.m_element);

    low.m_page->needs_write(true);

    BOOST_ASSERT(low.m_page->parent()->page_id() \
      == low.m_page->parent_page_id()); // max_cache_size logic OK?
    m_erase_branch_value(low.m_page->parent(), low.m_page->parent_element(), pos.m_page->page_id());

    m_hdr.decrement_element_count();

    ++pos;  // increment iterator to be returned before killing the page
    BOOST_ASSERT(pos.m_page != low.m_page);  // logic check: ++pos moved to next page

    // cut the page out of the page sequence list
    btree_page_ptr link_pg;
    if (low.m_page->leaf().prior_page_id())
    {
      link_pg = m_mgr.read(low.m_page->leaf().prior_page_id()); // prior page
      link_pg->leaf().next_page_id(low.m_page->leaf().next_page_id());
      link_pg->needs_write(true);
    }
    else
      m_hdr.first_page_id(low.m_page->leaf().next_page_id());
    if (low.m_page->leaf().next_page_id())
    {
      link_pg = m_mgr.read(low.m_page->leaf().next_page_id()); // next page
      link_pg->leaf().prior_page_id(low.m_page->leaf().prior_page_id());
      link_pg->needs_write(true);
    }
    else
      m_hdr.last_page_id(low.m_page->leaf().prior_page_id());

    m_free_page(low.m_page.get());  // add page to free page list

    return pos;
  }

  // erase an element from a leaf with multiple elements or erase the only element
  // on a leaf that is also the root; these use the same logic because they do not remove
  // the page from the tree.

  value_type* erase_point = &*pos.m_element;
  std::size_t erase_sz = dynamic_size(*erase_point);
  std::size_t move_sz = char_ptr(&*pos.m_page->leaf().end())
    - (char_ptr(erase_point) + erase_sz); 
  std::memmove(erase_point, char_ptr(erase_point) + erase_sz, move_sz);
  pos.m_page->size(pos.m_page->size() - erase_sz);
  std::memset(&*pos.m_page->leaf().end(), 0, erase_sz);
  m_hdr.decrement_element_count();
  pos.m_page->needs_write(true);

  if (erase_point == &*pos.m_page->leaf().end())  // was the element before the old end()
                                                  // just erased?
  {
    if (pos.m_page->empty())  // is the page empty, implying the page is an empty root?
    {
      BOOST_ASSERT(empty());  // "erase a single value leaf page that is not the root"
                              // logic as start of function should have taken care of
                              // cases that don't empty the tree
      return end();
    }

    --pos.m_element;  // make pos incrementable so ++pos can be used to advance to next page
    ++pos;            // advance to first element on next page
  }
  return pos; 
}

//------------------------------ m_erase_branch_value() --------------------------------//

template <class Key, class Base, class Traits, class Comp>   
void vbtree_base<Key,Base,Traits,Comp>::m_erase_branch_value(
  btree_page* pg, branch_iterator element, page_id_type erasee)
{
  BOOST_ASSERT(pg->is_branch());
  BOOST_ASSERT(&*element >= &*pg->branch().begin());
  BOOST_ASSERT(&*element <= &*pg->branch().end());  // equal to end if pseudo-element only

  if (pg->empty()) // end pseudo-element only element on page?
                   // i.e. after the erase, the entire sub-tree will be empty
  {
    BOOST_ASSERT(pg->level() != header().root_level());
    BOOST_ASSERT(pg->parent()->page_id() == pg->parent_page_id()); // max_cache_size logic OK?
    m_erase_branch_value(pg->parent(),
      pg->parent_element(), pg->page_id()); // erase parent value pointing to pg
    m_free_page(pg); // move page to free page list
  }
  else
  {
    void* erase_point;
  
    if (element != pg->branch().begin() || erasee != element->page_id())
    {
      BOOST_ASSERT(erasee == (element+1)->page_id());
      erase_point = &element->key();
    }
    else
    {
      BOOST_ASSERT(erasee == element->page_id());
      erase_point = &element->page_id();
    }
    std::size_t erase_sz = dynamic_size(element->key()) + sizeof(page_id_type); 
    std::size_t move_sz = char_distance(
      char_ptr(erase_point)+erase_sz, &pg->branch().end()->key());
    std::memmove(erase_point, char_ptr(erase_point) + erase_sz, move_sz);
    pg->size(pg->size() - erase_sz);
    std::memset(char_ptr(&pg->branch().end()) + sizeof(page_id_type), 0, erase_sz);
    pg->needs_write(true);

    while (pg->level()   // not the leaf (which can happen if iteration reaches leaf)
      && pg->branch().begin() == pg->branch().end()  // page empty except for P0
      && pg->level() == header().root_level())   // page is the root
    {
      // make the end pseudo-element the new root and then free this page
      m_hdr.root_page_id(pg->branch().end()->page_id());
      m_root = m_mgr.read(header().root_page_id());
      m_hdr.decrement_root_level();
      m_free_page(pg); // move page to free page list
      pg = m_root.get();
    }
  }
}

template <class Key, class Base, class Traits, class Comp>   
typename vbtree_base<Key,Base,Traits,Comp>::size_type
vbtree_base<Key,Base,Traits,Comp>::erase(const key_type& k)
{
  BOOST_ASSERT_MSG(is_open(), "erase() on unopen btree");
  size_type count = 0;
  const_iterator it = lower_bound(k);
    
  while (it != end() && !key_comp()(k, key(*it)))
  {
    ++count;
    it = erase(it);
  }
  return count;
}

template <class Key, class Base, class Traits, class Comp>   
typename vbtree_base<Key,Base,Traits,Comp>::const_iterator 
vbtree_base<Key,Base,Traits,Comp>::erase(const_iterator first, const_iterator last)
{
  BOOST_ASSERT_MSG(is_open(), "erase() on unopen btree");
  // caution: last must be revalidated when on the same page as first
  while (first != last)
  {
    if (last != end() && first.m_page == last.m_page)
    {
      BOOST_ASSERT(first.m_element < last.m_element);
      --last;  // revalidate in anticipation of erasing a prior element on same page
    }
    first = erase(first);
  }
  return last;
}

//--------------------------------- m_insert_unique() ----------------------------------//

template <class Key, class Base, class Traits, class Comp>   
std::pair<typename vbtree_base<Key,Base,Traits,Comp>::const_iterator, bool>
vbtree_base<Key,Base,Traits,Comp>::m_insert_unique(const key_type& k,
  const mapped_type& mv)
{
  BOOST_ASSERT_MSG(is_open(), "insert() on unopen btree");
  iterator insert_point = m_lower_page_bound(k);

  bool unique = insert_point.m_element == insert_point.m_page->leaf().end()
                || key_comp()(k, key(*insert_point))
                || key_comp()(key(*insert_point), k);

  if (unique)
    return std::pair<const_iterator, bool>(m_leaf_insert(insert_point, k, mv), true);

  return std::pair<const_iterator, bool>(
    const_iterator(insert_point.m_page, insert_point.m_element), false); 
}

//-------------------------------- m_insert_non_unique() -------------------------------//

template <class Key, class Base, class Traits, class Comp>   
inline typename vbtree_base<Key,Base,Traits,Comp>::const_iterator
vbtree_base<Key,Base,Traits,Comp>::m_insert_non_unique(const key_type& key,
  const mapped_type& mapped_value)
{
  BOOST_ASSERT_MSG(is_open(), "erase() on unopen btree");
  iterator insert_point = m_upper_page_bound(key);
  return m_leaf_insert(insert_point, key, mapped_value);
}

//--------------------------------- m_lower_page_bound() -------------------------------//

//  Differs from lower_bound() in that a trail of parent page and element pointers is left
//  behind, allowing inserts and erases to walk back up the tree to maintain the branch
//  invariants. Also, m_element of the returned iterator will be m_page->leaf().end() if
//  appropriate, rather than a pointer to the first element on the next page.

//  Analysis; consider a branch page with these entries:
//
//     P0, K0="B", P1, K1="D", P2, K2="F", P3, ---
//     ----------  ----------  ----------  ------------------
//     element 0   element 1   element 2   end pseudo-element
//
//  Search key:  A  B  C  D  E  F  G
//  lower_bound  
//   element     0  0  1  1  2  2  3
//  child_pg->   0  0  1  1  2  2  3
//   parent_element
//  Child page:  P0 P1 P1 P2 P2 P3 P3
//
//  Note well: the element returned by lower_bound becomes the child_pg element,
//  but the child page for equal keys comes from the ++lower_bound element

template <class Key, class Base, class Traits, class Comp>   
typename vbtree_base<Key,Base,Traits,Comp>::iterator
vbtree_base<Key,Base,Traits,Comp>::m_lower_page_bound(const key_type& k)
// returned iterator::m_element may be the 
// past-the-end Value* for iterator::m_page
{
  btree_page_ptr pg = m_root;

  // search branches down the tree until a leaf is reached
  while (pg->is_branch())
  {
    branch_iterator low
      = std::lower_bound(pg->branch().begin(), pg->branch().end(), k, branch_comp());

    branch_iterator element(low);     // &element->key() is the insert point for
                                      // inserts and the erase point for erases

    if (low != pg->branch().end()
      && !key_comp()(k, low->key()))  // if k isn't less that low->key(), it is equal
      ++low;                          // and so must be incremented; this follows from
                                      // the branch page invariant

    // create the ephemeral child->parent list
    btree_page_ptr child_pg = m_mgr.read(low->page_id());
    child_pg->parent(pg.get());
    child_pg->parent_element(element);
#   ifndef NDEBUG
    child_pg->parent_page_id(pg->page_id());
#   endif
    pg = child_pg;
  }

  //  search leaf
  leaf_iterator low
    = std::lower_bound(pg->leaf().begin(), pg->leaf().end(), k, value_comp());

  return iterator(pg, low);
}

//---------------------------------- lower_bound() -------------------------------------//

template <class Key, class Base, class Traits, class Comp>   
typename vbtree_base<Key,Base,Traits,Comp>::const_iterator
vbtree_base<Key,Base,Traits,Comp>::lower_bound(const key_type& k) const
{
  BOOST_ASSERT_MSG(is_open(), "lower_bound() on unopen btree");
  btree_page_ptr pg = m_root;

  // search branches down the tree until a leaf is reached
  while (pg->is_branch())
  {
    branch_iterator low
      = std::lower_bound(pg->branch().begin(), pg->branch().end(), k, branch_comp());

    if ((header().flags() & btree::flags::multi) == 0
      && low != pg->branch().end()
      && !key_comp()(k, low->key())) // if k isn't less that low->key(), it is equal
      ++low;                         // and so must be incremented; this follows from
                                     // the branch page invariant
    pg = m_mgr.read(low->page_id());
  }

  //  search leaf
  leaf_iterator low
    = std::lower_bound(pg->leaf().begin(), pg->leaf().end(), k, value_comp());

  if (low != pg->leaf().end())
    return const_iterator(pg, low);

  // lower bound is first element on next page; this may happen for non-unique containers
  if (!pg->leaf().next_page_id())
    return end();
  pg = m_mgr.read(pg->leaf().next_page_id());
  return const_iterator(pg, pg->leaf().begin());
}

//--------------------------------- m_upper_page_bound() -------------------------------//

//  Differs from upper_bound() in that a trail of parent page and element pointers is left
//  behind, allowing inserts and erases to walk back up the tree to maintain the branch
//  invariants. Also, m_element of the returned iterator will be m_page->leaf().end() if
//  appropriate, rather than a pointer to the first element on the next page.

template <class Key, class Base, class Traits, class Comp>   
typename vbtree_base<Key,Base,Traits,Comp>::iterator
vbtree_base<Key,Base,Traits,Comp>::m_upper_page_bound(const key_type& k)
// returned iterator::m_element may be the 
// past-the-end Value* for iterator::m_page
{
  btree_page_ptr pg = m_root;

  // search branches down the tree until a leaf is reached
  while (pg->is_branch())
  {
    branch_iterator up
      = std::upper_bound(pg->branch().begin(), pg->branch().end(), k, branch_comp());
    //if (up == pg->branch().end()        // all keys on page < search key, so up
    //                                  // must point to last value on page
    //  || key_comp()(k, up->key))      // search key < up key, so up
    //                                  // must point to P0 pseudo-value
    //  --up;

    // create the ephemeral child->parent list
    btree_page_ptr child_pg;
    child_pg = m_mgr.read(up->page_id());
    child_pg->parent(pg.get());
    child_pg->parent_element(up);
#   ifndef NDEBUG
    child_pg->parent_page_id(pg->page_id());
#   endif
    pg = child_pg;
  }

  //  search leaf
  leaf_iterator up
    = std::upper_bound(pg->leaf().begin(), pg->leaf().end(), k, value_comp());

  return iterator(pg, up);
}

//---------------------------------- upper_bound() -------------------------------------//

template <class Key, class Base, class Traits, class Comp>   
typename vbtree_base<Key,Base,Traits,Comp>::const_iterator
vbtree_base<Key,Base,Traits,Comp>::upper_bound(const key_type& k) const
{
  BOOST_ASSERT_MSG(is_open(), "upper_bound() on unopen btree");
  btree_page_ptr pg = m_root;

  // search branches down the tree until a leaf is reached
  while (pg->is_branch())
  {
    branch_iterator up
      = std::upper_bound(pg->branch().begin(), pg->branch().end(), k, branch_comp());

    //pg = (up == pg->branch().end()        // all keys on page < search key
    //      || key_comp()(k, up->key()))    // search key < up key
    //  ? m_mgr.read(prior_page_id(up))
    //  : m_mgr.read(up->page_id);
    pg = m_mgr.read(up->page_id());
  }

  //  search leaf
  leaf_iterator up
    = std::upper_bound(pg->leaf().begin(), pg->leaf().end(), k, value_comp());

  if (up != pg->leaf().end())
    return const_iterator(pg, up);

  // upper bound is first element on next page
  if (!pg->leaf().next_page_id())
    return end();
  pg = m_mgr.read(pg->leaf().next_page_id());
  return const_iterator(pg, pg->leaf().begin());
}

//------------------------------------- find() -----------------------------------------//

template <class Key, class Base, class Traits, class Comp>   
typename vbtree_base<Key,Base,Traits,Comp>::const_iterator
vbtree_base<Key,Base,Traits,Comp>::find(const key_type& k) const
{
  BOOST_ASSERT_MSG(is_open(), "find() on unopen btree");
  const_iterator low = lower_bound(k);
  return (low != end() && !key_comp()(k, key(*low)))
    ? low
    : end();
}

//------------------------------------ count() -----------------------------------------//

template <class Key, class Base, class Traits, class Comp>   
typename vbtree_base<Key,Base,Traits,Comp>::size_type
vbtree_base<Key,Base,Traits,Comp>::count(const key_type& k) const
{
  BOOST_ASSERT_MSG(is_open(), "lower_bound() on unopen btree");
  size_type count = 0;

  for (const_iterator it = lower_bound(k);
        it != end() && !key_comp()(k, key(*it));
        ++it) { ++count; } 

  return count;
}

//----------------------------------- dump_dot -----------------------------------------//

template <class Key, class Base, class Traits, class Comp>   
void vbtree_base<Key,Base,Traits,Comp>::dump_dot(std::ostream& os) const
{
  BOOST_ASSERT_MSG(is_open(), "dump_dot() on unopen btree");
  os << "digraph btree {\n"
    "node [shape = record,height=.1];\n";

  for (unsigned int p = 1; p < header().page_count(); ++p)
  {
    btree_page_ptr pg = m_mgr.read(p);

    if (pg->is_leaf())
    {
      os << "page" << p << "[label = \"<f0> ";
      for (leaf_iterator it = pg->leaf().begin(); it != pg->leaf().end(); ++it)
      {
        if (it != pg->leaf().begin())
          os << '|';
        os << *it;
      }
      os << "\"];\n";
    }
    else if (pg->is_branch())
    {
      os << "page" << p << "[label = \"";
      int f = 0;
      branch_iterator it;
      for (it = pg->branch().begin(); it != pg->branch().end(); ++it)
      {
        os << "<f" << f << "> " << it->page_id() << "," << it->key() << "|";
        ++f;
      }
      os << "<f" << f << "> " << it->page_id();
      os << "\"];\n";
      f = 0;
      for (it = pg->branch().begin(); it != pg->branch().end(); ++it)
      {
        os << "\"page" << p << "\":f" << f << " -> \"page" << it->page_id() << "\":f0;\n";
        ++f;
      }
    os << "\"page" << p << "\":f" << f << " -> \"page" << it->page_id() << "\":f0;\n";
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
vbtree_base<Key,Base,Traits,Comp>::iterator_type<T>::increment()
{
  BOOST_ASSERT_MSG(m_element != typename vbtree_base::leaf_iterator(0),
    "increment of end iterator"); 
  BOOST_ASSERT(m_page);
  BOOST_ASSERT(&*m_element >= &*m_page->leaf().begin());
  BOOST_ASSERT(&*m_element < &*m_page->leaf().end());

  if (++m_element != m_page->leaf().end())
    return;
  if (m_page->leaf().next_page_id())
  {  
    m_page = m_page->manager().read(m_page->leaf().next_page_id());
    m_element = m_page->leaf().begin();
  }
  else // end() reached
  {
    *this = reinterpret_cast<const vbtree_base<Key,Base,Traits,Comp>*>
        (m_page->manager().owner())->m_end_iterator;
  }
}

template <class Key, class Base, class Traits, class Comp>
template <class T>
void
vbtree_base<Key,Base,Traits,Comp>::iterator_type<T>::decrement()
{
  if (*this == reinterpret_cast<const vbtree_base<Key,Base,Traits,Comp>*>
        (m_page->manager().owner())->end())
    *this = reinterpret_cast<vbtree_base<Key,Base,Traits,Comp>*>
        (m_page->manager().owner())->last();
  else if (m_element != m_page->leaf().begin())
    --m_element;
  else if (m_page->leaf().prior_page_id())
  {  
    m_page = m_page->manager().read(m_page->leaf().prior_page_id());
    m_element = m_page->leaf().end();
    --m_element;
  }
  else // end() reached
    *this = reinterpret_cast<const vbtree_base<Key,Base,Traits,Comp>*>
        (m_page->manager().owner())->m_end_iterator;
}

}  // namespace btree
}  // namespace boost

#endif // BOOST_BTREE_COMMON_HPP

