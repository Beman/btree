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
    BTree& bt, std::ostream& msg_stream, std::size_t available_memory);

  template <class Key, class T, class Traits = default_traits,
    class Comp = btree::less<Key> >
  class bulk_load_map
  {
  public:
    void operator()(
      const boost::filesystem::path& source,
      const boost::filesystem::path& target,
      std::ostream& msg_stream,
      std::size_t available_memory,
      flags::bitmask flgs = boost::btree::flags::read_write,
      uint64_t signature = -1,  // for existing files, must match signature from creation
      std::size_t node_sz = default_node_size,  // ignored if existing file
      const Comp& comp = Comp()
      );
  };

 
  //------------------------------------------------------------------------------------//
  //                                  implementation                                    //
  //------------------------------------------------------------------------------------//

  //---------------------------------  bulk_load_map  ----------------------------------//

  template <class Key, class T, class Traits, class Comp>
  void bulk_load_map<Key, T, Traits, Comp>::operator()(
    const boost::filesystem::path& source, const boost::filesystem::path& target,
    std::ostream& msg_stream, std::size_t available_memory, flags::bitmask flgs,
    uint64_t signature, std::size_t node_sz, const Comp& comp )
    {
      boost::btree::btree_map<Key, T, Traits, Comp>
        bt(target, flgs, signature, node_sz, comp);
      bulk_load(source, bt, msg_stream, available_memory);
    }

  //-----------------------------------  bulk_load  ------------------------------------//

  template <class BTree>
  void bulk_load(const boost::filesystem::path& source, BTree& tree,
    std::ostream& msg_stream, std::size_t available_memory)
  {
    namespace bfs = boost::filesystem;

    typedef typename BTree::value_type value_type;

    std::size_t  max_elements_per_tmp_file  = available_memory / sizeof(value_type);
    uint64_t     file_size                  = bfs::file_size(source);
    uint64_t     n_elements                 = file_size / sizeof(value_type);
    std::size_t  n_tmp_files                = static_cast<std::size_t>
                                                (1 + n_elements/max_elements_per_tmp_file);
    uint64_t     elements_completed         = 0;

    if (file_size % sizeof(value_type))
    {
      std::string emess(source.string());
      emess += " file size is not a multiple of the value_type size";
      throw std::runtime_error(emess.c_str());
    }

    bfs::path temp_path(bfs::temp_directory_path() / bfs::unique_path());


    //  distribution phase: load, sort, and save source contents to temporary files

    {
      boost::btree::binary_file infile(source);
      msg_stream << "  distributing " << source << " contents to " << n_tmp_files
                 << " temporary file..." << std::endl;

      boost::scoped_array<value_type> buf(new value_type[max_elements_per_tmp_file]);
      value_type* begin = buf.get();;

      //  for each temporary file

      for (std::size_t file_n = 0; file_n < n_tmp_files; )
      {

        // elements to read, sort, write
        std::size_t elements = 
          (n_elements - elements_completed) < max_elements_per_tmp_file
          ? static_cast<std::size_t>(n_elements - elements_completed)
          : max_elements_per_tmp_file;

        msg_stream << "    temporary file " << file_n << ", " << elements << "elements\n"
                      "      reading..." << endl;
        infile.read(begin, elements);

        msg_stream << "      sorting..." << endl;
        std::stable_sort(begin, begin + elements);

        // TODO: if only one temp file, don't bother to write and then read it!

        msg_stream << "      writing..." << endl;
        std::string tmppath = temp_path.string() + ".tmp" + std::to_string(file_n);
        boost::btree::binary_file tmpfile(tmppath, boost::btree::oflag::out);
        tmpfile.write(begin, elements);

        elements_completed += elements;
      }
      BOOST_ASSERT(elements_completed == n_elements);

      // TODO: throw if elements_completed != n_elements
      // Maybe even throw if sum of file sizes != n_elements * sizeof(value_type)
    }

    //  merge and insert phase


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
