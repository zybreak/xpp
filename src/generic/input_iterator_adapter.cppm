module;
#include <xcb/xcb.h>

export module xpp.generic.input_iterator_adapter;

import std;

export {
    template<typename T, bool B>
    class HasMember_first {
      private:
        using Yes = char[2];
        using No = char[1];
        struct Fallback {
            int first;
        };
        struct Derived : T,
                         Fallback {};
        template<typename U>
        static No& test(decltype(U::first)*);
        template<typename U>
        static Yes& test(U*);

      public:static constexpr bool RESULT = sizeof(test<Derived>(nullptr)) == sizeof(Yes);
    };
    template<typename T>
    class HasMember_first<T, false> {
      public:static constexpr bool RESULT = false;
    };
    template<typename T>
    struct has_member_first : public std::integral_constant<bool, HasMember_first<T, std::is_class<T>::value>::RESULT> {};
    
    template<typename T, bool B>
    class HasMember_second {
      private:
        using Yes = char[2];
        using No = char[1];
        struct Fallback {
            int second;
        };
        struct Derived : T,
                         Fallback {};
        template<typename U>
        static No& test(decltype(U::second)*);
        template<typename U>
        static Yes& test(U*);

      public:static constexpr bool RESULT = sizeof(test<Derived>(nullptr)) == sizeof(Yes);
    };
    template<typename T>
    class HasMember_second<T, false> {
      public:static constexpr bool RESULT = false;
    };
    template<typename T>
    struct has_member_second : public std::integral_constant<bool, HasMember_second<T, std::is_class<T>::value>::RESULT> {};

    // namespace iterator {

    template<typename Iterator>
    struct value_iterator_base {
        value_iterator_base(Iterator const& iterator)
            : m_iterator(iterator) {
        }

        bool operator==(value_iterator_base const& other) {
            return m_iterator == other.m_iterator;
        }

        bool operator!=(value_iterator_base const& other) {
            return m_iterator != other.m_iterator;
        }

        void operator++(void) {
            ++m_iterator;
        }

        template<typename Key, typename Value>
        Value const& get_value(std::pair<Key, Value> const& pair) {
            return pair.second;
        }

        template<typename Value>
        Value const& get_value(Value const& v) {
            return v;
        }

        Iterator m_iterator;
    };

    template<typename Iterator>
    struct value_iterator_pair : public value_iterator_base<Iterator> {
        typedef value_iterator_base<Iterator> base;
        using base::base;

        typedef typename Iterator::value_type::second_type value_type;
        typedef typename std::iterator_traits<Iterator>::difference_type difference_type;
        typedef typename Iterator::value_type::second_type* pointer;
        typedef typename Iterator::value_type::second_type const& reference;
        typedef typename std::input_iterator_tag iterator_category;

        typename Iterator::value_type::second_type const&
        operator*(void) {
            return base::get_value(*base::m_iterator);
        }
    };

    template<typename Iterator>
    struct value_iterator_integral : public value_iterator_base<Iterator> {
        typedef value_iterator_base<Iterator> base;
        using base::base;

        typedef typename std::iterator_traits<Iterator>::value_type value_type;
        typedef typename std::iterator_traits<Iterator>::difference_type difference_type;
        typedef typename std::iterator_traits<Iterator>::pointer pointer;
        typedef typename std::iterator_traits<Iterator>::reference reference;
        typedef typename std::input_iterator_tag iterator_category;

        typename Iterator::value_type const&
        operator*(void) {
            return base::get_value(*base::m_iterator);
        }
    };

    template<typename Iterator>
    struct value_iterator
        : public std::conditional<
              has_member_first<typename Iterator::value_type>::value && has_member_second<typename Iterator::value_type>::value,
              value_iterator_pair<Iterator>,
              value_iterator_integral<Iterator>>::type {
        typedef typename std::conditional<
            has_member_first<typename Iterator::value_type>::value && has_member_second<typename Iterator::value_type>::value,
            value_iterator_pair<Iterator>,
            value_iterator_integral<Iterator>>::type base;
        using base::base;
    };

    template<typename T, bool B = true>
    struct value_type {
        typedef typename std::conditional<
            has_member_second<typename T::value_type>::value,
            typename T::value_type::second_type,
            typename T::value_type>::type
            type;
    };

    template<typename T>
    struct value_type<T, false> {
        typedef typename std::remove_const<
            typename std::remove_pointer<T>::type>::type type;
    };
};
