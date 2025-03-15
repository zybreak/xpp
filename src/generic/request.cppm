module;
#include <xcb/xcb.h>

export module xpp.generic.request;

import std;
import xpp.generic.error;
import xpp.generic.signature;

export namespace xpp::generic {

    void check(xcb_connection_t *c, xcb_void_cookie_t const &cookie);

    struct checked_tag {};
    struct unchecked_tag {};

    template<typename... Types>
    class reply;

    template<typename Check,
             typename Reply, typename Cookie, Reply *(&ReplyFunction)(xcb_connection_t *, Cookie, xcb_generic_error_t **), // REPLY_TEMPLATE
             typename... CookieParameter, Cookie (&CookieFunction)(CookieParameter...)> // REPLY_COOKIE_TEMPLATE
    class reply<
        Check,
        xpp::generic::signature<Reply *(xcb_connection_t *, Cookie, xcb_generic_error_t **), ReplyFunction>, // REPLY_SIGNATURE
        xpp::generic::signature<Cookie(CookieParameter...), CookieFunction>> { // REPLY_COOKIE_SIGNATURE
      public:
        template<typename ... Parameter>
        reply(xcb_connection_t *c, Parameter && ...parameter) : m_c(c), m_cookie(cookie(c, std::forward<Parameter>(parameter)...)) {
        }
        
        virtual ~reply() = default;

        operator bool(void) {
            return get().operator bool();
        }

        Reply const &operator*(void) {
            return *get();
        }

        Reply *operator->(void) {
            return get().get();
        }

        std::shared_ptr<Reply> const &get(void) {
            if (!m_reply) {
                m_reply = get(Check());
            }
            return m_reply;
        }
        
        template<typename... Parameter>
            static Cookie
            cookie(Parameter &&...parameter) {
            return CookieFunction(std::forward<Parameter>(parameter)...);
        }

      protected:
        xcb_connection_t *m_c;
        Cookie m_cookie;
        std::shared_ptr<Reply> m_reply;
        
        std::shared_ptr<Reply> get(checked_tag) {
            xcb_generic_error_t *error = nullptr;
            auto reply = std::shared_ptr<Reply>(ReplyFunction(m_c, m_cookie, &error), std::free);
            if (error) {
                dispatch(m_c, std::shared_ptr<xcb_generic_error_t>(error, std::free));
            }
            return reply;
        }

        std::shared_ptr<Reply> get(unchecked_tag) {
            return std::shared_ptr<Reply>(ReplyFunction(m_c, m_cookie, nullptr), std::free);
        }
    };

}  // namespace xpp::generic
