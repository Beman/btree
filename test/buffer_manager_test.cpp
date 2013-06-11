//  buffer_manager_test.cpp ------------------------------------------------------------//

//  Copyright Beman Dawes 2010

//  Distributed under the Boost Software License, Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt

//  See library home buffer at http://www.boost.org/libs/fileio

//--------------------------------------------------------------------------------------//

#include <boost/config/warning_disable.hpp>

#define BOOST_FILESYSTEM_VERSION 3
#define BOOST_BUFFER_MANAGER_TEST

#include <boost/btree/detail/buffer_manager.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/detail/lightweight_main.hpp>
#include <boost/detail/lightweight_test.hpp> 

#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>

using namespace boost::btree;
namespace fs = boost::filesystem;

using std::cout;
using std::endl;

namespace
{
//  iterator_to_test  ------------------------------------------------------------------//

  void iterator_to_test()
  {
    cout << "iterator_to_test..." << endl;

    buffer pg (1);

    buffer_manager::buffers_type set;

    set.insert(pg);
    BOOST_TEST(&*set.begin() == &pg);
    //  buffer_manager logic relies on iterator_to working as expected:
    BOOST_TEST(set.begin() == set.iterator_to(pg));

    buffer_manager::avail_buffers_type list;

    list.push_back(pg);
    BOOST_TEST(&*list.begin() == &pg);
    //  buffer_manager logic relies on iterator_to working as expected:
    BOOST_TEST(list.begin() == list.iterator_to(pg));

    //  retest, just to be sure list.push_back() had no side effect on set.iterator_to()
    BOOST_TEST(&*set.begin() == &pg);
    BOOST_TEST(set.begin() == set.iterator_to(pg));

  }

//  buffer_test  -------------------------------------------------------------------------//

  void buffer_test()
  {
    cout << "buffer_test..." << endl;

    buffer pg, pg1(1), pg2(2);

    BOOST_TEST(!pg.use_count());
    BOOST_TEST(pg.buffer_id() == static_cast<buffer::buffer_id_type>(-1));
    BOOST_TEST(!pg1.use_count());
    BOOST_TEST(pg1.buffer_id() == 1);

    // TODO: need lots more tests

    //BOOST_TEST();
  }

//  buffer_ptr_test  ---------------------------------------------------------------------//

  void buffer_ptr_test()
  {
    cout << "buffer_ptr_test..." << endl;

    buffer pg1(1), pg2(2);

    //  test default constructed buffer_ptr
    buffer_ptr default_pp;
    BOOST_TEST(!default_pp);
    buffer_ptr default_pp2(default_pp);
    BOOST_TEST(!default_pp2);
    default_pp.reset();
    BOOST_TEST(!default_pp);
    BOOST_TEST(default_pp == default_pp2);
    BOOST_TEST(!(default_pp != default_pp2));
    default_pp2 = default_pp;

    //  test construction from buffer
    buffer_ptr pp(pg2);
    BOOST_TEST(pp.get() == &pg2);
    BOOST_TEST(pg2.use_count() == 1);
    BOOST_TEST((*pp).buffer_id() == 2);
    BOOST_TEST(pp->buffer_id() == 2);
    BOOST_TEST(pp->use_count() == 1);

    //  test copy construction
    buffer_ptr ccpp(pp);
    BOOST_TEST(ccpp.get() == &pg2);
    BOOST_TEST(pg2.use_count() == 2);
    BOOST_TEST((*ccpp).buffer_id() == 2);
    BOOST_TEST(ccpp->buffer_id() == 2);
    BOOST_TEST(ccpp->use_count() == 2);

    //  test copy assignment
    buffer_ptr capp;
    capp = ccpp;
    BOOST_TEST(capp.get() == &pg2);
    BOOST_TEST(pg2.use_count() == 3);
    BOOST_TEST((*ccpp).buffer_id() == 2);
    BOOST_TEST(capp->buffer_id() == 2);
    BOOST_TEST(capp->use_count() == 3);

    //  test reset of active pp
    capp.reset();
    BOOST_TEST(ccpp.get() == &pg2);
    BOOST_TEST(pg2.use_count() == 2);
    BOOST_TEST((*ccpp).buffer_id() == 2);
    BOOST_TEST(ccpp->buffer_id() == 2);
    BOOST_TEST(ccpp->use_count() == 2);

    //  test destruction of active pp
    {
      buffer_ptr app(pp);
      BOOST_TEST(pg2.use_count() == 3);
    }
    BOOST_TEST(pg2.use_count() == 2);


    //BOOST_TEST();
    //BOOST_TEST();
  }

//  open_new_file_test  ----------------------------------------------------------------//

