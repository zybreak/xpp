module;
#include <xcb/xcb.h>
export module xpp.generic.reply_iterator;

import std;
import xpp.generic.factory;
import xpp.generic.signature;
import xpp.generic.iterator_traits;

export namespace xpp {

    namespace generic {

        template <typename Data>
        class get {
          public:
            Data
            operator()(Data* const data) {
                return *data;
            }
        };

        template <>
        class get<xcb_str_t> {
          public:
            std::string
            operator()(xcb_str_t* const data) {
                return std::string(xcb_str_name(data),
                                   xcb_str_name_length(data));
            }
        };

        namespace detail {

            template <typename F>
            struct function_traits;

            template <typename Signature, Signature& S>
            struct function_traits<signature<Signature, S>> : function_traits<Signature> {};

            template <typename R, typename... Args>
            struct function_traits<R (*)(Args...)> : function_traits<R(Args...)> {};

            template <typename R, typename... Args>
            struct function_traits<R(Args...)> {
                using result_type = R;
                static std::size_t const arity = sizeof...(Args);

                template <std::size_t I>
                struct argument {
                    static_assert(I < arity, "invalid argument index");
                    using type = typename std::tuple_element<I, std::tuple<Args...>>::type;
                };
            };
        }  // namespace detail

        // iterator for variable size data fields

        template <typename... Types>
        class iterator;

        template <typename Connection,
                  typename Object,
                  typename NextTemplate,
                  NextTemplate& Next,
                  typename SizeOfTemplate,
                  SizeOfTemplate& SizeOf,
                  typename GetIteratorTemplate,
                  GetIteratorTemplate& GetIterator>
        class iterator<Connection,
                       Object,
                       xpp::generic::signature<NextTemplate, Next>,
                       xpp::generic::signature<SizeOfTemplate, SizeOf>,
                       xpp::generic::signature<GetIteratorTemplate, GetIterator>> {
          protected:
            using self = iterator<Connection,
                                  Object,
                                  xpp::generic::signature<NextTemplate, Next>,
                                  xpp::generic::signature<SizeOfTemplate, SizeOf>,
                                  xpp::generic::signature<GetIteratorTemplate, GetIterator>>;

            using get_iterator_traits = detail::function_traits<GetIteratorTemplate>;
            using const_reply_ptr = typename get_iterator_traits::template argument<0>::type;
            using Reply = typename std::remove_pointer<typename std::remove_const<const_reply_ptr>::type>::type;
            using XcbIterator = typename get_iterator_traits::result_type;

            Connection m_c;
            std::shared_ptr<Reply> m_reply;
            std::stack<std::size_t> m_lengths;
            XcbIterator m_iterator;

          public:
            typedef Object value_type;
            typedef typename std::size_t difference_type;
            typedef Object* pointer;
            typedef Object const& reference;
            typedef typename std::input_iterator_tag iterator_category;

            iterator(void) {
            }

            template <typename C>
            iterator(C&& c, std::shared_ptr<Reply> const& reply)
                : m_c(std::forward<C>(c)), m_reply(reply), m_iterator(GetIterator(reply.get())) {
            }

            bool
            operator==(iterator const& other) {
                return m_iterator.rem == other.m_iterator.rem;
            }

            bool
            operator!=(iterator const& other) {
                return !(*this == other);
            }

            auto
            operator*(void) -> decltype(get<Object>()(this->m_iterator.data)) {
                return get<Object>()(m_iterator.data);
            }

            // prefix
            self&
            operator++(void) {
                m_lengths.push(SizeOf(m_iterator.data));
                Next(&m_iterator);
                return *this;
            }

            // postfix
            self
            operator++(int) {
                auto copy = *this;
                ++(*this);
                return copy;
            }

            // prefix
            self&
            operator--(void) {
                typedef typename std::remove_pointer<decltype(m_iterator.data)>::type data_t;
                if (m_lengths.empty()) {
                    data_t* data = m_iterator.data;
                    data_t* prev = data - m_lengths.top();
                    m_lengths.pop();
                    m_iterator.index = (char*)m_iterator.data - (char*)prev;
                    m_iterator.data = prev;
                    ++m_iterator.rem;
                }
                return *this;
            }

            // postfix
            self
            operator--(int) {
                auto copy = *this;
                --(*this);
                return copy;
            }

            template <typename C>
            static self
            begin(C&& c, std::shared_ptr<Reply> const& reply) {
                return self{std::forward<C>(c), reply};
            }

            template <typename C>
            static self
            end(C&& c, std::shared_ptr<Reply> const& reply) {
                auto it = self{std::forward<C>(c), reply};
                it.m_iterator.rem = 0;
                return it;
            }
        };  // class iterator

