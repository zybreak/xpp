module;
#include <xcb/xcb.h>
export module xpp.generic.error;

import std;
export namespace xpp::generic {

    class error_dispatcher {
      public:
        virtual void operator()(std::shared_ptr<xcb_generic_error_t> const &) const = 0;
    };
};  // namespace xpp::generic

namespace xpp::generic::detail {

    template<typename Object>
    void
    dispatch(Object const &object,
             std::shared_ptr<xcb_generic_error_t> const &error,
             std::true_type) {
        static_cast<xpp::generic::error_dispatcher const &>(object)(error);
    }

    template<typename Object>
    void
    dispatch(Object const &,
             std::shared_ptr<xcb_generic_error_t> const &error,
             std::false_type) {
        throw error;
    }

};  // namespace xpp::generic::detail

export namespace xpp {
    namespace generic {

        template<typename Object>
        void
        dispatch(Object const &object,
                 std::shared_ptr<xcb_generic_error_t> const &error) {
            detail::dispatch(object,
                             error,
                             std::is_base_of<xpp::generic::error_dispatcher, Object>());
        }

        template<typename Error>
        class error : public std::exception {
          public:
            error(std::shared_ptr<xcb_generic_error_t> const &error)
                : m_error(error) {
            }

            virtual ~error(void) {
            }

            virtual std::string_view description() const noexcept = 0;

            virtual char const *what() const noexcept {
                static std::string desc = std::string(description()) + " (" + std::to_string(m_error.get()->error_code) + ")";
                return desc.c_str();
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
            std::shared_ptr<xcb_generic_error_t> m_error;
        };  // class error

    }  // namespace generic
}  // namespace xpp