  void open_new_file_test()
  {
    cout << "open_new_file_test..." << endl;

    fs::path test_path("buffer_manager");
    fs::remove(test_path);

    buffer_manager f;

    BOOST_TEST(f.buffer_count() == 0);
    BOOST_TEST(f.data_size() == 0);
    BOOST_TEST(f.max_cache_size() == 0);
    BOOST_TEST(!f.is_open());

    // case: file does not exist 
    bool existing_file = f.open(test_path,
      oflag::truncate);  // implies oflag::in|oflag::out|oflag::truncate
    BOOST_TEST(!existing_file);
    BOOST_TEST(f.is_open());
    BOOST_TEST(fs::file_size(test_path) == 0);
    f.close();
    BOOST_TEST(!f.is_open());
    BOOST_TEST(fs::exists(test_path));
    BOOST_TEST(fs::file_size(test_path) == 0);

    // case: file exists, but oflag::truncate is present so effect is that of a new file
    existing_file = f.open(test_path,
      oflag::truncate);  // implies oflag::in|oflag::out|oflag::truncate
    BOOST_TEST(!existing_file);
    BOOST_TEST(f.is_open());
    f.close();
    BOOST_TEST(!f.is_open());
    BOOST_TEST(fs::exists(test_path));
    BOOST_TEST(fs::file_size(test_path) == 0);
  }

//  open_existing_file_test  ----------------------------------------------------------------//

  void open_existing_file_test()
  {
    cout << "open_existing_file_test..." << endl;

    fs::path test_path("buffer_manager");
    fs::remove(test_path);

    buffer_manager f;

    // create the test file
    f.open(test_path, oflag::out);
    BOOST_TEST(f.is_open());
    buffer_ptr pp;
    pp = f.new_buffer();
    pp = f.new_buffer();
    pp = f.new_buffer();
    BOOST_TEST(f.buffer_count() == 3);
    BOOST_TEST(pp->buffer_id() == 2);
    BOOST_TEST(pp->use_count() == 1);
    BOOST_TEST(pp->manager() == &f);
    BOOST_TEST(pp->needs_write());

    f.close();
    BOOST_TEST(!f.is_open());
    BOOST_TEST(fs::exists(test_path));
    BOOST_TEST(fs::file_size(test_path) != 0);

    // reopen the existing test file 
    bool existing_file = f.open(test_path, oflag::out);
    BOOST_TEST(existing_file);
    BOOST_TEST(f.is_open());
    BOOST_TEST(f.data_size() == 0);
    f.data_size(sizeof(int));
    BOOST_TEST(f.data_size() == sizeof(int));
  }

//  new_buffer_test  ---------------------------------------------------------------------//

  void new_buffer_test()
  {
    cout << "new_buffer_test..." << endl;

    fs::path test_path("buffer_manager");
    fs::remove(test_path);
    buffer_manager f;
    
    f.open(test_path, oflag::out);  // create the test file
    BOOST_TEST(f.is_open());
    BOOST_TEST(f.buffer_count() == 0);

    buffer_ptr pp;
    pp = f.new_buffer();
    BOOST_TEST(f.buffer_count() == 1);
    BOOST_TEST(pp->buffer_id() == 0);
    BOOST_TEST(pp->use_count() == 1);
    BOOST_TEST(pp->manager() == &f);
    BOOST_TEST(pp->needs_write());

    pp = f.new_buffer();
    BOOST_TEST(f.buffer_count() == 2);
    BOOST_TEST(pp->buffer_id() == 1);
    BOOST_TEST(pp->use_count() == 1);
    BOOST_TEST(pp->manager() == &f);
    BOOST_TEST(pp->needs_write());
  }

//  existing_buffer_test  ----------------------------------------------------------------//

