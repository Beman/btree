/*
   Copyright Beman Dawes 2013
   Copyright (c) Marshall Clow 2012-2012.

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

    For more information, see http://www.boost.org

    Based on the StringRef implementation in LLVM (http://llvm.org) and
    N3422 by Jeffrey Yasskin
        http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3442.html

*/

#ifndef BOOST_STRING_BOX_HPP
#define BOOST_STRING_BOX_HPP

#include <boost/config.hpp>
#include <boost/detail/workaround.hpp>
#include <boost/static_assert.hpp>

#include <cstddef>
#include <stdexcept>
#include <algorithm>
#include <iterator>
#include <string>
#include <cstring>
#include <climits>
#include <iosfwd>

namespace boost {

    template<typename charT, unsigned MaxLen,
      typename traits = std::char_traits<charT> > class basic_string_box;
    //typedef basic_string_box<char,     std::char_traits<char> >        string_box;
    //typedef basic_string_box<wchar_t,  std::char_traits<wchar_t> >    wstring_box;

//#ifndef BOOST_NO_CXX11_CHAR16_T
//    typedef basic_string_box<char16_t, std::char_traits<char16_t> > u16string_box;
//#endif
//
//#ifndef BOOST_NO_CXX11_CHAR32_T
//    typedef basic_string_box<char32_t, std::char_traits<char32_t> > u32string_box;
//#endif

    namespace detail {
    //  A helper functor because sometimes we don't have lambdas
        template <typename charT, unsigned MaxLen, typename traits>
        class string_box_traits_eq {
        public:
            string_box_traits_eq ( charT ch ) : ch_(ch) {}
            bool operator () ( charT val ) const { return traits::eq ( ch_, val ); }
            charT ch_;
            };
        }

    template<typename charT, unsigned MaxLen, typename traits>
    class basic_string_box {
      BOOST_STATIC_ASSERT_MSG(MaxLen <= UCHAR_MAX,         // draw the line, arbitrarily for
        "string_box MaxLen too large; max is UCHAR_MAX");  // charT other than char itself
                                                            

        unsigned char   len_;   // length in bytes; invariant: len_ <= MaxLen
          // remember there may be alignment padding bytes here if charT not char
        charT           rep_[MaxLen];
      public:
        // types
        typedef charT value_type;
        typedef const charT* pointer;
        typedef const charT& reference;
        typedef const charT& const_reference;
        typedef pointer const_iterator; // impl-defined
        typedef const_iterator iterator;
        typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
        typedef const_reverse_iterator reverse_iterator;
        typedef std::size_t size_type;
        typedef std::ptrdiff_t difference_type;
        static BOOST_CONSTEXPR_OR_CONST size_type npos = size_type(-1);

        // construct/copy
        BOOST_CONSTEXPR basic_string_box ()
            : len_(0) {}

        BOOST_CONSTEXPR basic_string_box (const basic_string_box &rhs) {
            std::memcpy(&rep_, &rhs.rep_, rhs.len_);
            len_ = rhs.len_;
            }
            

        basic_string_box& operator=(const basic_string_box &rhs) {
            if (rep_ != rhs.rep_) { // memcpy may fail if bytes overlap, so test self asgn
                std::memcpy(&rep_, &rhs.rep_, rhs.len_);
                len_ = rhs.len_;
                }
            return *this;
            }

