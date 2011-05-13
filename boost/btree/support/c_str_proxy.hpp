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

    class c_str_proxy
    {
    public:
      const char* c_str() const {return m_char;}
      std::size_t size() const {return std::strlen(m_char)+1;}
      bool operator<(const c_str_proxy& rhs) const
        {return std::strcmp(m_char, rhs.m_char) < 0;}
    private:
      c_str_proxy();  // poison construction
      c_str_proxy(const c_str_proxy&);
      c_str_proxy& operator=(const c_str_proxy&);

      char m_char[1];
    };

    const c_str_proxy& make_c_str(const char* s)
      {return *reinterpret_cast<const c_str_proxy*>(s);}
    const c_str_proxy& make_c_str(const std::string& s)
      {return *reinterpret_cast<const c_str_proxy*>(s.c_str());}

    inline std::size_t dynamic_size(const c_str_proxy& x) {return x.size();}

    template<> struct has_dynamic_size<c_str_proxy> : public true_type{};

    std::ostream& operator<<(std::ostream& os, const c_str_proxy& x)
      { os << x.c_str(); return os;
    }
  }
}

#endif  // BOOST_BTREE_CSTR_HPP
