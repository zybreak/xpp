module;
#include <xcb/xcb.h>
export module xpp.generic.resource;

import std;

#if 0
namespace xpp::generic::detail {

    template<typename ResourceId>
    class interfaces : public Interfaces<interfaces<Derived, Resource, ResourceId, Interfaces...>>... {
      public:
        ResourceId const & resource(void) const {
            return *static_cast<Derived const &>(*this);
        }

        //xpp::connection& connection(void) const {
        //    return static_cast<Resource const &>(*this).connection();
        //}
    };  // class interfaces

};  // namespace xpp::generic::detail
#endif

export namespace xpp {
    
    namespace generic {

        template<typename ResourceId>
        class resource {
          protected:
            xcb_connection_t *m_c;
            // reference counting for Resource object
            std::shared_ptr<ResourceId> m_resource;

          public:
            using Create = std::function<void(xcb_connection_t*, ResourceId const &)>;
            using Destroy = std::function<void(xcb_connection_t*, ResourceId const &)>;
            resource(xcb_connection_t *c, Create create, Destroy destroy) : m_c(c) {
                auto xid = xcb_generate_id(c);

                // class create before instatiating the shared_ptr
                // create might fail and throw an error, hence shared_ptr would hold an
                // invalid xid, causing possibly another exception in destroy()
                // when create() throws, then the shared_ptr will not be created
                create(c, static_cast<ResourceId>(xid));

                m_resource =
                    std::shared_ptr<ResourceId>(new ResourceId(xid),
                                                [&](ResourceId *r) {
                    destroy(m_c, *r);
                    delete r;
                });
            }
            
            resource(xcb_connection_t *c, ResourceId const &resource_id)
                : m_c(c), m_resource(std::make_shared<ResourceId>(resource_id)) {
            }

            resource(resource<ResourceId> const &other)
                : m_c(other.m_c), m_resource(other.m_resource) {
            }

            virtual void operator=(resource<ResourceId> const &other) {
                m_c = other.m_c;
                m_resource = other.m_resource;
            }

            virtual void operator=(ResourceId const &resource) {
                m_resource = std::make_shared<ResourceId>(resource);
            }

            virtual ResourceId const & operator*() const {
                return *m_resource;
            }

            virtual operator ResourceId const &() const {
                return *m_resource;
            }

            xcb_connection_t* get_connection() const {
                return m_c;
            }
            
            ResourceId const & get_resource() const {
                return *m_resource;
            }
        };  // class resource

        template<typename ResourceId>
        std::ostream & operator<<(std::ostream &os, resource<ResourceId> const &resource) {
            return os << std::hex << "0x" << *resource << std::dec;
        }

    }  // namespace generic

}  // namespace xpp
