//  boost/btree/index_helpers.hpp  -----------------------------------------------------//

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_BTREE_INDEX_HELPERS_HPP
#define BOOST_BTREE_INDEX_HELPERS_HPP

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable : 4996)   // equivalent to -D_SCL_SECURE_NO_WARNINGS
#endif

#include <boost/config/warning_disable.hpp>

#include <boost/btree/detail/config.hpp>
#include <boost/btree/helpers.hpp>
#include <boost/btree/support/size_t_codec.hpp>
#include <boost/btree/support/string_view.hpp>
#include <boost/btree/mmff.hpp>
#include <boost/assert.hpp>
#include <cstring>

namespace boost
{
namespace btree
{

  typedef boost::btree::extendible_mapped_file  flat_file_type;

//--------------------------------------------------------------------------------------//
//                                   index traits                                       //
//--------------------------------------------------------------------------------------//

//  index_serialize should be customized for variable length UDTs by specialization
//  rather than by overload to eliminate the silent  error of a user writing 
//  index_serialize<MyType>(...) by mistake and getting the primary template instead
//  of a MyType specialization.

//------------------  defaults for all fixed length data types  ------------------------//

  template <class T>
  struct index_reference { typedef const T&  type; };

  template <class T>
  inline void index_serialize(const T& x, flat_file_type& file)
  { 
    flat_file_type::position_type pos = file.file_size();
    file.increment_file_size(sizeof(T));  // will resize if needed
    std::memcpy(file.template data<char>() + pos,
      reinterpret_cast<const char*>(&x), sizeof(T));
  }

  template <class T>
  inline typename index_reference<T>::type index_deserialize(const char** flat)
  {
    BOOST_ASSERT(flat);
    BOOST_ASSERT(*flat);
    const char* p = *flat;
    *flat += sizeof(T);
    return *reinterpret_cast<const T*>(p);
  }

  //------------------  string_view (i.e. C++ style string) traits  --------------------//

  template <>
  struct index_reference<boost::string_view>
    { typedef const boost::string_view  type; };

  template <>
  inline void index_serialize<boost::string_view>(const boost::string_view& sv,
    flat_file_type& file)
  { 
    typedef btree::support::size_t_codec codec;
    std::size_t size_sz = codec::encoded_size(sv.size());
    flat_file_type::position_type pos = file.file_size();
    file.increment_file_size(size_sz + sv.size());  // will resize if needed
    codec::encode(sv.size(), file.data<char>() + pos, size_sz);
    pos += size_sz;
    std::memcpy(file.data<char>() + pos, sv.data(), sv.size());
  }

  template <>
  inline index_reference<boost::string_view>::type
    index_deserialize<boost::string_view>(const char** flat)
  {
    typedef btree::support::size_t_codec codec;
    BOOST_ASSERT(flat);
    BOOST_ASSERT(*flat);
    std::pair<std::size_t, std::size_t> dec = codec::decode(*flat);
    *flat += dec.second;  // size of the size prefix
    const char* p = *flat;
    *flat += dec.first;   // size of the string
    return boost::string_view(p, dec.first);
  }

}  // namespace btree
}  // namespace boost

#ifdef _MSC_VER
#  pragma warning(pop) 
#endif

#endif // BOOST_BTREE_INDEX_HELPERS_HPP
