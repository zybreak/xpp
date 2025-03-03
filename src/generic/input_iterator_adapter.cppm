module;
#include <xcb/xcb.h>

#define GENERATE_HAS_MEMBER(member)                                                   \
                                                                                      \
    template <typename T, bool B>                                                     \
    class HasMember_##member {                                                        \
      private:                                                                        \
        using Yes = char[2];                                                          \
        using No = char[1];                                                           \
                                                                                      \
        struct Fallback {                                                             \
            int member;                                                               \
        };                                                                            \
        struct Derived : T,                                                           \
                         Fallback {};                                                 \
                                                                                      \
        template <typename U>                                                         \
        static No& test(decltype(U::member)*);                                        \
        template <typename U>                                                         \
        static Yes& test(U*);                                                         \
                                                                                      \
      public:                                                                         \
        static constexpr bool RESULT = sizeof(test<Derived>(nullptr)) == sizeof(Yes); \
    };                                                                                \
                                                                                      \
    template <typename T>                                                             \
    class HasMember_##member<T, false> {                                              \
      public:                                                                         \
        static constexpr bool RESULT = false;                                         \
    };                                                                                \
                                                                                      \
    template <typename T>                                                             \
    struct has_member_##member                                                        \
        : public std::integral_constant<                                              \
              bool,                                                                   \
              HasMember_##member<T, std::is_class<T>::value>::RESULT> {};

export module xpp.generic.input_iterator_adapter;

import std;

export {
    GENERATE_HAS_MEMBER(first)
    GENERATE_HAS_MEMBER(second)

    // namespace iterator {

    template <typename Iterator>
    struct value_iterator_base {
        value_iterator_base(Iterator& iterator)
            : m_iterator(iterator) {
        }

        bool
        operator==(value_iterator_base& other) {
            return m_iterator == other.m_iterator;
        }

        bool
        operator!=(value_iterator_base& other) {
            return m_iterator != other.m_iterator;
        }

        void
        operator++(void) {
            ++m_iterator;
        }

        template <typename Key, typename Value>
        Value const&
        get_value(std::pair<Key, Value>& pair) {
            return pair.second;
        }

        template <typename Value>
        Value const&
        get_value(Value& v) {
            return v;
        }

        Iterator m_iterator;
    };

    template <typename Iterator>
    struct value_iterator_pair
        : public value_iterator_base<Iterator>,
          public std::iterator<typename std::input_iterator_tag,
                               // value_type
                               typename Iterator::value_type::second_type,
                               typename std::iterator_traits<Iterator>::difference_type,
                               // pointer
                               typename Iterator::value_type::second_type*,
                               // reference
                               typename Iterator::value_type::second_type&> {
        typedef value_iterator_base<Iterator> base;
        using base::base;

        typename Iterator::value_type::second_type&
        operator*(void) {
            return base::get_value(*base::m_iterator);
        }
    };

    template <typename Iterator>
    struct value_iterator_integral
        : public value_iterator_base<Iterator>,
          public std::iterator<typename std::input_iterator_tag,
                               typename std::iterator_traits<Iterator>::value_type,
                               typename std::iterator_traits<Iterator>::difference_type,
                               typename std::iterator_traits<Iterator>::pointer,
                               typename std::iterator_traits<Iterator>::reference> {
        typedef value_iterator_base<Iterator> base;
        using base::base;

        typename Iterator::value_type&
        operator*(void) {
            return base::get_value(*base::m_iterator);
        }
    };

    template <typename Iterator>
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

    template <typename T, bool B = true>
    struct value_type {
        typedef typename std::conditional<
            has_member_second<typename T::value_type>::value,
            typename T::value_type::second_type,
            typename T::value_type>::type
            type;
    };

    template <typename T>
    struct value_type<T, false> {
        typedef typename std::remove_const<
            typename std::remove_pointer<T>::type>::type type;
    };
};
