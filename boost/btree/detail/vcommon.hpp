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
#include <ostream>
#include <stdexcept>

namespace boost
{
namespace btree
{

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                   Helpers for variable length keys and/or data                       //
//                                                                                      //
//          Use cases include vbtree_set<char*> and vbtree_map<char*, char*>            //
//                                                                                      //
//--------------------------------------------------------------------------------------//

//  See http://www.gotw.ca/publications/mill17.htm,
//  Why Not Specialize: The Dimov/Abrahams Example

template <class T>
std::size_t size(const T&) { return sizeof(T); }

template <class T>
std::size_t size(const T*);  // pointers must be overloaded; if this overload is selected
                             // it means user failed to provide the required overload

std::size_t size(const char* s) { return std::strlen(s) + 1; }
std::size_t size(char* s)       { return std::strlen(s) + 1; }

//  less function object class

template <class T> struct less
{
  typedef T first_argument_type;
  typedef T second_argument_type;
  typedef bool result_type;
  bool operator()(const T& x, const T& y) const { return x < y; }
};

//  partial specialization to poison pointer that hasn't been fully specialized
template <class T> struct less<const T*>
{
  typedef T first_argument_type;
  typedef T second_argument_type;
  typedef bool result_type;
  bool operator()(const T& x, const T& y) const;
};

//  full specialization for C strings
template <> struct less<char*>
{
  typedef const char* first_argument_type;
  typedef const char* second_argument_type;
  typedef bool result_type;
  bool operator()(const char* x, const char* y) const { return std::strcmp(x, y) < 0; }
};

namespace detail
{
  //-------------------------------- dynamic_iterator ----------------------------------//
  //
  //  Pointers to elements on a page aren't useful as iterators because the elements are
  //  variable length. dynamic_iterator provides the correct semantics.

  template <class T>
  class dynamic_iterator
    : public boost::iterator_facade<dynamic_iterator<T>, T, forward_traversal_tag>
  {
  public:
    dynamic_iterator() : m_ptr(0) {} 
    explicit dynamic_iterator(T* ptr) : m_ptr(ptr) {}
    dynamic_iterator(T* ptr, std::size_t sz)
      : m_ptr(reinterpret_cast<T*>(reinterpret_cast<char*>(m_ptr) + sz)) {}

  private:
    friend class boost::iterator_core_access;

    T* m_ptr;

    T& dereference() const  { return *m_ptr; }
 
    bool equal(const dynamic_iterator& rhs) const { return m_ptr == rhs.ptr; }

    void increment()
    {
      m_ptr = reinterpret_cast<T*>(reinterpret_cast<char*>(m_ptr) + btree::size(m_ptr));
    }
  };

  //-------------------------------- dynamic_pair --------------------------------------//

  // TODO: either add code to align second() or add a requirement that T2 is an unaligned
  // type.


  template <class T1, class T2>
  class dynamic_pair
  {
  public:
    T1*       first()         { return reinterpret_cast<T1*>(this); }
    const T1* first() const   { return reinterpret_cast<const T1*>(this); }
    T2*       second()        { return reinterpret_cast<T2*>(reinterpret_cast<char*>(this)
                                  + size(first())); }
    T2*       second() const  { return reinterpret_cast<const T2*>(
                                  reinterpret_cast<const char*>(this) + size(first())); }
  };

}  // namespace detail

//--------------------------------------------------------------------------------------//
//                             class vbtree_set_base                                    //
//--------------------------------------------------------------------------------------//

template <class Key, class Comp>
class vbtree_set_base
{
public:
  typedef typename boost::remove_pointer<Key>::type const *  value_type;
  typedef value_type const                                   const_value_type;
  typedef Comp                                               value_compare;
  typedef typename boost::remove_pointer<Key>::type          element_type;

  const Key& key(const value_type& v) const {return v;}  // really handy, so expose

