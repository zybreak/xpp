#ifndef X_REQUEST_HPP
#define X_REQUEST_HPP

#include <memory>
#include <xcb/xcb.h>

namespace xpp {

namespace generic {

template<typename COOKIE,
         typename REPLY,
         REPLY * (*REPLY_FUN)(xcb_connection_t *, COOKIE, xcb_generic_error_t **)>
class request {
  public:
    template<typename ... COOKIE_ARGS>
    request(xcb_connection_t * const c,
            COOKIE (*cookie_fun)(xcb_connection_t *, COOKIE_ARGS ...),
            COOKIE_ARGS ... cookie_args)
      : m_c(c)
    {
      prepare(cookie_fun, cookie_args ...);
    }

    const REPLY * const operator*(void)
    {
      return this->get().get();
    }

    const REPLY * const operator->(void)
    {
      return *(*this);
    }

    std::shared_ptr<REPLY>
    get(void)
    {
      if (! m_reply) {
        m_reply = std::shared_ptr<REPLY>(REPLY_FUN(m_c, m_cookie, NULL));
      }
      return m_reply;
    }

    void reset(void)
    {
      m_reply.reset();
    }

  protected:
    xcb_connection_t * m_c;
    COOKIE m_cookie;
    std::shared_ptr<REPLY> m_reply;

    request(xcb_connection_t * const c)
      : m_c(c)
    {}

    template<typename ... COOKIE_ARGS>
    void
    prepare(COOKIE (*cookie_fun)(xcb_connection_t *, COOKIE_ARGS ...),
            COOKIE_ARGS ... cookie_args)
    {
      m_cookie = cookie_fun(m_c, cookie_args ...);
    }

    xcb_connection_t * const
    connection(void)
    {
      return m_c;
    }
};

}; // namespace generic

}; // namespace xpp

#endif // X_REQUEST_HPP
