from utils import _n, _ext, _n_item, get_namespace
from resource_classes import _resource_classes

_templates = {}

_templates['reply_class'] = \
'''\
namespace reply {

namespace detail {

template<typename Connection,
         typename Check,
         typename CookieFunction>
class %(name)s
  : public xpp::generic::reply<%(name)s<Connection, Check, CookieFunction>,
                               Connection,
                               Check,
                               xpp::generic::signature<decltype(%(c_name)s_reply), %(c_name)s_reply>,
                               CookieFunction>
{
  public:
    typedef xpp::generic::reply<%(name)s<Connection, Check, CookieFunction>,
                                Connection,
                                Check,
                                xpp::generic::signature<decltype(%(c_name)s_reply), %(c_name)s_reply>,
                                CookieFunction>
                                  base;

    template<typename C, typename ... Parameter>
    %(name)s(C && c, Parameter && ... parameter)
      : base(std::forward<C>(c), std::forward<Parameter>(parameter) ...)
    {}

%(make_static_getter)s\
%(accessors)s\
}; // class %(name)s

} // namespace detail

namespace checked {
template<typename Connection>
using %(name)s = detail::%(name)s<
    Connection, xpp::generic::checked_tag,
    xpp::generic::signature<decltype(%(c_name)s), %(c_name)s>>;
} // namespace checked

namespace unchecked {
template<typename Connection>
using %(name)s = detail::%(name)s<
    Connection, xpp::generic::unchecked_tag,
    xpp::generic::signature<decltype(%(c_name)s_unchecked), %(c_name)s_unchecked>>;
} // namespace unchecked

} // namespace reply
'''

def _reply_class(name, c_name, ns, cookie, accessors):
    return _templates['reply_class'] % {
        "name": name,
        "c_name": c_name,
        "make_static_getter": cookie.make_static_getter(),
        "accessors": accessors
    }

_templates['reply_member_accessor'] = \
'''\
    template<typename ReturnType = %s, typename ... Parameter>
    ReturnType
    %s(Parameter && ... parameter)
    {
      using make = xpp::generic::factory::make<Connection,
                                               decltype(this->get()->%s),
                                               ReturnType,
                                               Parameter ...>;
      return make()(this->m_c,
                    this->get()->%s,
                    std::forward<Parameter>(parameter) ...);
    }
'''

def _reply_member_accessor(request_name, name, c_type, template_type):
    return _templates['reply_member_accessor'] % \
            ( c_type
            , name
            , name
            , name
            )

class CppReply(object):
    def __init__(self, namespace, name, cookie, reply, accessors, parameter_list):
        self.namespace = namespace
        self.name = name
        self.reply = reply
        self.cookie = cookie
        self.accessors = accessors
        self.parameter_list = parameter_list
        self.request_name = _ext(_n_item(self.name[-1]))
        self.c_name = "xcb" \
            + (("_" + get_namespace(namespace)) if namespace.is_ext else "") \
            + "_" + self.request_name

    def make_accessors(self):
        return "\n".join(["\n%s\n" % a for a in self.accessors])

    def make(self):
        accessors = [self.make_accessors()]
        naccessors = len(self.accessors)

        for field in self.reply.fields:
            if (field.field_type[-1] in _resource_classes
                and not field.type.is_list
                and not field.type.is_container):

                naccessors = naccessors + 1

                name = field.field_name.lower()
                c_type = field.c_field_type
                template_type = field.field_name.capitalize()

                accessors.append(_reply_member_accessor(self.request_name, name, c_type, template_type))

        result = ""
        result += _reply_class(
            self.request_name, self.c_name, get_namespace(self.namespace),
            self.cookie, "\n".join(accessors))
        return result
