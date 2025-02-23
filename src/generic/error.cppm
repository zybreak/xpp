module;
#include <xcb/xcb.h>
export module xpp.generic.error;

import std;

export namespace xpp {
    namespace generic {

        class error_dispatcher {
          public:
            virtual void operator()(std::shared_ptr<xcb_generic_error_t> const &) const = 0;
        };

        namespace detail {

            template <typename Object>
            void
            dispatch(Object const &object,
                     std::shared_ptr<xcb_generic_error_t> const &error,
                     std::true_type) {
                static_cast<xpp::generic::error_dispatcher const &>(object)(error);
            }

            template <typename Object>
            void
            dispatch(Object const &,
                     std::shared_ptr<xcb_generic_error_t> const &error,
                     std::false_type) {
                throw error;
            }

        }  // namespace detail

        template <typename Object>
        void
        dispatch(Object const &object,
                 std::shared_ptr<xcb_generic_error_t> const &error) {
            detail::dispatch(object,
                             error,
                             std::is_base_of<xpp::generic::error_dispatcher, Object>());
        }

        template <typename Derived, typename Error>
        class error
            : public std::runtime_error {
          public:
            error(std::shared_ptr<xcb_generic_error_t> const &error)
                : runtime_error(get_error_description(error.get())), m_error(error) {
            }

            virtual ~error(void) {
            }

            virtual
            operator Error const &(void) const {
                return reinterpret_cast<Error const &>(*m_error);
            }

            virtual Error const &
            operator*(void) const {
                return reinterpret_cast<Error const &>(*m_error);
            }

            virtual Error *
            operator->(void) const {
                return reinterpret_cast<Error *const>(m_error.get());
            }

          protected:
            virtual std::string
            get_error_description(xcb_generic_error_t *error) const {
                return std::string(Derived::description()) + " (" + std::to_string(error->error_code) + ")";
            }

            std::shared_ptr<xcb_generic_error_t> m_error;
        };  // class error

    }  // namespace generic
}  // namespace xpp
