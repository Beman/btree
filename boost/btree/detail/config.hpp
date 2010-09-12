//  boost/btree/detail/config.hpp  -----------------------------------------------------//

//  Copyright Beman Dawes 2003, 2010

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  Library home page: http://www.boost.org/libs/btree

//--------------------------------------------------------------------------------------// 

#ifndef BOOST_BTREE_CONFIG_HPP
#define BOOST_BTREE_CONFIG_HPP

// This header implements separate compilation features as described in
// http://www.boost.org/more/separate_compilation.html

#include <boost/config.hpp>
#include <boost/system/api_config.hpp>  // for BOOST_POSIX_API or BOOST_WINDOWS_API

//  throw an exception  ----------------------------------------------------------------//
//
//  Exceptions were originally thrown via boost::throw_exception().
//  As throw_exception() became more complex, it caused user error reporting
//  to be harder to interpret, since the exception reported became much more complex.
//  The immediate fix was to throw directly, wrapped in a macro to make any later change
//  easier.

#define BOOST_BTREE_THROW(EX) throw EX

//  enable dynamic linking -------------------------------------------------------------//

#if defined(BOOST_ALL_DYN_LINK) || defined(BOOST_BTREE_DYN_LINK)
# if defined(BOOST_BTREE_SOURCE)
#   define BOOST_BTREE_DECL BOOST_SYMBOL_EXPORT
# else 
#   define BOOST_BTREE_DECL BOOST_SYMBOL_IMPORT
# endif
#else
# define BOOST_BTREE_DECL
#endif

//  enable automatic library variant selection  ----------------------------------------// 

#if !defined(BOOST_BTREE_SOURCE) && !defined(BOOST_ALL_NO_LIB) \
  && !defined(BOOST_BTREE_NO_LIB)
//
// Set the name of our library, this will get undef'ed by auto_link.hpp
// once it's done with it:
//
#define BOOST_LIB_NAME boost_btree
//
// If we're importing code from a dll, then tell auto_link.hpp about it:
//
#if defined(BOOST_ALL_DYN_LINK) || defined(BOOST_BTREE_DYN_LINK)
#  define BOOST_DYN_LINK
#endif
//
// And include the header that does the work:
//
#include <boost/config/auto_link.hpp>
#endif  // auto-linking disabled

#endif // BOOST_BTREE_CONFIG_HPP
