module;
#include <xcb/xcb.h>
export module xpp.generic.iterator_traits;

import std;

export namespace xpp {

    namespace generic {

        template <typename T>
        struct traits {
            typedef T type;
        };

        template <typename Object>
        struct conversion_type {
            using type = typename traits<Object>::type;
        };

    }  // namespace generic

}  // namespace xpp
