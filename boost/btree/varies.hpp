/*--------------------------------------------------------------------------------------//

Support for variable length keys and/or data.

This code probably will become part of another header or headers.

Approach:

*  If is_pointer<key_type>, length varies.

*  static_assert(is_pointer<key_type> == is_pointer<mapped_type>)

*  Use case: btree_map<char*, char*>
             btree_set<char*>

//--------------------------------------------------------------------------------------*/


//  See http://www.gotw.ca/publications/mill17.htm,
//  Why Not Specialize: The Dimov/Abrahams Example

template <class T>
std::size_t size(const T&) { return sizeof(T); }

template <class T>
std::size_t size(const T*);  // pointers must be overloaded; if this overload is selected
                             // it means user failed to provide the required overload

std::size_t size(const char* x) { return std::strlen(x) + 1; }
std::size_t size(char* x) { return std::strlen(x) + 1; }

//  less function object class

template <class T> struct less
{
  typedef T first_argument_type;
  typedef T second_argument_type;
  typedef bool result_type;
  bool operator()(const T& x, const T& y) const { return x < y; }
};

//  partial specialization to poison pointer that hasn't been fully specialized
template <class T> struct less<const T*>
{
  typedef T first_argument_type;
  typedef T second_argument_type;
  typedef bool result_type;
  bool operator()(const T& x, const T& y) const;
};

//  full specialization for C strings
template <> struct less<char*>
{
  typedef const char* first_argument_type;
  typedef const char* second_argument_type;
  typedef bool result_type;
  bool operator()(const char* x, const char* y) const { return std::strcmp(x, y) < 0; }
};
