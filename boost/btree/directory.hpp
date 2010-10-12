//  boost/btree/directory.hpp  ---------------------------------------------------------//

//  Copyright Beman Dawes 2010

//  Distributed under the Boost Software License, Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt

//  See http://www.boost.org/libs/btree for documentation.

#ifndef BOOST_BTREE_DIRECTORY_HPP
#define BOOST_BTREE_DIRECTORY_HPP

#define BOOST_FILESYSTEM_VERSION 3

#include <boost/btree/map.hpp>
#include <boost/btree/detail/fixstr.hpp>

/*--------------------------------------------------------------------------------------//

Rationale for plan-of-attack:

  Fear!
  
  * Fear of bugs that mixup pages from one tree with another tree.
  * Fear of making it difficult to reason about btree state.
  * Fear of complexifying code.
  * Fear of complexifying testing.

    By pushing as many changes as possible to the lowest level, the binary_file object,
    it is hoped that these fears will be avoided.

Plan-of-attack:

  * Change buffer manager to contain a shared_ptr to a binary_file_manager, instead of
    being derived from binary_file.
  * Push all disk page allocation and free list management into the binary_file_manager.
  * Add open() overloads to each btree class and base class that just copy the root's
    binary_file_manager shared_ptr.
  * It is a precondition on directory close() that all children are closed. Can be checked
    by testing the shared_ptr count is 1. Assert on count for debug builds, throw for
    release builds.
  * Document free list head in header only applies to root directory, not sub-directories
    or data.

//--------------------------------------------------------------------------------------*/


namespace boost
{
namespace btree
{

//--------------------------------------------------------------------------------------//
//                               class directory_entry                                  //
//--------------------------------------------------------------------------------------//

class directory_entry
{
  header_page::page_id_type  header_page_id;
};

//--------------------------------------------------------------------------------------//
//                                  class directory                                     //
//--------------------------------------------------------------------------------------//

class directory : public btree_map<boost::detail::fixstr<31>, directory_entry>
{
  ~directory
  open
  close
  insert
};


} // namespace btree
} // namespace boost

#endif  // BOOST_BTREE_DIRECTORY_HPP
