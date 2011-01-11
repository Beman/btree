/*--------------------------------------------------------------------------------------//

Support for variable length keys and/or data.

This code probably will become part of another header or headers.

Approach:

*  If is_pointer<key_type>, length varies.

*  static_assert(is_pointer<key_type> == is_pointer<mapped_type>)

*  Use case: btree_map<char*, char*>
             btree_set<char*>

//--------------------------------------------------------------------------------------*/


// See http://www.gotw.ca/publications/mill17.htm,
// Why Not Specialize: The Dimov/Abrahams Example

template <class T>
std::size_t size(const T&) { return sizeof(T); }

template <class T>
std::size_t size(const T*);  // pointers must be overloaded; if this overload is selected
                             // it means user failed to provide the required overload

std::size_t size(const char* x) { return std::strlen(x) + 1; }

