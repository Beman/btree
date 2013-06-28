//  boost/btree/bulk_loader.hpp  -------------------------------------------------------//

//  Copyright Beman Dawes 2013

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_BTREE_BULK_LOADER_HPP
#define BOOST_BTREE_BULK_LOADER_HPP

#include <boost/btree/set.hpp>
#include <boost/btree/map.hpp>
#include <algorithm>

namespace boost
{
namespace btree
{
  template <class T>
  class bulk_loader
  {
  public:
    typedef T value_type;

    explicit bulk_loader(std::size_t avail_memory)
      : m_available_memory(avail_memory) {}

    template <class InputIterator, class BT>
    void load(InputIterator begin, InputIterator end, BT& btree, std::ostream& msg); 

    int main(int argc, char * argv[], std::ostream& os);

    // observers
    std::size_t available_memory() const {return m_available_memory;}

  private:
    std::size_t m_available_memory;
  };

  //------------------------------------------------------------------------------------//
  //                                  implementation                                    //
  //------------------------------------------------------------------------------------//

  //-------------------------------------  load  ---------------------------------------//

    template <class InputIterator, class BT>
    void bulk_loader<T>::load(InputIterator begin, InputIterator end, BT& btree)
    {
      std::size_t max_size = available_memory() / sizeof(T);
      int n_files = 0;

      // TODO: put the files in a uniquely named directory.

      typedef std::vector<T> vector_type;
      vector_type v;
      v.reserve(max_size);

      //  load the vector, sort it, write contents to tempoary file
      while (being != end)
      {
        ++n_files;
        std::string path("sort");
        path += std::to_string(files);
        path += ".tmp";
        std::ofstream out(path, std::ios_base::out | std::ios_base::binary);
        if (!out)
        {
          throw std::runtime_error(std::string("Could not open ") + path);
        }

        // load the vector
        for (; v.size() < max_size && begin != end; ++begin)
        {
          v.push_back(*begin);
        }

        // sort the vector
        std::stable_sort(v.begin(), v.end());

        // write out the vector
        out.write(reinterpret_cast<const char*>(v.data()), v.size()*sizeof(T));
        out.close();

        v.clear();

        msg << "  file " << n_files << " created" << std::endl;
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
