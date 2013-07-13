//  mmf_experiment.cpp  -  memory mapped file experiments  -----------------------------//

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

#include <boost/filesystem.hpp>
#include <boost/btree/detail/binary_file.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/detail/lightweight_main.hpp>
#include <iostream>
#include <string>
#include <cstring>

using namespace boost::btree;

int cpp_main(int argc, char* argv[])
{
  std::string p("test.mmf");

  //  Test 1: Extend the file before opening as a mapped_file
  {
    {
      binary_file fi(p, oflag::in | oflag::out | oflag::truncate | oflag::random);
      fi.seek(12);
      fi.write(0);
    }

    boost::iostreams::mapped_file mmf(p,
      boost::iostreams::mapped_file::readwrite, 1000);

    std::memset(mmf.data(), 0x22, 100);
    mmf.close();
    std::cout << p << " size is " << boost::filesystem::file_size(p) << std::endl;
    // output: test.mmf size is 16
  }

  //  Test 2: Extend the file while also open as a mapped_file
  //  The mmf constructor throws with the message:
  //    "failed opening file: The process cannot access the file because it is
  //    being used by another process."
  if (false)
  {
    binary_file fi(p, oflag::in | oflag::out | oflag::truncate | oflag::random);
    fi.write(0);  // mapped_file open will fail if file is 0 length

    boost::iostreams::mapped_file mmf(p,
      boost::iostreams::mapped_file::readwrite, 1000);

    fi.seek(12);
    fi.write(0);

    std::memset(mmf.data(), 0x22, 100);
    mmf.close();
    std::cout << p << " size is " << boost::filesystem::file_size(p) << std::endl;
  }

  //  Test 3: Extend the file before opening as a mapped_file, then resize
  //          after mapped_file is closed
  {
    {
      binary_file fi(p, oflag::in | oflag::out | oflag::truncate | oflag::random);
      fi.seek(50);
      fi.write(0);
    }

    boost::iostreams::mapped_file mmf(p,
      boost::iostreams::mapped_file::readwrite, 1000);

    std::memset(mmf.data(), 0x22, 100);
    mmf.close();

    boost::filesystem::resize_file(p, 32);

    std::cout << p << " size is " << boost::filesystem::file_size(p) << std::endl;
    // output: test.mmf size is 32
  }

  //  Test 4: Resize the file before opening as a mapped_file, then resize
  //          after mapped_file is closed
  {

    boost::filesystem::resize_file(p, 75);

    std::cout << p << " size is " << boost::filesystem::file_size(p) << std::endl;
    // output: test.mmf size is 75

    boost::iostreams::mapped_file mmf(p,
      boost::iostreams::mapped_file::readwrite, 1000);

    std::memset(mmf.data(), 0x22, 100);
    mmf.close();

    boost::filesystem::resize_file(p, 32);

    std::cout << p << " size is " << boost::filesystem::file_size(p) << std::endl;
    // output: test.mmf size is 32
  }

  return 0;
}
