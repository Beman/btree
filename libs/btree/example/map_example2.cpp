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
using std::string;
using boost::btree::strbuf;

int main()
{
  typedef std::map<string, string> std_map_type;
  std_map_type std_map;

  std_map.insert(std::make_pair<string, string>("eat", "comer"));
  std_map.insert(std::make_pair<string, string>("drink", "beber"));
  std_map.insert(std::make_pair<string, string>("be merry", "ser feliz"));

  cout << "std_map:\n";
  for (std_map_type::iterator it = std_map.begin(); it != std_map.end(); ++it)
    cout << "  " << it->first << " --> " << it->second << '\n';

  typedef boost::btree::btree_map<strbuf, strbuf> btree_map_type;
  btree_map_type bt_map("bt_map.btr", boost::btree::flags::truncate, 128);

  strbuf key, mapped_value;

  bt_map.insert(strbuf("eat"), strbuf("comer"));
  bt_map.insert(strbuf("drink"), strbuf("beber"));
  bt_map.insert(strbuf("be merry"), strbuf("ser feliz"));

  cout << "bt_map:\n";
  for (btree_map_type::iterator it = bt_map.begin(); it != bt_map.end(); ++it)
    cout << "  " << it->key() << " --> " << it->mapped_value() << '\n';

  return 0;
}