  static std::size_t key_size() { return -1; }
  static std::size_t mapped_size() { return -1; }

protected:
  static void set_value(value_type& target, const void* source)
  {
    target = reinterpret_cast<value_type>(source);
  }
};

//--------------------------------------------------------------------------------------//
//                             class vbtree_map_base                                    //
//--------------------------------------------------------------------------------------//

template <class Key, class T, class Comp>
class vbtree_map_base
{
public:
  typedef typename boost::remove_pointer<Key>::type const * const_key_type;
  typedef std::pair<typename boost::remove_pointer<Key>::type const *, T>
    value_type;
  typedef std::pair<typename boost::remove_pointer<Key>::type const *,
                    typename boost::remove_pointer<T>::type const *>
    const_value_type;

  typedef detail::dynamic_pair<typename boost::remove_pointer<Key>::type,
                       typename boost::remove_pointer<T>::type>
    element_type;

  const const_key_type& key(const value_type& v) const  // really handy, so expose
    {return v.first;}

  static std::size_t key_size() { return -1; }
  static std::size_t mapped_size() { return -1; }

  class value_compare
  {
  public:
    value_compare() {}
    value_compare(Comp comp) : m_comp(comp) {}
    bool operator()(const value_type& x, const value_type& y) const
      { return m_comp(x.first, y.first); }
    bool operator()(const value_type& x, const Key& y) const
      { return m_comp(x.first, y); }
    bool operator()(const Key& x, const value_type& y) const
      { return m_comp(x, y.first); }
  private:
    Comp    m_comp;
  };

protected:
  static void set_value(value_type& target, const void* source)
  {
    target.first  = reinterpret_cast<Key>(source);
    target.second = reinterpret_cast<T>(source + btree::size(target.first));
  }
};

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
  class branch_value_type;
  template <class T>
    class iterator_type;

//--------------------------------------------------------------------------------------//
//                                public interface                                      //
//--------------------------------------------------------------------------------------//

public:
  // types:
  typedef Key                                   key_type;
  typedef typename Base::value_type             value_type;
  typedef typename Base::const_value_type       const_value_type;
  typedef Comp                                  key_compare;
  typedef typename Base::value_compare          value_compare; 
  typedef value_type&                           reference;
  typedef const const_value_type&               const_reference;
  typedef boost::uint64_t                       size_type;
  typedef value_type*                           pointer;
  typedef const const_value_type*               const_pointer;

  typedef iterator_type<const value_type>       iterator;
  typedef iterator_type<const const_value_type> const_iterator;

  typedef std::reverse_iterator<iterator>       reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
  typedef std::pair<const_iterator, const_iterator>
                                                const_iterator_range;

  typedef typename Traits::page_id_type         page_id_type;
  typedef typename Traits::page_size_type       page_size_type;
  typedef typename Traits::page_level_type      page_level_type;

  typedef typename Base::element_type           leaf_element_type;

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
    int              level() const         {return m_level;}
    void             level(int lv)         {m_level = lv;}
    bool             is_leaf() const       {return m_level == 0;}
    bool             is_branch() const     {return m_level != 0;}
    std::size_t      size() const          {return m_size;}  // std::size_t is correct!
    void             size(std::size_t sz)  {m_size = sz;}    // ditto

  private:
    page_level_type  m_level;        // leaf: 0, branches: distance from leaf,
                                     // header: 0xFFFF, free page list entry: 0xFFFE
    page_size_type   m_size;         // size in bytes of elements on page
  };
  
  //------------------------ leaf data formats and operations --------------------------//

  class leaf_data : public btree_data
  {
  public:
    typedef detail::dynamic_iterator<leaf_element_type>  leaf_element_iterator;

    page_id_type           prior_page_id() const           {return m_prior_page_id;}
    void                   prior_page_id(page_id_type id)  {m_prior_page_id = id;}
    page_id_type           next_page_id() const            {return m_next_page_id;}
    void                   next_page_id(page_id_type id)   {m_next_page_id = id;}
    leaf_element_iterator  begin()      { return leaf_element_iterator(m_value);}
    leaf_element_iterator  end()        { return leaf_element_iterator(m_value, size()); }

    //  private:
    page_id_type       m_next_page_id;   // page sequence fwd list; 0 for end
    page_id_type       m_prior_page_id;  // page sequence bwd list; 0 for end
    leaf_element_type  m_element[1];
  };

  //------------------------ branch data formats and operations ------------------------//

  struct branch_value_type
  {
    Key           key;
    page_id_type  page_id;
  };

