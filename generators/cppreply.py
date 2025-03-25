from utils import _ext, _n_item, get_namespace
from resource_classes import _resource_classes

_templates = {}

_templates['reply_class'] = \
'''\
    namespace reply {

        namespace detail {

            template<typename Check, typename CookieFunction>
            class %(name)s : public xpp::generic::reply<Check,
                                            xpp::generic::signature<decltype(%(c_name)s_reply), %(c_name)s_reply>,
                                            CookieFunction>
            {
              public:
                using base = xpp::generic::reply<Check, xpp::generic::signature<decltype(%(c_name)s_reply), %(c_name)s_reply>, CookieFunction>;

                template<typename ... Parameter>
                %(name)s(xcb_connection_t *c, Parameter && ... parameter) : base(c, std::forward<Parameter>(parameter)...) {}

            %(make_static_getter)s\
            %(accessors)s\
            }; // class %(name)s

        } // namespace detail

        namespace checked {
            using %(name)s = detail::%(name)s<
                xpp::generic::checked_tag,
                xpp::generic::signature<decltype(%(c_name)s), %(c_name)s>>;
        } // namespace checked

        namespace unchecked {
            using %(name)s = detail::%(name)s<
                xpp::generic::unchecked_tag,
                xpp::generic::signature<decltype(%(c_name)s_unchecked), %(c_name)s_unchecked>>;
        } // namespace unchecked

    } // namespace reply
'''

def _reply_class(name, c_name, ns, cookie, accessors, param):
    
    protos = param.wrapped_protos(param.has_defaults, param.has_defaults) if param.want_wrap else param.protos(param.has_defaults, param.has_defaults)
    calls = param.wrapped_calls(False) if param.want_wrap else param.calls(False)
    
    return _templates['reply_class'] % {
        "name": name,
        "c_name": c_name,
        "make_static_getter": cookie.make_static_getter(),
        "accessors": accessors,
        "protos": param.comma()+protos,
        "calls": param.comma()+calls
    }

_templates['reply_member_accessor'] = \
'''\
    template<typename ReturnType = %s, typename ... Parameter>
    ReturnType %s(Parameter && ... parameter) {
      using make = xpp::generic::factory::make<decltype(this->get()->%s),
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

    def make(self, header_writer, source_writer):
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
            self.cookie, "\n".join(accessors), self.parameter_list)
       
        header_writer(result)
        
        return result