        // iterator for fixed size data fields

        template <typename Connection,
                  typename Object,
                  typename AccessorTemplate,
                  AccessorTemplate& Accessor,
                  typename LengthTemplate,
                  LengthTemplate& Length>
        class iterator<Connection,
                       Object,
                       signature<AccessorTemplate, Accessor>,
                       signature<LengthTemplate, Length>> {
          protected:
            using accessor_traits = detail::function_traits<AccessorTemplate>;
            using Data = typename std::remove_pointer<typename accessor_traits::result_type>::type;
            using const_reply_ptr = typename accessor_traits::template argument<0>::type;
            using Reply = typename std::remove_pointer<typename std::remove_const<const_reply_ptr>::type>::type;

            using data_t = typename std::conditional<std::is_void<Data>::value,
                                                     typename xpp::generic::conversion_type<Object>::type, Data>::type;
            using make = xpp::generic::factory::make<Connection, data_t, Object>;

            Connection m_c;
            std::size_t m_index = 0;
            std::shared_ptr<Reply> m_reply;

          public:
            typedef Object value_type;
            typedef typename std::size_t difference_type;
            typedef Object* pointer;
            typedef Object const& reference;
            typedef typename std::input_iterator_tag iterator_category;

            typedef iterator<Connection,
                             Object,
                             signature<AccessorTemplate, Accessor>,
                             signature<LengthTemplate, Length>>
                self;

            iterator(void) {
            }

            template <typename C>
            iterator(C&& c,
                     std::shared_ptr<Reply> const& reply,
                     std::size_t index)
                : m_c(c), m_index(index), m_reply(reply) {
                if (std::is_void<Data>::value) {
                    m_index /= sizeof(data_t);
                }
            }

            bool operator==(iterator const& other) {
                return m_index == other.m_index;
            }

            bool operator!=(iterator const& other) {
                return !(*this == other);
            }

            Object operator*(void) {
                return make()(m_c, static_cast<data_t*>(Accessor(m_reply.get()))[m_index]);
            }

            // prefix
            self& operator++(void) {
                ++m_index;
                return *this;
            }

            // postfix
            self operator++(int) {
                auto copy = *this;
                ++(*this);
                return copy;
            }

            // prefix
            self& operator--(void) {
                --m_index;
                return *this;
            }

            // postfix
            self operator--(int) {
                auto copy = *this;
                --(*this);
                return copy;
            }

            template <typename C>
            static self
            begin(C&& c, std::shared_ptr<Reply> const& reply) {
                return self{std::forward<C>(c), reply, 0};
            }

            template <typename C>
            static self
            end(C&& c, std::shared_ptr<Reply> const& reply) {
                return self{std::forward<C>(c),
                            reply,
                            static_cast<std::size_t>(Length(reply.get()))};
            }
        };  // class iterator

        template <typename Connection, typename Reply, typename Iterator>
        class list {
          private:
            // before public part, to make decltype in begin() & end() work!
            Connection m_c;
            std::shared_ptr<Reply> m_reply;

          public:
            template <typename C>
            list(C&& c, std::shared_ptr<Reply> const& reply)
                : m_c(std::forward<C>(c)), m_reply(reply) {
            }

            auto
            begin(void) -> decltype(Iterator::begin(this->m_c, this->m_reply)) {
                return Iterator::begin(m_c, m_reply);
            }

            auto
            end(void) -> decltype(Iterator::end(this->m_c, this->m_reply)) {
                return Iterator::end(m_c, m_reply);
            }
        };  // class list

    }  // namespace generic

}  // namespace xpp
