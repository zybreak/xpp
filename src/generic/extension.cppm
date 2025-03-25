module;
#include <xcb/xcb.h>
export module xpp.generic.extension;

import std;

export namespace xpp::generic {

    class extension {
      public:
        explicit extension(xcb_connection_t *c, xcb_extension_t *id)
            : m_c(c), id(id), m_extension(xcb_get_extension_data(c, id)) {
            if (m_extension->present == 0) {
                throw std::runtime_error("extension not present");
            }
        }

        xcb_query_extension_reply_t const &operator*() const {
            return *m_extension;
        }

        xcb_query_extension_reply_t const *operator->() const {
            return m_extension;
        }

        operator xcb_query_extension_reply_t const *() const {
            return m_extension;
        }

        auto prefetch(this auto &self) -> decltype(self) {
            xcb_prefetch_extension_data(self.m_c, self.id);
            return self;
        }

      protected:
        xcb_connection_t *get_connection() const {
            return m_c;
        }

      private:
        xcb_connection_t *m_c{nullptr};
        xcb_extension_t *id{nullptr};
        // The result must not be freed.
        // This storage is managed by the cache itself.
        xcb_query_extension_reply_t const *m_extension{nullptr};
    };  // class extension

}  // namespace xpp::generic