  class branch_data : public btree_data
  {
  public:
    branch_value_type* begin()                       {return m_value;}
    branch_value_type* end()                         {return &m_value[size()];}

//  private:
    page_id_type P0;  // the initial pseudo-element, which has no key;
                      // this is often called P sub 0 in the literature
    branch_value_type  m_value[1];
  };

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
    btree_page*        parent()                        {return m_parent;}
    void               parent(btree_page* p)           {m_parent = p;}
    branch_value_type* parent_element()                {return m_parent_element;}
    void               parent_element(branch_value_type* p) {m_parent_element = p;}
#   ifndef NDEBUG
    page_id_type       parent_page_id()                {return m_parent_page_id;}
    void               parent_page_id(page_id_type id) {m_parent_page_id = id;}
#   endif

    leaf_data&         leaf()       {return *reinterpret_cast<leaf_data*>(buffer::data());}
    //const leaf_data&   leaf() const {return *reinterpret_cast<const leaf_data*>(buffer::data());}
    branch_data&       branch()     {return *reinterpret_cast<branch_data*>(buffer::data());}
    //const branch_data& branch()const{return *reinterpret_cast<const branch_data*>(buffer::data());}

  private:
    btree_page*         m_parent;   // by definition, the parent is a branch page.
                                    // rationale for raw pointer: (1) elminate overhead
                                    // of revisiting pages to do btree_page_ptr::reset()
                                    // (2) allows single m_upper/m_lower_page search
                                    // function to cover both modifying and non-modifying
                                    // uses.
    branch_value_type*  m_parent_element;
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
    : public boost::iterator_facade<iterator_type<T>, T, forward_traversal_tag>
  {

    BOOST_STATIC_ASSERT_MSG(boost::is_const<T>::value,
      "Internal logic error: T must be const since iterators reference a proxy"
      " inside the iterator itself");

  public:
    iterator_type(): m_element(0) {}
    iterator_type(buffer_ptr p,
      typename vbtree_base::leaf_data::leaf_element_iterator e)
      : m_page(static_cast<typename vbtree_base::btree_page_ptr>(p)),
        m_element(e) { set_value(); }

  private:
    iterator_type(buffer_ptr p)  // used solely to setup the end iterator
      : m_page(static_cast<typename vbtree_base::btree_page_ptr>(p)),
        m_element(0) {}

    friend class boost::iterator_core_access;
    friend class vbtree_base;
   
    typename vbtree_base::btree_page_ptr                    m_page; 
    typename vbtree_base::leaf_data::leaf_element_iterator  m_element;  // 0 for end iterator
    typename boost::remove_const<T>::type                   m_value;    // proxy value

    void set_value()
    { 
      vbtree_base::set_value(m_value, m_element);
    }

    T& dereference() const  { return m_value; }
 
    bool equal(const iterator_type& rhs) const
    {
      return m_element == rhs.m_element
        // check page_id() in case page memory has been reused
        && m_page->page_id() == rhs.m_page->page_id();
    }

    void increment();
    //void decrement();
  };

//--------------------------------------------------------------------------------------//
//                             protected member functions                               //
//--------------------------------------------------------------------------------------//
protected:

  std::pair<const_iterator, bool>
    m_insert_unique(const value_type& value);
  const_iterator
    m_insert_non_unique(const value_type& value);
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
  // past-the-end value_type* for iterator::m_page
  // postcondition: parent pointers are set, all the way up the chain to the root
  iterator m_upper_page_bound(const key_type& k);
  // returned iterator::m_element is the insertion point, and thus may be the 
  // past-the-end value_type* for iterator::m_page
  // postcondition: parent pointers are set, all the way up the chain to the root
  btree_page_ptr m_new_page(boost::uint16_t lv);
  void  m_new_root();
  const_iterator m_leaf_insert(iterator insert_iter, const value_type& value);
  void  m_branch_insert(btree_page* pg, branch_value_type* element,
    const key_type& k, page_id_type id);

