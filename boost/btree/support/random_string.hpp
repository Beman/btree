//  random_string.hpp  -----------------------------------------------------------------//

//  Copyright Beman Dawes 2011

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_RANDOM_STRING_HPP       
#define BOOST_RANDOM_STRING_HPP

#include <cstdlib>
#include <string>
#include <boost/random.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>

namespace boost
{
  class random_string
  {
  public:
    random_string(int min_len, int max_len,
            int min_char = ' ', int max_char = '~')
    {
      m_char_dist.reset(new uniform_int<> (min_char, max_char));
      m_char.reset(new variate_generator<rand48&, uniform_int<> >
        (m_char_rng, *m_char_dist));

      m_len_dist.reset(new uniform_int<> (min_len, max_len));
      m_len.reset(new variate_generator<rand48&, uniform_int<> >
        (m_len_rng, *m_len_dist));
    }
    std::string operator()();

    void seed(int x)
    {
      m_len_rng.seed(x);
      m_char_rng.seed(x);
    }

  private:

    rand48  m_char_rng;
    shared_ptr<uniform_int<> >  m_char_dist;
    shared_ptr<variate_generator<rand48&, uniform_int<> > >  m_char;

    rand48  m_len_rng;
    shared_ptr<uniform_int<> >  m_len_dist;
    shared_ptr<variate_generator<rand48&, uniform_int<> > >  m_len;
  };

  std::string random_string::operator()()
  {
    std::string s;
    for (int len = m_len->operator()(); len > 0; --len)
      s += m_char->operator()();
    //std::cout << s << '\n';
    return s;
  }

} // namespace boost

#endif  // BOOST_RANDOM_STRING_HPP
