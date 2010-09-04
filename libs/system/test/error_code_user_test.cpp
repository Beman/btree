//  error_code_user_test.cpp  ------------------------------------------------//

//  Copyright Beman Dawes 2006

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See library home page at http://www.boost.org/libs/system

//  ------------------------------------------------------------------------  //
//  This code demonstrates creation and use of a new category of error codes.

//  The motivation was a Boost posting by Christopher Kohlhoff on June 28, 2006.

#include <boost/system/error_code.hpp>
#include <boost/cerrno.hpp>


# ifndef BOOST_NO_STD_WSTRING  // workaround Cygwin's lack of wstring_t
    typedef std::wstring wstring_t;
# else
    typedef std::basic_string<wchar_t> wstring_t;
# endif

//  ------------------------------------------------------------------------  //

//  header asio.hpp

#define BOO_BOO 12345  // this could also be a constant; a macro is used for
                       // illustration because many older API's define errors
                       // via macro.
namespace boost
{
  namespace asio
  {
    // asio declares have its own error_category:
    class asio_error_category : public boost::system::error_category
    {
    public:
      const std::string & name() const;
      int                 to_errno( boost::int_least32_t ev ) const;
      std::string         message( boost::int_least32_t ev ) const;
      wstring_t           wmessage( boost::int_least32_t ev ) const;
    };
    extern asio_error_category asio_error;
    
    namespace error
    {
      extern boost::system::error_code boo_boo;
    }

    void boo_boo( boost::system::error_code & ec );
  }
}

//  ------------------------------------------------------------------------  //

//  implementation file asio.cpp:

namespace boost
{
  namespace asio
  {
    asio_error_category asio_error;

    const std::string & asio_error_category::name() const
    {
      static std::string s( "asio" );
      return s;
    }
    int asio_error_category::to_errno( boost::int_least32_t ev ) const
    {
      return ev == BOO_BOO ? EBADHANDLE : EOTHER;
    }

    std::string asio_error_category::message( boost::int_least32_t ev ) const
    {
      return std::string( "Barf" );
    }

    wstring_t asio_error_category::wmessage( boost::int_least32_t ev ) const
    {
      return wstring_t( L"Barf" );
    }

    namespace error
    {
      boost::system::error_code boo_boo( BOO_BOO, asio_error );
    }

    //  function sets ec arg to boo_boo
    void boo_boo( boost::system::error_code & ec )
    {
      ec = error::boo_boo;
    }
  }
}

//  ------------------------------------------------------------------------  //

//  a user program:


// #include <asio.hpp>
#include <boost/test/minimal.hpp>

int test_main( int, char *[] )
{
  boost::system::error_code ec;
  boost::asio::boo_boo( ec );

  BOOST_CHECK( ec );
  BOOST_CHECK( ec == boost::asio::error::boo_boo );
  BOOST_CHECK( ec.value() == BOO_BOO );
  BOOST_CHECK( ec.category() == boost::asio::asio_error );

  BOOST_CHECK( ec.to_errno() == EBADHANDLE );
  BOOST_CHECK( ec.message() == "Barf" );
  BOOST_CHECK( ec.wmessage() == L"Barf" );
  return 0;
}
