//  example/udt.hpp

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

///$id code=
#ifndef BOOST_BTREE_UDT_EXAMPLE_HPP
#define BOOST_BTREE_UDT_EXAMPLE_HPP

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

//  Stream inserter  ---------------------------------------------------------//

inline std::ostream& operator<<(std::ostream& os, const UDT& x)
{
  os << x.id << " \"" << x.english << "\" \"" << x.spanish << "\"";
  return os;
}

//  Specialization boost::btree::default_index_traits<UDT>  ------------------//

namespace boost
{
  namespace btree
  {

    template <>
    class default_index_traits<UDT>
    {
      typedef btree::support::size_t_codec codec;

    public:
      typedef UDT                     reference;             // proxy
      typedef endian::big_uint32_t  index_position_type;   // big enough

      static std::size_t size(const UDT& x)
      {
        return sizeof(x.id)
          + default_index_traits<boost::string_view>::size(x.english)
          + default_index_traits<boost::string_view>::size(x.spanish);
      }

      static void build_flat_element(const UDT& x, char* dest,
        std::size_t)
      {
        BOOST_ASSERT(dest);
        std::memcpy(dest, &x.id, sizeof(x.id));
        dest += sizeof(x.id);
        std::size_t sz
          = default_index_traits<boost::string_view>::size(x.english);
        default_index_traits<boost::string_view>::build_flat_element(
          x.english, dest, sz);
        dest += sz;
        default_index_traits<boost::string_view>::build_flat_element(
          x.spanish, dest,
          default_index_traits<boost::string_view>::size(x.spanish));
      }

      static reference dereference(const char* x)
      {
        return UDT
          (default_index_traits<int>::dereference(x),
          default_index_traits<boost::string_view>::dereference(
            x + sizeof(UDT::id_type)),
          default_index_traits<boost::string_view>::dereference(
            x + sizeof(UDT::id_type)
            + default_index_traits<boost::string_view>::size(
              x + sizeof(UDT::id_type))));
      }
      static std::size_t size(const char* x)
      {
        std::size_t sz = sizeof(UDT::id_type);
        sz +=
          default_index_traits<boost::string_view>::size(x + sz);  // english
        sz +=
          default_index_traits<boost::string_view>::size(x + sz);  // spanish
        return sz;
      }
    };

  }
} // namespaces

#endif
///$endid