  void existing_buffer_test()
  {
    cout << "existing_buffer_test..." << endl;

    fs::path test_path("buffer_manager");
    fs::remove(test_path);
    buffer_manager f;
    
    f.open(test_path, oflag::out, 3, 256);  // create the test file
    BOOST_TEST(f.is_open());
    BOOST_TEST(f.buffer_count() == 0);
    BOOST_TEST_EQ(f.max_cache_size(), 3U);
    BOOST_TEST_EQ(f.data_size(), 256U);

    //  create three buffers to test with
    buffer_ptr pp0 = f.new_buffer();  // buffer_id 0
    buffer_ptr pp1 = f.new_buffer();  // buffer_id 1
    buffer_ptr pp2 = f.new_buffer();  // buffer_id 2
    BOOST_TEST(f.file_buffers_written() == 0);
    f.flush();
    BOOST_TEST(f.file_buffers_written() == 3);
    BOOST_TEST(f.buffers_in_memory() == 3);
    BOOST_TEST(f.buffers_available() == 0);


    BOOST_TEST(pp0 != pp1);
    BOOST_TEST(pp0 != pp2);
    BOOST_TEST(pp1 != pp2);

    //  test active buffers
    BOOST_TEST(f.read(0) == pp0);
    BOOST_TEST(f.read(0) != pp1);
    BOOST_TEST(f.read(0) != pp2);
    BOOST_TEST(f.read(1) == pp1);
    BOOST_TEST(f.read(1) != pp0);
    BOOST_TEST(f.read(1) != pp2);
    BOOST_TEST(f.read(2) == pp2);
    BOOST_TEST(f.read(2) != pp0);
    BOOST_TEST(f.read(2) != pp1);
    BOOST_TEST_EQ(f.active_buffers_read(), 9U);

    pp0.reset();
    pp1.reset();
    pp2.reset();
    BOOST_TEST_EQ(f.buffers_in_memory(), 3U);
    BOOST_TEST_EQ(f.buffers_available(), 3U);

    pp0 = f.read(0);
    pp1 = f.read(1);
    pp2 = f.read(2);
    BOOST_TEST_EQ(f.buffers_in_memory(), 3U);
    BOOST_TEST_EQ(f.buffers_available(), 0U);
    pp0.reset();
    pp1.reset();
    pp2.reset();
    BOOST_TEST_EQ(f.buffers_available(), 3U);
    f.new_buffer();  // buffer_id 3
    BOOST_TEST_EQ(f.buffers_in_memory(), 3U);
    BOOST_TEST_EQ(f.buffers_available(), 3U);
    BOOST_TEST_EQ(f.file_buffers_read(), 0U);
    buffer_ptr pp3 = f.read(0);  // least recently used
    BOOST_TEST_EQ(f.file_buffers_read(), 1U);
    BOOST_TEST_EQ(f.buffers_in_memory(), 3U);
    BOOST_TEST_EQ(f.buffers_available(), 2U);

    f.flush();
    cout << f;
  }

} // unnamed namespace

//  cpp_main  --------------------------------------------------------------------------//

int cpp_main(int argc, char * argv[])
{
  iterator_to_test();
  buffer_test();
//  buffer_ptr_test();
  open_new_file_test();
  open_existing_file_test();
  new_buffer_test();
  existing_buffer_test();

  cout << "all tests complete" << endl;

  return boost::report_errors();
}

//--------------------------------------------------------------------------------------//
