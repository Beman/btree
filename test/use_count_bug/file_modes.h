// Author:  Bruce Allen <bdallen@nps.edu>
// Created: 2/25/2013
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

/**
 * \file
 * This file defines file modes used by hashdb.
 */

#ifndef FILE_MODES_H
#define FILE_MODES_H

//#include <stdint.h>
#include <cassert>
//#include <string.h> // for memcmp
#include <string>
#include <iostream>

enum file_mode_type_t {READ_ONLY,
                       RW_NEW,
                       RW_MODIFY};

inline std::string file_mode_type_to_string(file_mode_type_t type) {
  switch(type) {
    case READ_ONLY: return "read_only";
    case RW_NEW: return "rw_new";
    case RW_MODIFY: return "rw_modify";
    default: assert(0); return "";
  }
}

inline bool string_to_file_mode_type(const std::string& name, file_mode_type_t& type) {
  if (name == "read_only") { type = READ_ONLY; return true; }
  if (name == "rw_new")    { type = RW_NEW;    return true; }
  if (name == "rw_modify") { type = RW_MODIFY; return true; }
  type = READ_ONLY;
  return false;
}

inline std::ostream& operator<<(std::ostream& os, const file_mode_type_t& t) {
  os << file_mode_type_to_string(t);
  return os;
}

#endif