        basic_string_box(const charT* str) {
            size_type sz = traits::length(str);
            if (sz > UCHAR_MAX)
              { BOOST_STRING_BOX_THROW(
                std::runtime_error("string_box ctor string too long, max is UCHAR_MAX"); }
            std::memcpy(rep_, str, sz);
            len_ = sz;
            }

        template<typename Allocator>
        basic_string_box(const std::basic_string<charT, traits, Allocator>& str) {
            if (str.length() > UCHAR_MAX)
              { BOOST_STRING_BOX_THROW(
                std::runtime_error("string_box op= string too long, max is UCHAR_MAX"); }
            std::memcpy(rep_, str.data(), str.length());
            len_ = str.length();
            }


        BOOST_CONSTEXPR basic_string_box(const charT* str, size_type len) (
            if (len > UCHAR_MAX)
              { BOOST_STRING_BOX_THROW(
                std::runtime_error("string_box op= string too long, max is UCHAR_MAX"); }
            std::memcpy(rep_, str, len);
            len_ = len;
            }

#ifndef BOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS
        template<typename Allocator>
        explicit operator std::basic_string<charT, traits, Allocator>() const {
            return std::basic_string<charT, traits, Allocator> ( data(), length() );
            }
#endif

        std::basic_string<charT, traits> to_string () const {
            return std::basic_string<charT, traits> ( data(), length() );
            }

        // iterators
        BOOST_CONSTEXPR const_iterator   begin() const { return ptr_; }
        BOOST_CONSTEXPR const_iterator  cbegin() const { return ptr_; }
        BOOST_CONSTEXPR const_iterator     end() const { return ptr_ + len_; }
        BOOST_CONSTEXPR const_iterator    cend() const { return ptr_ + len_; }
                const_reverse_iterator  rbegin() const { return const_reverse_iterator (end()); }
                const_reverse_iterator crbegin() const { return const_reverse_iterator (end()); }
                const_reverse_iterator    rend() const { return const_reverse_iterator (begin()); }
                const_reverse_iterator   crend() const { return const_reverse_iterator (begin()); }

        // capacity
        BOOST_CONSTEXPR size_type size()     const { return len_; }
        BOOST_CONSTEXPR size_type length()   const { return len_; }
        BOOST_CONSTEXPR size_type max_size() const { return len_; }
        BOOST_CONSTEXPR bool empty()         const { return len_ == 0; }

        // element access
        BOOST_CONSTEXPR const charT& operator[](size_type pos) const { return ptr_[pos]; }

        const charT& at(size_t pos) const {
            if ( pos >= len_ )
                BOOST_THROW_EXCEPTION( std::out_of_range ( "boost::string_box::at" ) );
            return ptr_[pos];
            }

        BOOST_CONSTEXPR const charT& front() const { return ptr_[0]; }
        BOOST_CONSTEXPR const charT& back()  const { return ptr_[len_-1]; }
        BOOST_CONSTEXPR const charT* data()  const { return ptr_; }

        // modifiers
        void clear() { len_ = 0; }
        void remove_prefix(size_type n) {
            if ( n > len_ )
                n = len_;
            ptr_ += n;
            len_ -= n;
            }

        void remove_suffix(size_type n) {
            if ( n > len_ )
                n = len_;
            len_ -= n;
            }


        // basic_string_box string operations
        basic_string_box substr(size_type pos, size_type n=npos) const {
            if ( pos > size())
                BOOST_THROW_EXCEPTION( std::out_of_range ( "string_box::substr" ) );
            if ( n == npos || pos + n > size())
                n = size () - pos;
            return basic_string_box ( data() + pos, n );
            }

        int compare(basic_string_box x) const {
            const int cmp = traits::compare ( ptr_, x.ptr_, (std::min)(len_, x.len_));
            return cmp != 0 ? cmp : ( len_ == x.len_ ? 0 : len_ < x.len_ ? -1 : 1 );
            }

        bool starts_with(charT c) const { return !empty() && traits::eq ( c, front()); }
        bool starts_with(basic_string_box x) const {
            return len_ >= x.len_ && traits::compare ( ptr_, x.ptr_, x.len_ ) == 0;
            }

        bool ends_with(charT c) const { return !empty() && traits::eq ( c, back()); }
        bool ends_with(basic_string_box x) const {
            return len_ >= x.len_ && traits::compare ( ptr_ + len_ - x.len_, x.ptr_, x.len_ ) == 0;
            }

        size_type find(basic_string_box s) const {
            const_iterator iter = std::search ( this->cbegin (), this->cend (),
                                                s.cbegin (), s.cend (), traits::eq );
            return iter == this->cend () ? npos : std::distance ( this->cbegin (), iter );
            }

        size_type find(charT c) const {
            const_iterator iter = std::find_if ( this->cbegin (), this->cend (),
                                    detail::string_box_traits_eq<charT, traits> ( c ));
            return iter == this->cend () ? npos : std::distance ( this->cbegin (), iter );
            }

        size_type rfind(basic_string_box s) const {
            const_reverse_iterator iter = std::search ( this->crbegin (), this->crend (),
                                                s.crbegin (), s.crend (), traits::eq );
            return iter == this->crend () ? npos : reverse_distance ( this->crbegin (), iter );
            }

        size_type rfind(charT c) const {
            const_reverse_iterator iter = std::find_if ( this->crbegin (), this->crend (),
                                    detail::string_box_traits_eq<charT, traits> ( c ));
            return iter == this->crend () ? npos : reverse_distance ( this->crbegin (), iter );
            }

        size_type find_first_of(charT c) const { return  find (c); }
        size_type find_last_of (charT c) const { return rfind (c); }

        size_type find_first_of(basic_string_box s) const {
            const_iterator iter = std::find_first_of
                ( this->cbegin (), this->cend (), s.cbegin (), s.cend (), traits::eq );
            return iter == this->cend () ? npos : std::distance ( this->cbegin (), iter );
            }

        size_type find_last_of(basic_string_box s) const {
            const_reverse_iterator iter = std::find_first_of
                ( this->crbegin (), this->crend (), s.cbegin (), s.cend (), traits::eq );
            return iter == this->crend () ? npos : reverse_distance ( this->crbegin (), iter);
            }

        size_type find_first_not_of(basic_string_box s) const {
            const_iterator iter = find_not_of ( this->cbegin (), this->cend (), s );
            return iter == this->cend () ? npos : std::distance ( this->cbegin (), iter );
            }

        size_type find_first_not_of(charT c) const {
            for ( const_iterator iter = this->cbegin (); iter != this->cend (); ++iter )
                if ( !traits::eq ( c, *iter ))
                    return std::distance ( this->cbegin (), iter );
            return npos;
            }

        size_type find_last_not_of(basic_string_box s) const {
            const_reverse_iterator iter = find_not_of ( this->crbegin (), this->crend (), s );
            return iter == this->crend () ? npos : reverse_distance ( this->crbegin (), iter );
            }

        size_type find_last_not_of(charT c) const {
            for ( const_reverse_iterator iter = this->crbegin (); iter != this->crend (); ++iter )
                if ( !traits::eq ( c, *iter ))
                    return reverse_distance ( this->crbegin (), iter );
            return npos;
            }

    private:
        template <typename r_iter>
        size_type reverse_distance ( r_iter first, r_iter last ) const {
            return len_ - 1 - std::distance ( first, last );
            }

        template <typename Iterator>
        Iterator find_not_of ( Iterator first, Iterator last, basic_string_box s ) const {
            for ( ; first != last ; ++first )
                if ( 0 == traits::find ( s.ptr_, s.len_, *first ))
                    return first;
            return last;
            }
      };


//  Comparison operators
//  Equality
    template<typename charT, typename traits>
    inline bool operator==(basic_string_box<charT, traits> x, basic_string_box<charT, traits> y) {
        if ( x.size () != y.size ()) return false;
        return x.compare(y) == 0;
        }

    template<typename charT, typename traits, typename Allocator>
    inline bool operator==(basic_string_box<charT, traits> x, const std::basic_string<charT, traits, Allocator> & y) {
        return x == basic_string_box<charT, traits>(y);
        }

    template<typename charT, typename traits, typename Allocator>
    inline bool operator==(const std::basic_string<charT, traits, Allocator> & x, basic_string_box<charT, traits> y) {
        return basic_string_box<charT, traits>(x) == y;
        }

    template<typename charT, typename traits>
    inline bool operator==(basic_string_box<charT, traits> x, const charT * y) {
        return x == basic_string_box<charT, traits>(y);
        }

    template<typename charT, typename traits>
    inline bool operator==(const charT * x, basic_string_box<charT, traits> y) {
        return basic_string_box<charT, traits>(x) == y;
        }

//  Inequality
    template<typename charT, typename traits>
    inline bool operator!=(basic_string_box<charT, traits> x, basic_string_box<charT, traits> y) {
        if ( x.size () != y.size ()) return true;
        return x.compare(y) != 0;
        }

    template<typename charT, typename traits, typename Allocator>
    inline bool operator!=(basic_string_box<charT, traits> x, const std::basic_string<charT, traits, Allocator> & y) {
        return x != basic_string_box<charT, traits>(y);
        }

    template<typename charT, typename traits, typename Allocator>
    inline bool operator!=(const std::basic_string<charT, traits, Allocator> & x, basic_string_box<charT, traits> y) {
        return basic_string_box<charT, traits>(x) != y;
        }

    template<typename charT, typename traits>
    inline bool operator!=(basic_string_box<charT, traits> x, const charT * y) {
        return x != basic_string_box<charT, traits>(y);
        }

    template<typename charT, typename traits>
    inline bool operator!=(const charT * x, basic_string_box<charT, traits> y) {
        return basic_string_box<charT, traits>(x) != y;
        }

//  Less than
    template<typename charT, typename traits>
    inline bool operator<(basic_string_box<charT, traits> x, basic_string_box<charT, traits> y) {
        return x.compare(y) < 0;
        }

    template<typename charT, typename traits, typename Allocator>
    inline bool operator<(basic_string_box<charT, traits> x, const std::basic_string<charT, traits, Allocator> & y) {
        return x < basic_string_box<charT, traits>(y);
        }

    template<typename charT, typename traits, typename Allocator>
    inline bool operator<(const std::basic_string<charT, traits, Allocator> & x, basic_string_box<charT, traits> y) {
        return basic_string_box<charT, traits>(x) < y;
        }

    template<typename charT, typename traits>
    inline bool operator<(basic_string_box<charT, traits> x, const charT * y) {
        return x < basic_string_box<charT, traits>(y);
        }

    template<typename charT, typename traits>
    inline bool operator<(const charT * x, basic_string_box<charT, traits> y) {
        return basic_string_box<charT, traits>(x) < y;
        }

//  Greater than
    template<typename charT, typename traits>
    inline bool operator>(basic_string_box<charT, traits> x, basic_string_box<charT, traits> y) {
        return x.compare(y) > 0;
        }

    template<typename charT, typename traits, typename Allocator>
    inline bool operator>(basic_string_box<charT, traits> x, const std::basic_string<charT, traits, Allocator> & y) {
        return x > basic_string_box<charT, traits>(y);
        }

    template<typename charT, typename traits, typename Allocator>
    inline bool operator>(const std::basic_string<charT, traits, Allocator> & x, basic_string_box<charT, traits> y) {
        return basic_string_box<charT, traits>(x) > y;
        }

    template<typename charT, typename traits>
    inline bool operator>(basic_string_box<charT, traits> x, const charT * y) {
        return x > basic_string_box<charT, traits>(y);
        }

    template<typename charT, typename traits>
    inline bool operator>(const charT * x, basic_string_box<charT, traits> y) {
        return basic_string_box<charT, traits>(x) > y;
        }

//  Less than or equal to
    template<typename charT, typename traits>
    inline bool operator<=(basic_string_box<charT, traits> x, basic_string_box<charT, traits> y) {
        return x.compare(y) <= 0;
        }

    template<typename charT, typename traits, typename Allocator>
    inline bool operator<=(basic_string_box<charT, traits> x, const std::basic_string<charT, traits, Allocator> & y) {
        return x <= basic_string_box<charT, traits>(y);
        }

    template<typename charT, typename traits, typename Allocator>
    inline bool operator<=(const std::basic_string<charT, traits, Allocator> & x, basic_string_box<charT, traits> y) {
        return basic_string_box<charT, traits>(x) <= y;
        }

    template<typename charT, typename traits>
    inline bool operator<=(basic_string_box<charT, traits> x, const charT * y) {
        return x <= basic_string_box<charT, traits>(y);
        }

    template<typename charT, typename traits>
    inline bool operator<=(const charT * x, basic_string_box<charT, traits> y) {
        return basic_string_box<charT, traits>(x) <= y;
        }

//  Greater than or equal to
    template<typename charT, typename traits>
    inline bool operator>=(basic_string_box<charT, traits> x, basic_string_box<charT, traits> y) {
        return x.compare(y) >= 0;
        }

    template<typename charT, typename traits, typename Allocator>
    inline bool operator>=(basic_string_box<charT, traits> x, const std::basic_string<charT, traits, Allocator> & y) {
        return x >= basic_string_box<charT, traits>(y);
        }

    template<typename charT, typename traits, typename Allocator>
    inline bool operator>=(const std::basic_string<charT, traits, Allocator> & x, basic_string_box<charT, traits> y) {
        return basic_string_box<charT, traits>(x) >= y;
        }

    template<typename charT, typename traits>
    inline bool operator>=(basic_string_box<charT, traits> x, const charT * y) {
        return x >= basic_string_box<charT, traits>(y);
        }

    template<typename charT, typename traits>
    inline bool operator>=(const charT * x, basic_string_box<charT, traits> y) {
        return basic_string_box<charT, traits>(x) >= y;
        }

    namespace detail {

        template<class charT, class traits>
        inline void insert_fill_chars(std::basic_ostream<charT, traits>& os, std::size_t n) {
            enum { chunk_size = 8 };
            charT fill_chars[chunk_size];
            std::fill_n(fill_chars, static_cast< std::size_t >(chunk_size), os.fill());
            for (; n >= chunk_size && os.good(); n -= chunk_size)
                os.write(fill_chars, static_cast< std::size_t >(chunk_size));
            if (n > 0 && os.good())
                os.write(fill_chars, n);
            }

        template<class charT, class traits>
        void insert_aligned(std::basic_ostream<charT, traits>& os, const basic_string_box<charT,traits>& str) {
            const std::size_t size = str.size();
            const std::size_t alignment_size = static_cast< std::size_t >(os.width()) - size;
            const bool align_left = (os.flags() & std::basic_ostream<charT, traits>::adjustfield) == std::basic_ostream<charT, traits>::left;
            if (!align_left) {
                detail::insert_fill_chars(os, alignment_size);
                if (os.good())
                    os.write(str.data(), size);
                }
            else {
                os.write(str.data(), size);
                if (os.good())
                    detail::insert_fill_chars(os, alignment_size);
                }
            }

        } // namespace detail

    // Inserter
    template<class charT, class traits>
    inline std::basic_ostream<charT, traits>&
    operator<<(std::basic_ostream<charT, traits>& os, const basic_string_box<charT,traits>& str) {
        if (os.good()) {
            const std::size_t size = str.size();
            const std::size_t w = static_cast< std::size_t >(os.width());
            if (w <= size)
                os.write(str.data(), size);
            else
                detail::insert_aligned(os, str);
            os.width(0);
            }
        return os;
        }

#if 0
    // numeric conversions
    //
    //  These are short-term implementations.
    //  In a production environment, I would rather avoid the copying.
    //
    inline int stoi (string_box str, size_t* idx=0, int base=10) {
        return std::stoi ( std::string(str), idx, base );
        }

    inline long stol (string_box str, size_t* idx=0, int base=10) {
        return std::stol ( std::string(str), idx, base );
        }

    inline unsigned long stoul (string_box str, size_t* idx=0, int base=10) {
        return std::stoul ( std::string(str), idx, base );
        }

    inline long long stoll (string_box str, size_t* idx=0, int base=10) {
        return std::stoll ( std::string(str), idx, base );
        }

    inline unsigned long long stoull (string_box str, size_t* idx=0, int base=10) {
        return std::stoull ( std::string(str), idx, base );
        }

    inline float stof (string_box str, size_t* idx=0) {
        return std::stof ( std::string(str), idx );
        }

    inline double stod (string_box str, size_t* idx=0) {
        return std::stod ( std::string(str), idx );
        }

    inline long double stold (string_box str, size_t* idx=0)  {
        return std::stold ( std::string(str), idx );
        }

    inline int  stoi (wstring_box str, size_t* idx=0, int base=10) {
        return std::stoi ( std::wstring(str), idx, base );
        }

    inline long stol (wstring_box str, size_t* idx=0, int base=10) {
        return std::stol ( std::wstring(str), idx, base );
        }

    inline unsigned long stoul (wstring_box str, size_t* idx=0, int base=10) {
        return std::stoul ( std::wstring(str), idx, base );
        }

    inline long long stoll (wstring_box str, size_t* idx=0, int base=10) {
        return std::stoll ( std::wstring(str), idx, base );
        }

    inline unsigned long long stoull (wstring_box str, size_t* idx=0, int base=10) {
        return std::stoull ( std::wstring(str), idx, base );
        }

    inline float  stof (wstring_box str, size_t* idx=0) {
        return std::stof ( std::wstring(str), idx );
        }

    inline double stod (wstring_box str, size_t* idx=0) {
        return std::stod ( std::wstring(str), idx );
        }

    inline long double stold (wstring_box str, size_t* idx=0) {
        return std::stold ( std::wstring(str), idx );
        }
#endif

}

#if 0
namespace std {
    // Hashing
    template<> struct hash<boost::string_box>;
    template<> struct hash<boost::u16string_box>;
    template<> struct hash<boost::u32string_box>;
    template<> struct hash<boost::wstring_box>;
}
#endif

#endif
