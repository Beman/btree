//  random_string.hpp  -----------------------------------------------------------------//

//  Copyright Beman Dawes 2011

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_RANDOM_STRING_HPP       
#define BOOST_RANDOM_STRING_HPP

#include <cstddef>
#include <string>
#include <boost/random.hpp>

namespace boost
{
  class random_string
  {
  public:
    random_string(std::size_t min_len_, std::size_t max_len_,
            int min_char_ = ' ', int max_char_ = '~')
            : m_min_len(min_len_), m_max_len(max_len_),
              m_min_char(min_char_), m_max_char(max_char) {}

    std::string operator()();

    void seed(int x) {m_char_rng.seed(x);}

  private:
    std::size_t           m_min_len, m_max_len;
    int                   m_min_char, m_max_char;
    boost::rand48         m_char_rng;
    boost::uniform_int<>  m_char_dist(m_min_char, m_max_char);
    boost::variate_generator<boost::rand48&, boost::uniform_int<> >
                          m_char(m_char_rng, m_char_dist);
    boost::rand48         m_len_rng;
    boost::uniform_int<>  m_len_dist(m_min_char, m_max_char);
    boost::variate_generator<boost::rand48&, boost::uniform_int<> >
                          m_len(m_len_rng, m_len_dist);
  };

  std::string random_string::operator()()
  {
    std::string s;
    for (std::size_t len = m_len(); len > 0; --len);
      s += m_char();
    return s;
  }

} // namespace boost

#endif  // BOOST_RANDOM_STRING_HPP
