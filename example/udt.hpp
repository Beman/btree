//  example/udt.hpp

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

///$id code=
#ifndef BOOST_BTREE_UDT_EXAMPLE_HPP
#define BOOST_BTREE_UDT_EXAMPLE_HPP

#include <boost/btree/index_helpers.hpp>
#include <boost/btree/support/string_view.hpp>
#include <iosfwd>

using std::cout;
using namespace boost::btree;
using boost::string_view;

//  The actual UDT  ----------------------------------------------------------//

struct UDT
{
  typedef int id_type;

  id_type      id;
  string_view  english;
  string_view  spanish;

  UDT() {}
  UDT(id_type n, string_view en, string_view sp)
    : id(n), english(en), spanish(sp)
  {}

  bool operator < (const UDT& rhs) const { return english < rhs.english; }
};

//  stream inserter  ---------------------------------------------------------//

inline std::ostream& operator<<(std::ostream& os, const UDT& x)
{
  os << x.id << " \"" << x.english << "\" \"" << x.spanish << "\"";
  return os;
}

//  function objects for different orderings  --------------------------------//

struct id_ordering
{
  bool operator()(const UDT& x, const UDT& y) const
    {return x.id < y.id;}
};

struct spanish_ordering
{
  bool operator()(const UDT& x, const UDT& y) const
    {return x.spanish < y.spanish;}
};

//  specializations to support btree indexes  --------------------------------//

namespace boost
{
namespace btree
{
  template <> struct index_reference<UDT> { typedef const UDT type; };

  template <>
  inline void index_serialize<UDT>(const UDT& udt, flat_file_type& file)
  {
    index_serialize(udt.id, file);
    index_serialize(udt.english, file);
    index_serialize(udt.spanish, file);
  }

  template <>
  inline index_reference<UDT>::type index_deserialize<UDT>(const char** flat)
  {
    UDT udt;
    udt.id = index_deserialize<UDT::id_type>(flat);
    udt.english = index_deserialize<boost::string_view>(flat);
    udt.spanish = index_deserialize<boost::string_view>(flat);
    return udt;
  }
}} // namespaces

#endif
///$endid
