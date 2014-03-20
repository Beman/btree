// Test to manifest the use_count bug, 1/15/2014

#include "file_modes.h"
#include "map_btree.hpp"
#include <iostream>
#include <cstdio>
#include <boost/detail/lightweight_main.hpp>

static const char temp_file[] = "temp_file";

int cpp_main(int argc, char* argv[]) {
  typedef map_btree_t<uint64_t, uint64_t> map_t;

  map_t* map;
  size_t size /*__attribute__((unused))*/;
  map_t::map_const_iterator_bool_pair it_bool_pair; 

  // clean up from any previous run
  remove(temp_file);

  // create new map
  map = new map_t(temp_file, RW_NEW);

  // check count
  size = map->size();

  // change entry invalid
  it_bool_pair = map->change(6000006, 60);

  // check count stayed same
  size = map->size();

  // end RW tests
  delete map;

  // open and reclose Read Only
  map = new map_t(temp_file, READ_ONLY);
  delete map;

  std::cout << "Almost done.\n";

  // done
  return 0;
}

