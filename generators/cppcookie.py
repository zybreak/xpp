from utils import _ext, _n_item, get_namespace

_templates = {}

_templates['void_cookie_function_impl'] = \
'''\
    %(export)s
    %(template)s\
    void %(name)s_checked(xcb_connection_t *c%(protos)s)
    {%(initializer)s\
      xpp::generic::check/*<xpp::%(ns)s::error::dispatcher>*/(
          c,
          %(c_name)s_checked(c%(calls)s));
    }

    %(export)s
    %(template)s\
    void %(name)s(xcb_connection_t *c%(protos)s)
    {%(initializer)s\
      %(c_name)s(c%(calls)s);
    }
'''

def _void_cookie_function_impl(ns, name, c_name, template, return_value, protos, calls, initializer, export = ""):
    if len(template) == 0: template = ""
    return _templates['void_cookie_function_impl'] % {
        "template": template,
        "calls": calls,
        "c_name": c_name,
        "initializer": initializer,
        "name": name,
        "ns": ns,
        "protos": protos,
        "export": export
    }

_templates['void_cookie_function'] = \
'''\
    export
    %(template)s\
    void %(name)s_checked(xcb_connection_t *c%(protos)s);

    export
    %(template)s\
    void %(name)s(xcb_connection_t *c%(protos)s);
'''

def _void_cookie_function(ns, name, c_name, template, return_value, protos, calls, initializer):
    if len(template) == 0: template = ""
    return _templates['void_cookie_function'] % {
        "template": template,
        "name": name,
        "protos": protos
    }

_templates['cookie_static_getter'] = \
'''\
%s\
    static %s cookie(xcb_connection_t * const c%s) {%s\
      return base::cookie(c%s);
    }
'''

def _cookie_static_getter(template, return_value, protos, calls, initializer):
    return _templates['cookie_static_getter'] % \
            ( template
            , return_value
            , protos
            , initializer
            , calls
            )

class CppCookie(object):
    def __init__(self, namespace, is_void, name, reply, parameter_list):
        self.namespace = namespace
        self.is_void = is_void
        self.name = name
        self.reply = reply
        self.parameter_list = parameter_list
        self.request_name = _ext(_n_item(self.name[-1]))
        self.c_name = "xcb" \
            + (("_" + get_namespace(namespace)) if namespace.is_ext else "") \
            + "_" + self.request_name

    def comma(self):
        return self.parameter_list.comma()

    def calls(self, sort):
        return self.parameter_list.calls(sort)

    def protos(self, sort, defaults):
        return self.parameter_list.protos(sort, defaults)

    def iterator_template(self, indent="    ", tail="\n"):
        prefix = "template<typename "
        return indent + prefix \
                + ", typename ".join(self.parameter_list.iterator_templates \
                                   + self.parameter_list.templates) \
                + ">" + tail \
                if len(self.parameter_list.iterator_templates) > 0 \
                else ""

    def iterator_calls(self, sort):
        return self.parameter_list.iterator_calls(sort)

    def iterator_protos(self, sort, defaults):
        return self.parameter_list.iterator_protos(sort, defaults)

    def iterator_initializers(self):
        return self.parameter_list.iterator_initializers()
    
    def void_function_impls(self, protos, calls, template="", initializer=[], export = ""):
        inits = "" if len(initializer) > 0 else "\n"
        for i in initializer:
            inits += "\n"
            for line in i.split('\n'):
                inits += "      " + line + "\n"

        return_value = "xcb_void_cookie_t"

        return _void_cookie_function_impl(get_namespace(self.namespace),
                                     self.request_name,
                                     self.c_name,
                                     template,
                                     return_value,
                                     self.comma() + protos,
                                     self.comma() + calls,
                                     inits,
                                      export)

    def void_functions(self, protos, calls, template="", initializer=[]):
        inits = "" if len(initializer) > 0 else "\n"
        for i in initializer:
            inits += "\n"
            for line in i.split('\n'):
                inits += "      " + line + "\n"

        return_value = "xcb_void_cookie_t"

        return _void_cookie_function(get_namespace(self.namespace),
                                     self.request_name,
                                     self.c_name,
                                     template,
                                     return_value,
                                     self.comma() + protos,
                                     self.comma() + calls,
                                     inits)


    def static_reply_methods(self, protos, calls, template="", initializer=[]):
        inits = "" if len(initializer) > 0 else "\n"
        for i in initializer:
            inits += "\n"
            for line in i.split('\n'):
                inits += "      " + line + "\n"

        if self.is_void: return_value = "xcb_void_cookie_t"
        else: return_value = self.c_name + "_cookie_t"

        return _cookie_static_getter(template,
                                     return_value,
                                     self.comma() + protos,
                                     self.comma() + calls,
                                     inits)


    def make_static_getter(self):
        default = ""

        if self.parameter_list.has_defaults:
            default = self.static_reply_methods(self.protos(True, True), self.calls(False))
        else:
            default = self.static_reply_methods(self.protos(False, False), self.calls(False))

        wrapped = ""
        if self.parameter_list.want_wrap:
            wrapped = \
                self.static_reply_methods(self.iterator_protos(True, True),
                        self.iterator_calls(False), self.iterator_template(),
                        self.iterator_initializers())

        default_args = ""
        if self.parameter_list.is_reordered():
            default_args = \
                self.static_reply_methods(self.protos(True, True), self.calls(False))

        result = ""

        if (self.parameter_list.has_defaults
            or self.parameter_list.is_reordered()
            or self.parameter_list.want_wrap):
            result += default

        if self.parameter_list.is_reordered():
            result += "\n" + default_args

        if self.parameter_list.want_wrap:
            result += "\n" + wrapped

        return result

    def make_void_functions(self, header_writer, source_writer):
        
        result = ""

        if self.parameter_list.has_defaults or self.parameter_list.is_reordered() or self.parameter_list.want_wrap:

            result += "// has_defaults\n"
            if self.parameter_list.has_defaults:
                result += self.void_functions(self.protos(True, True), self.calls(False))
            else:
                result += self.void_functions(self.protos(False, False), self.calls(False))
                
            source_writer(self.void_function_impls(self.protos(self.parameter_list.has_defaults, False), self.calls(False)))
        
        if self.parameter_list.is_reordered():
            result += "\n// is_reordered\n"
            result += self.void_functions(self.protos(True, True), self.calls(False))
            source_writer(self.void_function_impls(self.protos(True, False), self.calls(False)))
        
        if self.parameter_list.want_wrap:
            result += "\n// want_wrap\n"
            result += self.void_function_impls(self.iterator_protos(True, True),
                                self.iterator_calls(False),
                                self.iterator_template(indent=""),
                                self.iterator_initializers(),
                                export="export")
            
        header_writer(result)
        
        return result
