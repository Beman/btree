//  boost/btree/detail/test_traits.hpp  ------------------------------------------------//

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_BTREE_DETAIL_TEST_TRAITS_HPP
#define BOOST_BTREE_DETAIL_TEST_TRAITS_HPP

#include <boost/btree/btree_map.hpp>
#include <boost/btree/btree_set.hpp>
#include <boost/btree/btree_set_index.hpp>
#include <boost/random.hpp>
#include <boost/btree/support/random_string.hpp>
#include <cstdint>
#include <string>
#include <utility>
#include <map>
#include <SET>


namespace boost { namespace btree { namespace detail {

  //------------------------------------------------------------------------------------//
  //                                                                                    //
  //  Abstract away differences between btree and std types, between sets and maps,     //
  //  between trees and indexes, and between generator types.                           //
  //                                                                                    //
  //------------------------------------------------------------------------------------//

  class map_64_64
  {
    mt19937_64  m_rng;
    uniform_int<int64_t>  m_dist;
    variate_generator<mt19937_64&, uniform_int<int64_t> >  m_key;
  public:
    typedef boost::btree::btree_map<int64_t, int64_t>  btree_type;
    typedef btree_type::key_type                       btree_key_type;
    typedef btree_type::value_type                     btree_value_type;
    typedef std::map<int64_t, int64_t>                 stl_type;
    typedef stl_type::key_type                         stl_key_type;
    typedef stl_type::value_type                       stl_value_type;

    map_64_64(int64_t n) : m_dist(0, n-1), m_key(m_rng, m_dist) {}

    void              seed(int64_t seed_)           {m_rng.seed(seed_);}
    btree_key_type    key()                         {return m_key();}
    btree_value_type  value(int64_t mapped_value)   {return std::make_pair(m_key(),
                                                       mapped_value);}

    stl_key_type    stl_key(const btree_value_type& v)   {return v.first;}
    stl_value_type  stl_value(const btree_value_type& v) {return v;}
  };

  //class set_64_generator
  //{
  //  mt19937_64  m_rng;
  //  uniform_int<int64_t>  m_dist;
  //  variate_generator<mt19937_64&, uniform_int<int64_t> >  m_key;
  //public:
  //  typedef std::set<int64_t>  stl_type;

  //  set_64_generator(int64_t n) : m_dist(0, n-1), m_key(m_rng, m_dist) {}

  //  void     seed(int64_t seed_)           {m_rng.seed(seed_);}
  //  int64_t  key()                         {return m_key();}
  //  int64_t  value(int64_t)                {return m_key();}

  //  static int64_t stl_key(int64_t vt)                 {return vt;}
  //  static stl_type::value_type stl_value(int64_t vt)  {return vt;}
  //};

  //class set_index_64_generator
  //{
  //  mt19937_64  m_rng;
  //  uniform_int<int64_t>  m_dist;
  //  variate_generator<mt19937_64&, uniform_int<int64_t> >  m_key;
  //public:
  //  typedef std::set<int64_t>  stl_type;

  //  set_index_64_generator(int64_t n) : m_dist(0, n-1), m_key(m_rng, m_dist) {}

  //  void     seed(int64_t seed_)           {m_rng.seed(seed_);}
  //  int64_t  key()                         {return m_key();}
  //  int64_t  value(int64_t)                {return m_key();}

  //  static int64_t stl_key(int64_t vt)                 {return vt;}
  //  static stl_type::value_type stl_value(int64_t vt)  {return vt;}
  //};

  class set_index_string_view
  {
    boost::random_string m_rsg;
    std::string m_string;
  public:
    typedef boost::btree::
       btree_set_index<boost::string_view>  btree_type;
    typedef std::set<std::string>           stl_type;
                                           
    typedef btree_type::key_type            btree_key_type;
    typedef btree_type::value_type          btree_value_type;
    typedef btree_type::value_type          btree_mapped_type;
    typedef stl_type::key_type              stl_key_type;
    typedef stl_type::value_type            stl_value_type;
    typedef stl_type::value_type            stl_mapped_type;

    set_index_string_view(int64_t) 
      : m_rsg(0, 512, 'a', 'z') {}

    void              seed(int64_t seed_)  {m_rsg.seed(static_cast<int>(seed_));}

    btree_key_type    generate_btree_key() {m_string = m_rsg();
                                            return boost::string_view(m_string);}
    btree_value_type  generate_btree_value(int64_t) {return generate_btree_key();}

    btree_key_type    btree_key(const btree_value_type& v) const
                                           {return v;}
    btree_key_type    btree_mapped(const btree_value_type& v) const
                                           {return v;}

    stl_value_type    stl_value(const btree_value_type& v) const
                                           {return stl_value_type(v.data(), v.size());}
    stl_key_type      stl_key(const stl_value_type& v) const
                                           {return v;}
    stl_mapped_type   stl_mapped(const stl_value_type& v) const
                                           {return v;}
  };

}}}  // namespaces

#endif  // BOOST_BTREE_DETAIL_TEST_TRAITS_HPP
