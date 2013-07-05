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
#include <boost/lexical_cast.hpp>
#include <boost/timer/timer.hpp>
#include <iterator>
#include <algorithm>
#include <ostream>
#include <istream>
#include <fstream>
#include <stdexcept>

#define BOOST_BTREE_CSTDIO
#ifdef BOOST_BTREE_CSTDIO
#include <cstdio>
#include <boost/system/error_code.hpp>
#endif

/*

  TODO:

  * If file_size(source) <= max_memory: just load, sort, and insert:-)

*/

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
  void bulk_load(
    const boost::filesystem::path& source,
    BTree& bt,
    const boost::filesystem::path& temp_dir,
    std::ostream& msg_stream,
    std::size_t max_memory,
    uint64_t log_point);

  template <class Key, class T, class Traits = default_traits,
    class Comp = btree::less<Key> >
  class bulk_load_map
  {
  public:
    void operator()(
      const boost::filesystem::path& source,
      const boost::filesystem::path& target,
      const boost::filesystem::path& temp_dir,
      std::ostream& msg_stream,
      std::size_t max_memory,
      uint64_t log_point = 0,
      flags::bitmask flgs = boost::btree::flags::read_write,
      uint64_t signature = -1,  // for existing files, must match signature from creation
      std::size_t node_sz = default_node_size,  // ignored if existing file
      const Comp& comp = Comp()
      );
  };

 
  //------------------------------------------------------------------------------------//
  //                                  implementation                                    //
  //------------------------------------------------------------------------------------//

  namespace detail
  {
    //  This helper was originally written as a local non-template class inside
    //  bulk_load(), but GCC in C++03 mode correctly rejects use of a local class as a
    //  template-argument for a template type-parameter. Asside: this restriction is
    //  lifted in  C++11
    template <class T>
    struct file_state
    {
#ifdef BOOST_BTREE_CSTDIO
      FILE*  stream_ptr;
#else
      boost::shared_ptr<std::ifstream>  stream_ptr;
#endif
      T                                 element;  // current element
      std::size_t                       bytes_left;

#ifdef BOOST_BTREE_CSTDIO
      file_state() : stream_ptr(0)
#else
      file_state() : stream_ptr(new std::ifstream)
#endif
      {
#ifndef BOOST_BTREE_CSTDIO
        stream_ptr->exceptions(std::ios_base::badbit | std::ios_base::failbit);
#endif
      }

      bool operator<(const file_state& rhs) const {return element < rhs.element;}
     };
  }

  //---------------------------------  bulk_load_map  ----------------------------------//

  template <class Key, class T, class Traits, class Comp>
  void bulk_load_map<Key, T, Traits, Comp>::operator()(
      const boost::filesystem::path& source,
      const boost::filesystem::path& target,
      const boost::filesystem::path& temp_dir,
      std::ostream& msg_stream,
      std::size_t max_memory,
      uint64_t log_point,
      flags::bitmask flgs,
      uint64_t signature,
      std::size_t node_sz,
      const Comp& comp)
    {
      boost::btree::btree_map<Key, T, Traits, Comp>
        bt(target, flgs, signature, node_sz, comp);
      bulk_load(source, bt, temp_dir, msg_stream, max_memory, log_point);
    }

  //-----------------------------------  bulk_load  ------------------------------------//

  template <class BTree>
  void bulk_load(
    const boost::filesystem::path& source,
    BTree& bt,
    const boost::filesystem::path& temp_dir,
    std::ostream& msg_stream,
    std::size_t max_memory,
    uint64_t log_point)
  {
    namespace bfs = boost::filesystem;

    typedef typename BTree::key_type         key_type;
    typedef typename BTree::mapped_type      mapped_type;
    typedef map_data<key_type, mapped_type>  value_type;

    std::size_t  max_elements_per_tmp_file  = (max_memory / sizeof(value_type)) + 1;
    uint64_t     file_size                  = bfs::file_size(source);
    uint64_t     n_elements                 = file_size / sizeof(value_type);
    std::size_t  n_tmp_files                = static_cast<std::size_t>
                                                (1 + n_elements/max_elements_per_tmp_file);
    if (file_size % sizeof(value_type))
    {
      std::string emess(source.string());
      emess += " file size is not a multiple of the value_type size";
      throw std::runtime_error(emess.c_str());
    }

    //----------------------------------------------------------------------------------//
    //  distribution phase: load, sort, and save source contents to temporary files
    //----------------------------------------------------------------------------------//

    uint64_t  elements_completed = 0;
    boost::timer::auto_cpu_timer t(msg_stream, 1);

    {
      boost::btree::binary_file infile(source, boost::btree::oflag::in
        | boost::btree::oflag::sequential);
      msg_stream << "  distributing " << source << " contents to " << n_tmp_files
                 << " temporary files..." << std::endl;

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
                      "      reading..." << std::endl;
        t.start();
        infile.read(begin, elements);
        t.stop();
        msg_stream << "       ";
        t.report();

        msg_stream << "      sorting..." << std::endl;
        t.start();
        std::stable_sort(begin, begin + elements);
        t.stop();
        msg_stream << "       " ;
        t.report();

        msg_stream << "      writing..." << std::endl;
        t.start();
        std::string file_n_string = boost::lexical_cast<std::string>(file_n);
        bfs::path tmp_path = temp_dir / bfs::path("btree.tmp" + file_n_string);
        boost::btree::binary_file tmpfile(tmp_path, boost::btree::oflag::out
          | boost::btree::oflag::truncate | boost::btree::oflag::sequential);
        tmpfile.write(begin, elements);
        t.stop();
        msg_stream << "       ";
        t.report();

        elements_completed += elements;
      }
      BOOST_ASSERT(elements_completed == n_elements);

      // TODO: throw if elements_completed != n_elements
      // Maybe even throw if sum of file sizes != n_elements * sizeof(value_type)

      msg_stream << "   end of distribution phase" << std::endl;
    }

    //----------------------------------------------------------------------------------//
    //  merge and insert phase
    //----------------------------------------------------------------------------------//

    msg_stream << n_tmp_files
               << " temporary files to be processed by merge/insert phase" << std::endl;

    typedef std::vector<detail::file_state<value_type> > vector_type;

    vector_type files;
    files.reserve(n_tmp_files);

    // open each temporary file and set up its current element
    for (std::size_t file_n = 0; file_n < n_tmp_files; ++file_n)
    {
      std::string file_n_string = boost::lexical_cast<std::string>(file_n);
      bfs::path tmp_path = temp_dir / bfs::path("btree.tmp" + file_n_string);
      msg_stream << "      opening " << tmp_path << std::endl;
      files.push_back(detail::file_state<value_type>());
#ifdef BOOST_BTREE_CSTDIO
      files[file_n].stream_ptr = std::fopen(tmp_path.string().c_str(), "rb");
      if (!files[file_n].stream_ptr)
        throw std::runtime_error(std::string("open failure: ") + tmp_path.string());
      if (std::setvbuf(files[file_n].stream_ptr, 0, _IOFBF, 4096))
        throw std::runtime_error(std::string("setvbuf failure: ") + tmp_path.string());
      if (std::fread(&files[file_n].element,
        sizeof(value_type), 1, files[file_n].stream_ptr) != 1)
          throw std::runtime_error(std::string("read failure: ") + tmp_path.string());
#else
      files[file_n].stream_ptr->open(tmp_path.string().c_str(), std::ios_base::binary);
      files[file_n].stream_ptr->read(reinterpret_cast<char*>(&files[file_n].element),
        sizeof(value_type));
#endif
      files[file_n].bytes_left = static_cast<std::size_t>(bfs::file_size(tmp_path.c_str()))
        - sizeof(value_type);
    }

    uint64_t emplace_calls = 0;
    uint64_t inserts = 0;
    //  hold most recent iterator to minimize cache thrashing; may become unneccessary
    //  if an packed_insert is implemented, but still needed if insert/emplace with
    //  hint implemented.
    std::pair<typename BTree::const_iterator, bool> result;  

    t.start();
    timer::cpu_times then = t.elapsed();

    //  until all elements done, insert minimum element
    //    note well: stable due to stable_sort of each file, and then min_element
    //    order guarantee that files will be checked in begin to end order
    while (!files.empty())
    {
      typename vector_type::iterator min = std::min_element(files.begin(), files.end());

      result = bt.emplace(min->element.key, min->element.mapped);
      ++emplace_calls;

      if (result.second)
      {
        ++inserts;
        BOOST_ASSERT(bt.inspect_leaf_to_root(msg_stream, result.first));
      }

      if (log_point && emplace_calls % log_point == 0)
      {
        const double sec = 1000000000.0;
        t.stop();
        timer::cpu_times now = t.elapsed();
        msg_stream << "    " << emplace_calls << " emplace calls, "
                             << inserts << " inserts, "
        //msg_stream << ", this one from file " << (min - files.begin())
        //           << " of " << files.size()-1 << std::endl;
                   << (now.wall-then.wall)/sec << " secs, "
                   << log_point / ((now.wall-then.wall)/sec) << " per sec"
                   << std::endl;
        then = now;
        t.resume();
      }

      if (min->bytes_left)
      {
#ifdef BOOST_BTREE_CSTDIO
      if (std::fread(&min->element, sizeof(value_type), 1, min->stream_ptr) != 1)
        throw std::runtime_error("temp file read failure");
#else
        min->stream_ptr->read(
          reinterpret_cast<char*>(&min->element), sizeof(value_type));
#endif
        min->bytes_left -= sizeof(value_type);
      }
      else
      {
#ifdef BOOST_BTREE_CSTDIO
        std::fclose(min->stream_ptr);
#else
        min-stream_ptr->close();
#endif
        files.erase(min);
      }
    }
    t.stop();
    t.report();

    msg_stream << emplace_calls << " emplace calls, " << inserts << " inserts\n";

    msg_stream << bt << std::endl;
    msg_stream << bt.manager() << std::endl;

    BOOST_ASSERT(emplace_calls == n_elements);
    // TODO throw if inserts != n_elements

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
