module;
#include <xcb/xcb.h>
export module xpp.generic.extension;

import std;

export namespace xpp {
    namespace generic {

        template <typename Derived, xcb_extension_t *Id>
        class extension {
          public:
            extension(xcb_connection_t *const c)
                : m_c(c) {
                prefetch();
            }

            xcb_query_extension_reply_t const &
            operator*(void) const {
                return *m_extension;
            }

            xcb_query_extension_reply_t const *
            operator->(void) const {
                return m_extension;
            }

            operator xcb_query_extension_reply_t const *(void) const {
                return m_extension;
            }

            Derived &
            get(void) {
                m_extension = xcb_get_extension_data(m_c, Id);
                return static_cast<Derived &>(*this);
            }

            Derived &
            prefetch(void) {
                xcb_prefetch_extension_data(m_c, Id);
                return static_cast<Derived &>(*this);
            }

          private:
            xcb_connection_t *m_c = nullptr;
            // The result must not be freed.
            // This storage is managed by the cache itself.
            xcb_query_extension_reply_t const *m_extension = nullptr;
        };  // class extension

    }  // namespace generic
}  // namespace xpp
