module;
#include <xcb/xcb.h>
export module xpp.core;

import std;

export namespace xpp {

    class connection_error
        : public std::runtime_error {
      public:
        connection_error(uint8_t code, std::string const &description)
            : std::runtime_error(description + "(" + std::to_string(code) + ")"), m_code(code), m_description(description) {
        }

        uint8_t
        code() {
            return m_code;
        }

        std::string
        description() {
            return m_description;
        }

      protected:
        uint8_t m_code;
        std::string m_description;
    };

}  // namespace xpp
