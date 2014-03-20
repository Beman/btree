// Author:  Joel Young <jdyoung@nps.edu>
//
// The software provided here is released by the Naval Postgraduate
// School, an agency of the U.S. Department of Navy.  The software
// bears no warranty, either expressed or implied. NPS does not assume
// legal liability nor responsibility for a User's use of the software
// or the results of such use.
//
// Please note that within the United States, copyright protection,
// under Section 105 of the United States Code, Title 17, is not
// available for any work of the United States Government and/or for
// any works created by United States Government employees. User
// acknowledges that this software contains work which was created by
// NPS government employees and is therefore in the public domain and
// not subject to copyright.
//
// Released into the public domain on February 25, 2013 by Bruce Allen.

#ifndef MAP_BTREE_HPP
#define MAP_BTREE_HPP

#include <vector>
#include <string>
#include <sstream>
#include <cstdio>
#include <cassert>

// Boost includes
#include <boost/functional/hash.hpp>
#include <boost/btree/btree_map.hpp>

// KEY_T must be something that is a lot like md5_t (nothing with pointers)
// both KEY_T and PAY_T must not use dynamic memory
template<typename KEY_T, typename PAY_T>
class map_btree_t {
  private:
    // btree map
    typedef boost::btree::btree_map<KEY_T, PAY_T> map_t;

  public:
    // btree map iterator
    typedef /*class*/ typename map_t::const_iterator map_const_iterator;

    // pair returned by emplace
    typedef /*class*/ typename std::pair<map_const_iterator, bool> map_const_iterator_bool_pair;

  private:
    const std::string filename;
    const file_mode_type_t file_mode;
    map_t* map;
    
    // do not allow copy or assignment
    map_btree_t(const map_btree_t&);
    map_btree_t& operator=(const map_btree_t&);

  public:

    // access to new store based on file_mode_type_t in hashdb_types.h,
    // specifically: READ_ONLY, RW_NEW, RW_MODIFY
    map_btree_t(const std::string& p_filename,
                         file_mode_type_t p_file_mode) : 
          filename(p_filename)
         ,file_mode(p_file_mode)
         ,map(0) {

      if (file_mode == READ_ONLY) {
        map = new map_t(filename, boost::btree::flags::read_only);
        // note, old code used to "map->max_cache_size(65536);" here.
        // lets not limit this now.
      } else if (file_mode == RW_NEW) {
        map = new map_t(filename, boost::btree::flags::truncate);
      } else if (file_mode == RW_MODIFY) {
        map = new map_t(filename, boost::btree::flags::read_write);
      } else {
        assert(0);
      }
    }

    ~map_btree_t() {
      // pack btree to .scratch
      // zz lets not optimize this at this time, see manager_modified.h

      // close
      delete map;
    }

  public:
    // insert
    std::pair</*class*/ typename map_t::const_iterator, bool>
    emplace(const KEY_T& key, const PAY_T& pay) {
      if (file_mode == READ_ONLY) {
        throw std::runtime_error("Error: emplace called in RO mode");
      }

      return map->emplace(key, pay);
    }

    // erase
    size_t erase(const KEY_T& key) {
      if (file_mode == READ_ONLY) {
        throw std::runtime_error("Error: erase called in RO mode");
      }

      size_t num_erased = map->erase(key);
      return num_erased;
    }

    // change
    std::pair</*class*/ typename map_t::const_iterator, bool>
    change(const KEY_T& key, const PAY_T& pay) {
      if (file_mode == READ_ONLY) {
        throw std::runtime_error("Error: change called in RO mode");
      }

      // erase the old element
      size_t num_erased = erase(key);
      if (num_erased != 1) {
        // erase failed
        return std::pair</*class*/ typename map_t::const_iterator, bool>(map->end(), false);
      } else {
        // put in new
        return map->emplace(key, pay);
      }
    }

    // find
    typename map_t::const_iterator find(const KEY_T& key) const {
        class map_t::const_iterator itr = map->find(key);
        return itr;
    }

    // has
    bool has(const KEY_T& key) const {
      if (find(key) != map->end()) {
        return true;
      } else {
        return false;
      }
    }

    // begin
    typename map_t::const_iterator begin() const {
      return map->begin();
    }

    // end
    typename map_t::const_iterator end() const {
      return map->end();
    }

    // number of elements
    size_t size() {
      return map->size();
    }
};

#endif

