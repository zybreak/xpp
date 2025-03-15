module;
#include <xcb/xcb.h>
export module xpp.generic.factory;

import std;

export namespace xpp {
    namespace generic {

        namespace factory {

            template<typename ReturnType>
            class make_object {
              public:
                template<typename... Parameter>
                ReturnType operator()(xcb_connection_t *c, Parameter &&...parameter) const {
                    return ReturnType{c, std::forward<Parameter>(parameter)...};
                }
            };

            template<typename ReturnType>
            class make_object_with_member {
              public:
                template<typename Member, typename... Parameter>
                ReturnType operator()(xcb_connection_t *c, Member &&member, Parameter &&...parameter) const {
                    return ReturnType{c, std::forward<Member>(member), std::forward<Parameter>(parameter)...};
                }
            };

            template<typename ReturnType>
            class make_object_with_connection {
              public:
                template<typename... Parameter>
                ReturnType operator()(xcb_connection_t *c, Parameter &&...parameter) const {
                    return ReturnType{c, std::forward<Parameter>(parameter)...};
                }
            };

            template<typename ReturnType>
            class make_fundamental {
              public:
                template<typename Member, typename... Parameter>
                ReturnType operator()(xcb_connection_t *c, Member &&member) const {
                    return std::forward<Member>(member);
                }
            };

            template<typename MemberType,
                     typename ReturnType,
                     typename... Parameter>
            class make
                : public std::conditional<
                      std::is_constructible<ReturnType, MemberType>::value,
                      make_fundamental<ReturnType>,
                      typename std::conditional<
                          std::is_constructible<ReturnType,
                                                MemberType,
                                                Parameter...>::value,
                          make_object_with_member<ReturnType>,
                          make_object<ReturnType>>::type>::type {};

        }  // namespace factory

    }  // namespace generic
}  // namespace xpp
