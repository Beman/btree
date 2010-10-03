//  large_file_test.cpp  ---------------------------------------------------------------//

//  Copyright Beman Dawes 2010

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

#include <boost/btree/detail/binary_file.hpp>
#include <boost/filesystem.hpp>
#include <iostream>

using namespace boost::filesystem;
using std::cout;
using std::endl;

int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    cout << "Usage large_file_test <path>\n";
    return 1;
  }

  cout << "sizeof(boost::btree::binary_file::offset_type) is "
       << sizeof(boost::btree::binary_file::offset_type) << endl;

  path p(argv[1]);

  cout << "opening " << p << endl;
  boost::btree::binary_file lft(p, boost::btree::oflag::out
    | boost::btree::oflag::truncate);

  boost::btree::binary_file::offset_type result;
  const boost::btree::binary_file::offset_type five_billion = 5000000000LL;

  cout << "seeking" << endl;
  result = lft.seek(five_billion);

  cout << "seek(" << five_billion << ") returned " << result << endl;

  cout << "writing 1 byte" << endl;
  lft.write(" ", 1);

  cout << "closing " << p << endl;
  lft.close();

  cout << "file_size(" << p << ") is " << file_size(p) << endl;

  cout << "removing " << p << endl;
  remove(p);

  cout << "tests complete" << endl;
  return 0;
}
