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
#include <boost/shared_ptr.hpp>
#include <iterator>
#include <algorithm>
#include <ostream>
#include <istream>
#include <fstream>

namespace boost
{
namespace btree
{
  template <class Key>
  struct set_data
  {
    Key key;

    bool operator<(const set_data& rhs) const {return key < rhs.key;}
  };

  template <class Key, class Mapped>
  struct map_data
  {
    Key key;
    Mapped mapped;

    bool operator<(const map_data& rhs) const {return key < rhs.key;}
  };

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
  void bulk_load(const boost::filesystem::path& source, BTree& bt,
    std::ostream& msg_stream, std::size_t available_memory)
  {
    namespace bfs = boost::filesystem;

    typedef typename BTree::key_type         key_type;
    typedef typename BTree::mapped_type      mapped_type;
    typedef map_data<key_type, mapped_type>  value_type;

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


    // TODO: fix this
    //bfs::path temp_path(bfs::temp_directory_path() / bfs::unique_path());
    bfs::path temp_path("d:/temp/btree");


    //  distribution phase: load, sort, and save source contents to temporary files

    {
      boost::btree::binary_file infile(source);
      msg_stream << "  distributing " << source << " contents to " << n_tmp_files
                 << " temporary file..." << std::endl;

      boost::scoped_array<value_type> buf(new value_type[max_elements_per_tmp_file]);
      value_type* begin = buf.get();;

      //  for each temporary file
      for (std::size_t file_n = 0; file_n < n_tmp_files; ++file_n)
      {

        // elements to read, sort, write
        std::size_t elements = 
          (n_elements - elements_completed) < max_elements_per_tmp_file
          ? static_cast<std::size_t>(n_elements - elements_completed)
          : max_elements_per_tmp_file;

        msg_stream << "    temporary file " << file_n << ", " << elements << " elements\n"
                      "      reading..." << endl;
        infile.read(begin, elements);

        msg_stream << "      sorting..." << endl;
        std::stable_sort(begin, begin + elements);

        // TODO: if only one temp file, don't bother to write and then read it!

        msg_stream << "      writing..." << endl;

        std::string tmppath = temp_path.string() + std::to_string(file_n) + ".tmp";
        boost::btree::binary_file tmpfile(tmppath, boost::btree::oflag::out);
        tmpfile.write(begin, elements);

        elements_completed += elements;
      }
      BOOST_ASSERT(elements_completed == n_elements);

      // TODO: throw if elements_completed != n_elements
      // Maybe even throw if sum of file sizes != n_elements * sizeof(value_type)
    }

    //  merge and insert phase

    struct file_state
    {
      boost::shared_ptr<std::ifstream>  stream_ptr;
      value_type                        element;  // current element
      std::size_t                       bytes_left;

      file_state() : stream_ptr(new std::ifstream)
      {
        stream_ptr->exceptions(std::ios_base::badbit | std::ios_base::failbit);
      }

      bool operator<(const file_state& rhs) const {return element < rhs.element;}

    };

    typedef std::vector<file_state> vector_type;

    vector_type files;
    files.reserve(n_tmp_files);

    // open each temporary file and set up its current element
    for (std::size_t file_n = 0; file_n < n_tmp_files; ++file_n)
      {
        std::string tmppath = temp_path.string() + std::to_string(file_n) + ".tmp";
        msg_stream << "      opening " << tmppath << endl;
        files[file_n].stream_ptr->open(tmppath.c_str(), std::ios_base::binary);
        files[file_n].stream_ptr->read(
          reinterpret_cast<char*>(&files[file_n].element),
          sizeof(value_type));
      }

    // insert minimum element into btree, preserving stability
    uint64_t inserts = 0;

    while (!files.empty())
    {
      vector_type::iterator min = std::min_element(files.begin(), files.end());

      bt.emplace(min->element.key, min->element.mapped);
      ++inserts;

      min->bytes_left -= sizeof(value_type);
      if (min->bytes_left)
        min->stream_ptr->read(
          reinterpret_cast<char*>(&min->element), sizeof(value_type));
      else
        files.erase(min);
    }
    BOOST_ASSERT(inserts == n_elements);
    // TODO throw if inserts != n_elements

    cout << "  done!\n\n";

    cout << bt << endl;

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