  struct branch_value_type;
  void  m_erase_branch_value(btree_page* pg, branch_value_type* value);
  void  m_free_page(btree_page* pg)
  {
    pg->needs_write(true);
    pg->level(0xFFFE);
    pg->size(0);
    pg->prior_page_id(page_id_type());
    pg->next_page_id(page_id_type(m_hdr.free_page_list_head_id()));
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
      {return m_comp(x.key, y.key);}
   bool operator()(const Key& x, const branch_value_type& y) const
      {return m_comp(x, y.key);}
   bool operator()(const branch_value_type& x, const Key& y) const
      {return m_comp(x.key, y);}
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
  BOOST_ASSERT(pg_sz / sizeof(value_type) >= 3);
  BOOST_ASSERT(pg_sz / sizeof(branch_value_type) >= 3);

  oflag::bitmask open_flags = oflag::in;
  if (flgs & flags::read_write)
    open_flags |= oflag::out;
  if (flgs & flags::truncate)
    open_flags |= oflag::out | oflag::truncate;
  if (flgs & flags::preload)
    open_flags |= oflag::preload;

  m_read_only = (open_flags & oflag::out) == 0;
  m_ok_to_pack = true;

  if (m_mgr.open(p, open_flags, btree::default_max_cache_pages, pg_sz))
  { // existing non-truncated file
    m_read_header();
    if (!m_hdr.marker_ok())
      BOOST_BTREE_THROW(std::runtime_error(file_path().string()+" isn't a btree"));
    if (m_hdr.big_endian() != (Traits::header_endianness == integer::endianness::big))
      BOOST_BTREE_THROW(std::runtime_error(file_path().string()+" has wrong endianness"));
    if (m_hdr.key_size() != Base::key_size())
      BOOST_BTREE_THROW(std::runtime_error(file_path().string()+" has wrong key_type size"));
    if (m_hdr.mapped_size() != Base::mapped_size())
      BOOST_BTREE_THROW(std::runtime_error(file_path().string()+" has wrong mapped_type size"));
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
    m_root->leaf().level(0);
    m_root->leaf().size(0);
    m_root->leaf().prior_page_id(0);
    m_root->leaf().next_page_id(0);
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
    m_hdr.free_page_list_head_id(pg->next_page_id());
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
  std::memcpy(&(m_root->branch_begin()-1)->page_id, &old_root_id, sizeof(page_id_type));
  m_root->size(0);  // the P0 pseudo-element doesn't count as an element
  // TODO: why maintain the child->parent list? By the time m_new_root() is called,
  // hasn't the need passed?
  m_root->parent(0);  
  m_root->parent_element(0);
  old_root->parent(m_root.get());
  old_root->parent_element(m_root->branch_begin()-1);  // point to P0 pseudo-element
# ifndef NDEBUG
  m_root->parent_page_id(page_id_type(0));
  old_root->parent_page_id(m_root->page_id());
# endif
}

//---------------------------------- m_leaf_insert() -----------------------------------//

template <class Key, class Base, class Traits, class Comp>   
typename vbtree_base<Key,Base,Traits,Comp>::const_iterator
vbtree_base<Key,Base,Traits,Comp>::m_leaf_insert(iterator insert_iter,
  const value_type& value)
{
  btree_page_ptr     pg = insert_iter.m_page;
  value_type*        insert_begin = const_cast<value_type*>(insert_iter.m_element);
  btree_page_ptr     pg2;
  value_type*        split_begin;

  BOOST_ASSERT(pg->is_leaf());
  BOOST_ASSERT(pg->size() <= m_max_leaf_size);

  m_hdr.increment_element_count();
  pg->needs_write(true);

  if (pg->size() == m_max_leaf_size)  // is the page full?
  {
    //  page is full, so must be split

    if (pg->level() == m_hdr.root_level()) // splitting the root?
      m_new_root();  // create a new root
    
    pg2 = m_new_page(pg->level());  // create the new page 

    // ck pack conditions now, since leaf seq list update may chg header().last_page_id()
    if (m_ok_to_pack
        && (insert_begin != pg->leaf().end() || pg->page_id() != header().last_page_id()))
      m_ok_to_pack = false;  // conditions for pack optimization not met

    // insert the new page into the leaf sequence lists
    pg2->prior_page_id(pg->page_id());
    pg2->next_page_id(pg->next_page_id());
    pg->next_page_id(pg2->page_id());
    if (pg2->next_page_id())
    {
      const btree_page_ptr next_page(m_mgr.read(pg2->next_page_id()));
      next_page->prior_page_id(pg2->page_id());
      next_page->needs_write(true);
    }
    else 
      m_hdr.last_page_id(pg2->page_id());

    // apply pack optimization if applicable
    if (m_ok_to_pack)  // have all inserts been ordered and no erases occurred?
    {
      // pack optimization: instead of splitting pg, just put value alone on pg2
      std::memcpy(pg2->leaf().begin(), &value, sizeof(value_type));  // insert value
      pg2->size(1);
      BOOST_ASSERT(pg->parent()->page_id() == pg->parent_page_id()); // max_cache_size logic OK?
      m_branch_insert(pg->parent(), pg->parent_element()+1,
        key(*pg2->leaf().begin()), pg2->page_id());
      return const_iterator(pg2, pg2->leaf().begin());
    }

    // split page pg by moving half the elements to page p2

    std::size_t split_sz = pg->size() >> 1;  // if odd, lower size goes on pg2 for speed
    BOOST_ASSERT(split_sz);
    split_begin = pg->leaf().begin() + (pg->size() - split_sz);
    std::memcpy(pg2->leaf().begin(), split_begin, split_sz * sizeof(value_type));
    pg2->size(split_sz);
//std::cout << "******leaf "<< pg->leaf().end() - split_begin << std::endl;
    std::memset(split_begin, 0,                              // zero unused space to make
      (pg->leaf().end() - split_begin) * sizeof(value_type));  // file dumps easier to read
    pg->size(pg->size() - split_sz);

    // adjust pg and insert_begin if they now fall on the new page due to the split
    if (split_begin <= insert_begin)
    {
      pg = pg2;
      insert_begin = pg2->leaf().begin() + (insert_begin - split_begin);
    }
  }

  //  insert value into pg at insert_begin
  BOOST_ASSERT(insert_begin >= pg->leaf().begin());
  BOOST_ASSERT(insert_begin <= pg->leaf().end());
  std::size_t memmove_sz = (pg->leaf().end() - insert_begin) * sizeof(value_type);
  std::memmove(insert_begin+1, insert_begin, memmove_sz);  // make room
  std::memcpy(insert_begin, &value, sizeof(value_type));   // insert value
  pg->size(pg->size()+1);

  // if there is a new page, its initial key and page_id are inserted into parent
  if (pg2)
  {
    BOOST_ASSERT(insert_iter.m_page->parent()->page_id() \
      == insert_iter.m_page->parent_page_id()); // max_cache_size logic OK?
    m_branch_insert(insert_iter.m_page->parent(), insert_iter.m_page->parent_element()+1,
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
  btree_page* pg1, branch_value_type* element, const key_type& k, page_id_type id) 
{
  btree_page*          pg = pg1;
  branch_value_type*   insert_begin = element;
  btree_page_ptr       pg2;
  branch_value_type*   split_begin;

  BOOST_ASSERT(pg->is_branch());
  BOOST_ASSERT(pg->size() <= m_max_branch_size);

  pg->needs_write(true);

  if (pg->size() == m_max_branch_size)  // is the page full?
  {
// std::cout << "******branch " << pg->page_id() << std::endl;
   //  page is full, so must be split

    if (pg->level() == m_hdr.root_level()) // splitting the root?
      m_new_root();  // create a new root
    
    pg2 = m_new_page(pg->level());  // create the new page

    // split the page
    std::size_t split_sz = pg->size() >> 1;  // if odd, lower size goes on pg2 for speed
    BOOST_ASSERT(split_sz);
    split_begin = pg->branch_begin() + (pg->size() - split_sz);

    if (split_begin == insert_begin)
    {
      pg2->branch_ref().P0 = id;
      std::memcpy(pg2->branch_begin(), split_begin,
        split_sz * sizeof(branch_value_type)); // move split values to the new page
      pg2->size(split_sz);
      std::memset(split_begin, 0,  // zero unused space to make file dumps easier to read
        (pg->branch_end() - split_begin) * sizeof(branch_value_type)); 
      pg->size(pg->size() - split_sz);
      BOOST_ASSERT(pg->parent()->page_id() == pg->parent_page_id()); // max_cache_size logic OK?
      m_branch_insert(pg->parent(), pg->parent_element()+1, k, pg2->page_id());
      return;
    }

    // TODO: insert_begin == split_begin+1 could special cased since the copy could
    // go directly to the correct location, elminating the need for the make-room
    // memmove

    pg2->branch_ref().P0 = split_begin->page_id;
    std::memcpy(pg2->branch_begin(), split_begin+1,
      (split_sz-1) * sizeof(branch_value_type)); // copy the values
    pg2->size(split_sz-1);
    BOOST_ASSERT(pg->parent()->page_id() == pg->parent_page_id()); // max_cache_size logic OK?
    m_branch_insert(pg->parent(), pg->parent_element()+1, split_begin->key,
      pg2->page_id());
    std::memset(split_begin, 0,  // zero unused space to make file dumps easier to read
      (pg->branch_end() - split_begin) * sizeof(branch_value_type)); 
    pg->size(pg->size() - split_sz);

    // adjust pg and insert_begin if they now fall on the new page due to the split
    if (split_begin < insert_begin)
    {
      pg = pg2.get();
      insert_begin = pg2->branch_begin() + (insert_begin - split_begin - 1);
    }
  }

  //  insert k, id into pg at insert_begin
  BOOST_ASSERT(insert_begin >= pg->branch_begin());
  BOOST_ASSERT(insert_begin <= pg->branch_end());
  std::size_t memmove_sz = (pg->branch_end() - insert_begin) * sizeof(branch_value_type);
  std::memmove(insert_begin+1, insert_begin, memmove_sz);  // make room
  insert_begin->key = k;       // do the
  insert_begin->page_id = id;  //  insert
  pg->size(pg->size()+1);

#ifndef NDEBUG
  if (!(m_hdr.flags() & btree::flags::multi))
  {
    key_type prev_key = 0;
    for(const branch_value_type* beg = pg->branch_begin(); beg != pg->branch_end(); ++beg)
    {
      BOOST_ASSERT(key_comp()(prev_key, beg->key));
      prev_key = beg->key;
    }
  }
#endif
}

//------------------------------ m_erase_branch_value() --------------------------------//

template <class Key, class Base, class Traits, class Comp>   
void vbtree_base<Key,Base,Traits,Comp>::m_erase_branch_value(
  btree_page* pg, branch_value_type* value)
{
  BOOST_ASSERT(pg->is_branch());
  if (pg->branch_begin() == pg->branch_end()) // P0 pseudo-value only value on page?
                                              // i.e. the entire sub-tree is now empty
  {
    BOOST_ASSERT(value < pg->branch_begin());
    BOOST_ASSERT(pg->level() != header().root_level());
    BOOST_ASSERT(pg->parent()->page_id() == pg->parent_page_id()); // max_cache_size logic OK?
    m_erase_branch_value(pg->parent(),
      pg->parent_element()); // erase parent value pointing to pg
    m_free_page(pg); // move page to free page list
  }
  else
  {
    // erase branch_value that is not on a P0-only page
    char* erase_point = (char*)value;
    if (value < pg->branch_begin()) // is value the P0 pseudo-value?
       erase_point = (char*)&value->page_id;
    std::size_t memmove_sz = (char*)pg->branch_end()
      - erase_point - sizeof(branch_value_type);
    std::memmove(erase_point, erase_point+sizeof(branch_value_type), memmove_sz);
    pg->size(pg->size()-1);
    std::memset(pg->branch_end(), 0, sizeof(branch_value_type));
    pg->needs_write(true);

    while (pg->level()   // not the leaf (which can happen if iteration reaches leaf)
      && pg->branch_begin() == pg->branch_end()  // page empty except for P0
      && pg->level() == header().root_level())   // page is the root
    {
      // make P0 the new root and then free this page
      m_hdr.root_page_id((pg->branch_begin()-1)->page_id);
      m_root = m_mgr.read(header().root_page_id());
      m_hdr.decrement_root_level();
      m_free_page(pg); // move page to free page list
      pg = m_root.get();
    }
  }
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
  BOOST_ASSERT(pos.m_element < pos.m_page->leaf().end());
  BOOST_ASSERT(pos.m_element >= pos.m_page->leaf().begin());

   m_ok_to_pack = false;

  if (pos.m_page->page_id() != m_root->page_id()  // not root?
    && (pos.m_page->size() == 1))  // only 1 value on page?
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
    m_erase_branch_value(low.m_page->parent(), low.m_page->parent_element());

    m_hdr.decrement_element_count();

    ++pos;  // increment iterator to be returned before killing the page
    BOOST_ASSERT(pos.m_page != low.m_page);  // logic check: ++pos moved to next page

    // cut the page out of the page sequence list
    btree_page_ptr link_pg;
    if (low.m_page->prior_page_id())
    {
      link_pg = m_mgr.read(low.m_page->prior_page_id()); // prior page
      link_pg->next_page_id(low.m_page->next_page_id());
      link_pg->needs_write(true);
    }
    else
      m_hdr.first_page_id(low.m_page->next_page_id());
    if (low.m_page->next_page_id())
    {
      link_pg = m_mgr.read(low.m_page->next_page_id()); // next page
      link_pg->prior_page_id(low.m_page->prior_page_id());
      link_pg->needs_write(true);
    }
    else
      m_hdr.last_page_id(low.m_page->prior_page_id());

    m_free_page(low.m_page.get());  // add page to free page list

    return pos;
  }

  // erase an element from a leaf with multiple elements or erase the only element
  // on a leaf that is also the root; these use the same logic because they do not remove
  // the page from the tree.

  value_type* erase_point = const_cast<value_type*>(pos.m_element);
  std::size_t memmove_sz = (pos.m_page->leaf().end() - erase_point - 1) * sizeof(value_type);
  std::memmove(erase_point, (const char*)erase_point + sizeof(value_type), memmove_sz);
  pos.m_page->size(pos.m_page->size()-1);
  std::memset(pos.m_page->leaf().end(), 0, sizeof(value_type));
  m_hdr.decrement_element_count();
  pos.m_page->needs_write(true);

  if (!memmove_sz)  // at end of page?
  {
    --pos.m_element;  // make pos incrementable so ++pos can be used to advance to next
    if (pos.m_element < pos.m_page->leaf().begin()) // is page empty?
    {
      BOOST_ASSERT(empty());  // logic check: the page is empty, implying page is an empty
                              // root; i.e. the tree is empty
      return end();
    }
    ++pos;
  }
  return pos; 
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
vbtree_base<Key,Base,Traits,Comp>::m_insert_unique(const value_type& value)
{
  BOOST_ASSERT_MSG(is_open(), "insert() on unopen btree");
  iterator insert_point = m_lower_page_bound(key(value));

  bool unique = insert_point.m_element == insert_point.m_page->leaf().end()
                || key_comp()(key(value), key(*insert_point))
                || key_comp()(key(*insert_point), key(value));

  if (unique)
    return std::pair<const_iterator, bool>(m_leaf_insert(insert_point, value), true);

  return std::pair<const_iterator, bool>(
    const_iterator(insert_point.m_page, insert_point.m_element), false); 
}

//-------------------------------- m_insert_non_unique() -------------------------------//

template <class Key, class Base, class Traits, class Comp>   
inline typename vbtree_base<Key,Base,Traits,Comp>::const_iterator
vbtree_base<Key,Base,Traits,Comp>::m_insert_non_unique(const value_type& value)
{
  BOOST_ASSERT_MSG(is_open(), "erase() on unopen btree");
  iterator insert_point = m_upper_page_bound(key(value));
  return m_leaf_insert(insert_point, value);
}

//--------------------------------- m_lower_page_bound() -------------------------------//

//  Differs from lower_bound() in that a trail of parent page and element pointers is left
//  behind, allowing inserts and erases to walk back up the tree to maintain the branch
//  invariants. Also, m_element of the returned iterator will be m_page->leaf().end() if
//  appropriate, rather than a pointer to the first element on the next page.

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
    branch_value_type* low
      = std::lower_bound(pg->branch_begin(), pg->branch_end(), k, branch_comp());
    if (low == pg->branch_end()        // all keys on page < search key, so low
                                       // must point to last value on page
      || key_comp()(k, low->key))      // search key < low key, so low
                                       // must point to P0 pseudo-value
      --low;
    btree_page_ptr child_pg;
    // create the ephemeral child->parent list
    child_pg = m_mgr.read(low->page_id);
    child_pg->parent(pg.get());
    child_pg->parent_element(low);
#   ifndef NDEBUG
    child_pg->parent_page_id(pg->page_id());
#   endif
    pg = child_pg;
  }

  //  search leaf
  value_type* low
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
    branch_value_type* low
      = std::lower_bound(pg->branch_begin(), pg->branch_end(), k, branch_comp());

    pg = (low == pg->branch_end()        // all keys on page < search key
          || key_comp()(k, low->key)   // search key < low key
          || ((m_hdr.flags() & btree::flags::multi)
              && !key_comp()(low->key, k) ))  // non-unique && search key == low key
      ? m_mgr.read((low-1)->page_id)
      : m_mgr.read(low->page_id);
  }

  //  search leaf
  value_type* low
    = std::lower_bound(pg->leaf().begin(), pg->leaf().end(), k, value_comp());

  if (low != pg->leaf().end())
    return const_iterator(pg, low);

  // lower bound is first element on next page
  if (!pg->next_page_id())
    return end();
  pg = m_mgr.read(pg->next_page_id());
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
    branch_value_type* up
      = std::upper_bound(pg->branch_begin(), pg->branch_end(), k, branch_comp());
    if (up == pg->branch_end()        // all keys on page < search key, so up
                                      // must point to last value on page
      || key_comp()(k, up->key))      // search key < up key, so up
                                      // must point to P0 pseudo-value
      --up;
    btree_page_ptr child_pg;
    // create the ephemeral child->parent list
    child_pg = m_mgr.read(up->page_id);
    child_pg->parent(pg.get());
    child_pg->parent_element(up);
#   ifndef NDEBUG
    child_pg->parent_page_id(pg->page_id());
#   endif
    pg = child_pg;
  }

