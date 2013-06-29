//  boost/btree/bulk_loader.hpp  -------------------------------------------------------//

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_BTREE_BULK_LOADER_HPP
#define BOOST_BTREE_BULK_LOADER_HPP

#include <boost/btree/map.hpp>
#include <boost/btree/set.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/btree/detail/binary_file.hpp>
#include <boost/scoped_array.hpp>
#include <algorithm>
#include <ostream>
#include <istream>
#include <fstream>

namespace boost
{
namespace btree
{

  template <class BTree>
  void bulk_load(const boost::filesystem::path& source,
    BTree& bt, std::ostream& msg_stream,
    std::size_t available_memory);

  template <class Key, class T, class Traits = default_traits,
    class Comp = btree::less<Key> >
  void bulk_load_map(
    const boost::filesystem::path& source,
    const boost::filesystem::path& target,
    std::ostream& msg_stream,
    std::size_t available_memory,
    flags::bitmask flgs = boost::btree::flags::read_write,
    uint64_t signature = -1,  // for existing files, must match signature from creation
    std::size_t node_sz = default_node_size,  // ignored if existing file
    const Comp& comp = Comp()
    );

 
  //------------------------------------------------------------------------------------//
  //                                  implementation                                    //
  //------------------------------------------------------------------------------------//

  //---------------------------------  bulk_load_map  ----------------------------------//

   template <class Key, class T, class Traits = default_traits,
    class Comp = btree::less<Key> >
  void bulk_load_map(const boost::filesystem::path& source,
    const boost::filesystem::path& target, std::ostream& msg_stream,
    std::size_t available_memory, flags::bitmask flgs, uint64_t signature,
    std::size_t node_sz, const Comp& comp )
    {
      boost::btree::btree_map<Key, T, Traits, Comp>
        bt(target, flgs, signature, node_sz, comp);
      bulk_load(source, bt, msg_stream, available_memory);
    }

  //-----------------------------------  bulk_load  ------------------------------------//

  template <class BTree>
  void bulk_load(const boost::filesystem::path& source, BT& tree,
    std::ostream& msg_stream, std::size_t available_memory)
  {
    const std::size_t max_elements_per_block = available_memory / sizeof(T);
    uint64_t file_size = boost::filesystem::file_size(source);
    uint64_t elements_in_file = file_size / sizeof(T);

    if (elements_in_file % sizeof(T))
    {
      std::string emess(source.string());
      emess += " file size is not a multiple of the data element size";
      throw std::runtime_error(emess.c_str());
    }

// TODO: should be a temporary file.

    std::fstream tmpfile("temp",
      std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);

    //  load, sort, and save to distribution files
    {
      std::ifstream infile(source);

      boost::scoped_array<T> buf(new T[max_elements_per_block]);
      T* begin = buf.get();;

      //  for each block
      for (uint64_t element_count = 0; element_count < elements_in_file;)
      {
        // elements to read, sort, write
        uint64_t elements = (elements_in_file - element_count) < max_elements_per_block
          ? (elements_in_file - element_count) :  max_elements_per_block;

        infile.read(begin, elements);                // load
        std::stable_sort(begin, begin + elements);   // sort
        tmpfile.write(begin, elements);              // save

        element_count += elements;
      }
    }

    //  insert phase

    }

    // set up the merge iterators

  }

  //-------------------------------------  main  ---------------------------------------//

  //int bulk_loader<T>::main(int argc, char * argv[], std::ostream& os)
  //{


  //  if (argc != 3) 
  //  {
  //    os << "Usage: " << argv[0] << " [Options] source-path btree_path\n"
  //      " Options:\n"
  //      "   -l#      log progress every # actions; default is to not log\n"
  //      "   -sep[punct] cout thousands separator; space if punct omitted, default -sep,\n"
  //// TODO:      "   -v       Verbose output statistics\n"
  //      ;
  //    return 1;
  //  }
  //}

}  // namespace btree
}  // namespace boost

#endif
