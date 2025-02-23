module;
#include <xcb/xcb.h>
export module xpp.event;

import std;
import xpp.proto.x;

export {
    namespace xpp {

        namespace event {

            namespace detail {

                class dispatcher {
                  public:
                    virtual ~dispatcher(void) {
                    }
                    template <typename Event>
                    void dispatch(Event const &e);
                };  // class dispatcher

                template <typename Event>
                class sink : virtual public dispatcher {
                  public:
                    virtual ~sink(void) {
                    }
                    virtual void handle(Event const &) = 0;
                };

            }  // namespace detail

            template <typename Event, typename... Events>
            class sink
                : public detail::sink<Event>,
                  public detail::sink<Events>... {};

            template <typename Connection, typename... Extensions>
            class registry
                : public xpp::x::event::dispatcher<Connection>,
                  public Extensions::template event_dispatcher<Connection>... {
              public:
                typedef unsigned int priority;

                template <typename C>
                explicit registry(C &&c)
                    : xpp::x::event::dispatcher<Connection>(std::forward<C>(c)), Extensions::template event_dispatcher<Connection>(std::forward<C>(c), c.template extension<Extensions>())..., m_c(std::forward<C>(c)) {
                }

                bool
                dispatch(std::shared_ptr<xcb_generic_event_t> const &event) const {
                    return dispatch<xpp::x::extension, Extensions...>(event);
                }

                template <typename Event, typename... Rest>
                void
                attach(priority p, sink<Event, Rest...> *s) {
                    attach<sink<Event, Rest...>, Event, Rest...>(p, s);
                }

                template <typename Event, typename... Rest>
                void
                detach(priority p, sink<Event, Rest...> *s) {
                    detach<sink<Event, Rest...>, Event, Rest...>(p, s);
                }

              private:
                typedef std::multimap<priority, detail::dispatcher *> priority_map;

                Connection m_c;
                std::unordered_map<uint8_t, priority_map> m_dispatchers;

                template <typename Event>
                uint8_t opcode(xpp::x::extension const &) const {
                    return Event::opcode();
                }

                template <typename Event, typename Extension>
                uint8_t opcode(Extension const &extension) const {
                    return Event::opcode(extension);
                }

                template <typename Event>
                uint8_t opcode(void) const {
                    return opcode<Event>(m_c.template extension<typename Event::extension>());
                }

                template <typename Event>
                void
                handle(Event const &event) const {
                    auto it = m_dispatchers.find(opcode<Event>());
                    if (it != m_dispatchers.end()) {
                        for (auto &item : it->second) {
                            item.second->dispatch(event);
                        }
                    }
                }

                struct handler {
                    handler(registry<Connection, Extensions...> const &registry)
                        : m_registry(registry) {
                    }

                    registry<Connection, Extensions...> const &m_registry;

                    template <typename Event>
                    void
                    operator()(Event const &event) const {
                        m_registry.handle(event);
                    }
                };

                template <typename Extension>
                bool
                dispatch(std::shared_ptr<xcb_generic_event_t> const &event) const {
                    typedef typename Extension::template event_dispatcher<Connection> const &dispatcher;
                    return static_cast<dispatcher>(*this)(handler(*this), event);
                }

                template <typename Extension, typename Next, typename... Rest>
                bool
                dispatch(std::shared_ptr<xcb_generic_event_t> const &event) const {
                    dispatch<Extension>(event);
                    return dispatch<Next, Rest...>(event);
                }

                template <typename Sink, typename Event>
                void
                attach(priority p, Sink *s) {
                    attach(p, s, opcode<Event>());
                }

                template <typename Sink, typename Event, typename Next, typename... Rest>
                void
                attach(priority p, Sink *s) {
                    attach(p, s, opcode<Event>());
                    attach<Sink, Next, Rest...>(p, s);
                }

                void attach(priority p, detail::dispatcher *d, uint8_t opcode) {
                    m_dispatchers[opcode].emplace(p, d);
                }

                template <typename Sink, typename Event>
                void
                detach(priority p, Sink *s) {
                    detach(p, s, opcode<Event>());
                }

                template <typename Sink, typename Event, typename Next, typename... Rest>
                void
                detach(priority p, Sink *s) {
                    detach(p, s, opcode<Event>());
                    detach<Sink, Next, Rest...>(p, s);
                }

                void
                detach(priority p, detail::dispatcher *d, uint8_t opcode) {
                    auto it = m_dispatchers.find(opcode);

                    if (it == m_dispatchers.end()) {
                        return;
                    }

                    auto &prio_map = it->second;
                    auto const &prio_sink_pair = prio_map.equal_range(p);
                    for (auto it = prio_sink_pair.first; it != prio_sink_pair.second;) {
                        if (d == it->second) {
                            it = prio_map.erase(it);
                        } else {
                            ++it;
                        }
                    }
                }

            };  // xpp::event::source

        }  // namespace event

    }  // namespace xpp
}

template <typename Event>
void xpp::event::detail::dispatcher::dispatch(Event const &e) {
    auto event_sink = dynamic_cast<xpp::event::detail::sink<Event> *>(this);
    if (event_sink != nullptr) {
        event_sink->handle(e);
    }
}
