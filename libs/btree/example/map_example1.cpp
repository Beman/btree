//  map_example1.cpp  ------------------------------------------------------------------//

//  Copyright Beman Dawes 2011

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt


#include <map>
#include <utility>
#include <string>
#include <boost/btree/map.hpp>
#include <boost/btree/support/strbuf.hpp>
#include <iostream>
using std::cout;

int main()
{
  typedef std::map<int, long> std_map_type;
  std_map_type std_map;

  std_map.insert(std::make_pair<int, long>(2, -2));
  std_map.insert(std::make_pair<int, long>(3, -3));
  std_map.insert(std::make_pair<int, long>(1, -1));

  cout << "std_map:\n";
  for (std_map_type::iterator it = std_map.begin(); it != std_map.end(); ++it)
    cout << "  " << it->first << " --> " << it->second << '\n';

  typedef boost::btree::btree_map<int, long> btree_map_type;
  btree_map_type bt_map("bt_map.btr", boost::btree::flags::truncate);

  bt_map.emplace(2, -2);
  bt_map.emplace(3, -3);
  bt_map.emplace(1, -1);

  cout << "bt_map:\n";
  for (btree_map_type::iterator it = bt_map.begin(); it != bt_map.end(); ++it)
    cout << "  " << it->key() << " --> " << it->mapped_value() << '\n';

  return 0;
}
