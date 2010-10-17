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

  By minimizing change, and limiting operationsal differences, it is hoped that these fears
  will not turn into realities.

Plan-of-attack:

  * Change buffer_manager to contain a shared_ptr to a binary_file, instead of being
    derived from binary_file. In a directory_tree and its sub-trees, the shared_ptr will
    point to the same instance of binary_file.

  * btree_base will have an additional header pointer. For all children of a
    btree_directory, it will point to the root btree_directory's header. For independent
    btrees, it will point to the independent btree's header. All disk page allocation,
    deallocation, and free list management will use that header pointer. All other uses
    will continue to use m_hdr.

  * Add open() overloads to each btree class and base class that copy the root's
    binary_file shared_ptr and set up the header pointer.

  * It is a precondition on directory close() that all children are closed. Can be checked
    by testing the shared_ptr count is 1. Assert on count for debug builds, throw for
    release builds.

  * Document in header which values are shared and which are unique.

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
