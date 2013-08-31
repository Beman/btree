//  example/udt_index_set.cpp

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

///$id code=
#include <boost/btree/btree_index_set.hpp>
#include <boost/btree/support/string_view.hpp>
#include <iostream>
#include <boost/detail/lightweight_main.hpp> 

using std::cout;
using namespace boost::btree;
using boost::string_view;

struct udt
{
  typedef int id_type;
  id_type      id;
  string_view  english;
  string_view  spanish;

  udt() {}
  udt(id_type n, string_view en, string_view sp)
    : id(n), english(en), spanish(sp) {} 

  bool operator < (const udt& rhs) const { return english < rhs.english; }
};

inline std::ostream& operator<<(std::ostream& os, const udt& x)
{
  os << x.id << " \"" << x.english << "\" \"" << x.spanish << "\"\n";
  return os;
}

namespace boost { namespace btree {

template <>
class default_index_traits<udt>
{
  typedef btree::support::size_t_codec codec;

public:
  typedef udt                        reference;             // proxy
  typedef endian::big_uint32un_t     index_position_type;   // 32-bits is plenty

  static std::size_t size(const udt& x)
  {
    return sizeof(x.id) + default_index_traits<boost::string_view>::size(x.english)
      + default_index_traits<boost::string_view>::size(x.spanish);
  }

  static void build_flat_element(const udt& x, char* dest,
    std::size_t)
  {
    BOOST_ASSERT(dest);
    std::memcpy(dest, &x.id, sizeof(x.id));
    dest += sizeof(x.id);
    std::size_t sz = default_index_traits<boost::string_view>::size(x.english);
    default_index_traits<boost::string_view>::build_flat_element(x.english, dest, sz);
    dest += sz;
    default_index_traits<boost::string_view>::build_flat_element(x.spanish, dest,
      default_index_traits<boost::string_view>::size(x.spanish));
  }

  static reference dereference(const char* x)
  {
    return udt
      (default_index_traits<int>::dereference(x),
       default_index_traits<boost::string_view>::dereference(x + sizeof(udt::id_type)),
       default_index_traits<boost::string_view>::dereference(x + sizeof(udt::id_type)
         + default_index_traits<boost::string_view>::size(x + sizeof(udt::id_type)))
      );
  }
  static std::size_t size(const char* x)
  {
    std::size_t sz = sizeof(udt::id_type);
    sz += default_index_traits<boost::string_view>::size(x + sz);  // english
    sz += default_index_traits<boost::string_view>::size(x + sz);  // spanish
    return sz;
  }
};

}} // namespaces


int cpp_main(int, char *[])
{
  typedef btree_index_set<udt> BT;
  BT bt("udt_index_set", flags::truncate);

  bt.insert(udt(1, "eat", "comer"));
  bt.insert(udt(2, "drink", "beber"));
  bt.insert(udt(3, "be merry", "ser feliz"));

  for (BT::iterator itr = bt.begin(); itr != bt.end(); ++itr)
    cout << *itr << '\n';

  return 0;    // required
}
///$endid