  //  search leaf
  value_type* up
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
    branch_value_type* up
      = std::upper_bound(pg->branch_begin(), pg->branch_end(), k, branch_comp());

    pg = (up == pg->branch_end()        // all keys on page < search key
          || key_comp()(k, up->key))    // search key < up key
      ? m_mgr.read((up-1)->page_id)
      : m_mgr.read(up->page_id);
  }

  //  search leaf
  value_type* up
    = std::upper_bound(pg->leaf().begin(), pg->leaf().end(), k, value_comp());

  if (up != pg->leaf().end())
    return const_iterator(pg, up);

  // upper bound is first element on next page
  if (!pg->next_page_id())
    return end();
  pg = m_mgr.read(pg->next_page_id());
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
  BOOST_ASSERT_MSG(m_element, "increment of uninitialized iterator"); 
  BOOST_ASSERT(m_page);
  BOOST_ASSERT(m_element >= m_page->leaf().begin());
  BOOST_ASSERT(m_element < m_page->leaf().end());

  if (++m_element != m_page->leaf().end())
    return;
  if (m_page->next_page_id())
  {  
    m_page = m_page->manager().read(m_page->next_page_id());
    m_element = m_page->leaf().begin();
  }
  else // end() reached
  {
    *this = reinterpret_cast<const vbtree_base<Key,Base,Traits,Comp>*>
        (m_page->manager().owner())->m_end_iterator;
  }
}

//template <class Key, class Base, class Traits, class Comp>
//template <class T>
//void
//vbtree_base<Key,Base,Traits,Comp>::iterator_type<T>::decrement()
//{
//  if (*this == reinterpret_cast<const vbtree_base<Key,Base,Traits,Comp>*>
//        (m_page->manager().owner())->end())
//    *this = reinterpret_cast<vbtree_base<Key,Base,Traits,Comp>*>
//        (m_page->manager().owner())->last();
//  else if (m_element != m_page->leaf().begin())
//    --m_element;
//  else if (m_page->prior_page_id())
//  {  
//    m_page = m_page->manager().read(m_page->prior_page_id());
//    m_element = m_page->leaf().end();
//    --m_element;
//  }
//  else // end() reached
//    *this = reinterpret_cast<const vbtree_base<Key,Base,Traits,Comp>*>
//        (m_page->manager().owner())->m_end_iterator;
//}

}  // namespace btree
}  // namespace boost

#endif // BOOST_BTREE_COMMON_HPP

