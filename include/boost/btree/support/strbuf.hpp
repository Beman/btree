//  boost/btree/support/strbuf.hpp  ----------------------------------------------------//

//  Copyright Beman Dawes 2011

//  Distributed under the Boost Software License, Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt

#include <boost/btree/dynamic_size.hpp>
#include <string>
#include <cstring>
#include <boost/cstdint.hpp>
#include <boost/assert.hpp>

#ifndef BOOST_BTREE_STRBUF_HPP
#define BOOST_BTREE_STRBUF_HPP

namespace boost
{
namespace btree
{

  class strbuf
  {
  public:
    static const std::size_t max_size = 255;

    strbuf() : m_size(0) {m_buf[0] = '\0';}
    
    strbuf(const char* s)
    {
      std::size_t sz = std::strlen(s);
      if (sz > max_size)
        sz = max_size;
      std::memcpy(m_buf, s, sz);
      m_buf[sz] = '\0';
      m_size = sz;
    }

    strbuf(const strbuf& s)
    {
      std::strcpy(m_buf, s.m_buf);
      m_size = s.m_size;
    }

    strbuf& operator=(const char* s)
    {
      std::size_t sz = std::strlen(s);
      if (sz > max_size)
        sz = max_size;
      std::memcpy(m_buf, s, sz);
      m_buf[sz] = '\0';
      m_size = sz;
      return *this;
    }

    strbuf& operator=(const strbuf& s)
    {
      std::strcpy(m_buf, s.m_buf);  // self-assignment is harmless
      m_size = s.m_size;
      return *this;
    }

    std::size_t  size() const  {return m_size + 1 + sizeof(m_size);}
    const char*  c_str() const {return m_buf;}

    bool operator==(const strbuf& rhs) const {return std::strcmp(m_buf, rhs.m_buf) == 0;}
    bool operator!=(const strbuf& rhs) const {return std::strcmp(m_buf, rhs.m_buf) != 0;}
    bool operator< (const strbuf& rhs) const {return std::strcmp(m_buf, rhs.m_buf) < 0;}
    bool operator<=(const strbuf& rhs) const {return std::strcmp(m_buf, rhs.m_buf) <= 0;}
    bool operator> (const strbuf& rhs) const {return std::strcmp(m_buf, rhs.m_buf) > 0;}
    bool operator>=(const strbuf& rhs) const {return std::strcmp(m_buf, rhs.m_buf) >= 0;}

  private:
    boost::uint8_t  m_size;  // std::strlen(m_buf); for speed, particularly on large strings 
    char            m_buf[max_size+1];  // '\0' terminated
  };


  std::ostream& operator<<(std::ostream& os, const strbuf& x)
  {
    os << x.c_str();
    return os;
  }

  inline std::size_t dynamic_size(const strbuf& sb)  {return sb.size();}
  template<> struct has_dynamic_size<strbuf> : public true_type{};

}  // namespace btree
}  // namespace boost

#endif  // BOOST_BTREE_STRBUF_HPP
