//  boost/btree/support/cstr.hpp  ------------------------------------------------------//

//  Copyright Beman Dawes 2011

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt


#ifndef BOOST_BTREE_CSTR_HPP
#define BOOST_BTREE_CSTR_HPP

#include <boost/btree/dynamic_size.hpp>
#include <cstring>

namespace boost
{ 
  namespace btree
  {

    class cstr
    {
    public:
      const char* c_str() const {return m_char;}
      std::size_t size() const {return std::strlen(m_char)+1;}
      bool operator<(const cstr& rhs) const {return std::strcmp(m_char, rhs.m_char) < 0;}
    private:
      cstr();  // poison construction
      char m_char[1];
    };

    const cstr& make_cstr(const char* s) {return *reinterpret_cast<const cstr*>(s);}

    inline std::size_t dynamic_size(const cstr& x) {return x.size();}

    template<> struct has_dynamic_size<cstr> : public true_type{};

    std::ostream& operator<<(std::ostream& os, const cstr& x)
    {
      os << x.c_str();
      return os;
    }
  }
}

#endif  // BOOST_BTREE_CSTR_HPP
