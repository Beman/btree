//  volume/data.hpp  -----------------------------------------------------------------//

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_BTREE_VOLUME_DATA_HPP
#define BOOST_BTREE_VOLUME_DATA_HPP

#include <boost/cstdint.hpp>
#include <utility>

namespace volume
{
  class u128_t
  {
    boost::uint64_t m_hi;
    boost::uint64_t m_lo;
  public:
    u128_t() {}
    u128_t(const u128_t& x) : m_hi(x.m_hi), m_lo(x.m_lo) {}
    u128_t(const boost::uint64_t hi, const boost::uint64_t lo) : m_hi(hi), m_lo(lo) {}

    u128_t& operator=(const u128_t& x) {m_hi = x.m_hi; m_lo = x.m_lo; return *this;}
    u128_t& assign(const boost::uint64_t hi, const boost::uint64_t lo)
      {m_hi = hi; m_lo = lo; return *this;}

    bool operator<(const u128_t& x) const
    {
      return m_hi < x.m_hi
        || (m_hi == x.m_hi && m_lo < x.m_lo);
    }

  };

  class data
  {
  public:
    u128_t            key;
    boost::uint64_t   mapped;
  };

  //  

}  // namespace volume

#endif
