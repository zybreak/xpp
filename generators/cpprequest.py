# vim: set ts=4 sws=4 sw=4:

from utils import _ext, _n_item, get_namespace, _reserved_keywords
from parameter import ParameterList
from cppreply import CppReply
from cppcookie import CppCookie
from sys import stderr

_templates = {}

_templates['void_request_function_impl'] = \
'''\
    void %(name)s_checked(xcb_connection_t *c%(comma)s%(protos)s) {
      xpp::generic::check/*<xpp::%(ns)s::error::dispatcher>*/(
          c,
          %(c_name)s_checked(c%(comma)s%(calls)s));
    }

    void %(name)s(xcb_connection_t *c%(comma)s%(protos)s) {
      %(c_name)s(c%(comma)s%(calls)s);
    }
'''

def _void_request_function_impl(ns, name, c_name, param):
    return _templates['void_request_function_impl'] % {
        "c_name": c_name,
        "name": name,
        "ns": ns,
        "comma": param.comma(),
        "protos": param.protos(param.has_defaults, param.has_defaults),
        "calls": param.calls(False)
    }


_templates['void_request_function'] = \
'''\
    export void %(name)s_checked(xcb_connection_t *c%(protos)s);
    export void %(name)s(xcb_connection_t *c%(protos)s);
'''

def _void_request_function(ns, name, c_name, param):
    return _templates['void_request_function'] % {
        "name": name,
        "protos": param.comma()+param.protos(param.has_defaults, param.has_defaults),
        "calls": param.comma()+param.calls(False)
    }

_templates['reply_request_function'] = \
'''\
    export reply::checked::%(name)s %(name)s(xcb_connection_t *c%(protos)s);
    export reply::unchecked::%(name)s %(name)s_unchecked(xcb_connection_t *c%(protos)s);
'''

def _reply_request_function(name, param):
    return _templates['reply_request_function'] % {
        "name": name,
        "protos": param.comma()+param.protos(param.has_defaults, param.has_defaults),
        "calls": param.comma()+param.calls(False)
    }

_templates['reply_request_function_impl'] = \
'''\
    reply::checked::%(name)s %(name)s(xcb_connection_t *c%(protos)s) {
      return reply::checked::%(name)s(c%(calls)s);
    }
    
    reply::unchecked::%(name)s %(name)s_unchecked(xcb_connection_t *c%(protos)s) {
      return reply::unchecked::%(name)s(c%(calls)s);
    }
'''

def _reply_request_function_impl(name, param):
    return _templates['reply_request_function_impl'] % {
        "name": name,
        "protos": param.comma()+param.protos(param.has_defaults, False),
        "calls": param.comma()+param.calls(False)
    }

_templates['inline_reply_class_impl'] = \
'''\
    reply::checked::%(request_name)s %(class_name)s::%(method_name)s(%(protos)s) const {
      return xpp::%(ns)s::%(request_name)s(get_connection()%(member)s%(calls)s);
    }

    reply::unchecked::%(request_name)s %(class_name)s::%(method_name)s_unchecked(%(protos)s) const {
      return xpp::%(ns)s::%(request_name)s_unchecked(get_connection()%(member)s%(calls)s);
    }
'''

def _inline_reply_class_impl(class_name, request_name, method_name, member, ns, protos, calls):
    return _templates['inline_reply_class_impl'] % {
        "class_name": class_name,
        "member": member,
        "method_name": method_name,
        "ns": ns,
        "request_name": request_name,
        "protos": protos,
        "calls": calls 
    }


_templates['inline_reply_class'] = \
'''\
    reply::checked::%(request_name)s %(method_name)s(%(protos)s) const;
    reply::unchecked::%(request_name)s %(method_name)s_unchecked(%(protos)s) const;
'''

def _inline_reply_class(request_name, method_name, member, ns, param):
    return _templates['inline_reply_class'] % {
            "member": member,
            "method_name": method_name,
            "ns": ns,
            "request_name": request_name,
            "protos": param.protos(param.has_defaults, param.has_defaults),
            "calls": param.comma()+param.calls(False)
        }

_templates['inline_void_class'] = \
'''\
    void %(method_name)s_checked(%(protos)s) const;
    void %(method_name)s(%(protos)s) const;
'''

def _inline_void_class(request_name, method_name, member, ns, param):
    return _templates['inline_void_class'] % {
        "method_name": method_name,
        "ns": ns,
        "request_name": request_name,
        "member": member,
        "protos": param.protos(param.has_defaults, param.has_defaults),
        "calls": param.comma()+param.calls(False)
    }

_templates['inline_void_class_impl'] = \
    '''\
    void %(class_name)s::%(method_name)s_checked(%(protos)s) const {
      xpp::%(ns)s::%(request_name)s_checked(get_connection()%(member)s%(calls)s);
    }

    void %(class_name)s::%(method_name)s(%(protos)s) const {
      xpp::%(ns)s::%(request_name)s(get_connection()%(member)s%(calls)s);
    }
    '''

def _inline_void_class_impl(class_name, request_name, method_name, member, ns, protos, calls):
    return _templates['inline_void_class_impl'] % {
        "class_name": class_name,
        "method_name": method_name,
        "ns": ns,
        "request_name": request_name,
        "member": member,
        "protos": protos,
        "calls": calls
    }

_replace_special_classes = \
        { "gcontext" : "gc" }

def replace_class(method, class_name):
    cn = _replace_special_classes.get(class_name, class_name)
    method_name = method.replace("_" + cn, "")
    if method_name in _reserved_keywords:
        stderr.write('Reserved word "%s" used for method in class "%s", replacing with "%s"\n' % (method_name, class_name, method))
        return method
    else:
        return method_name

class CppRequest(object):
    def __init__(self, request, name, is_void, namespace, reply):
        self.request = request
        self.name = name
        self.request_name = _ext(_n_item(self.request.name[-1]))
        self.is_void = is_void
        self.namespace = namespace
        self.reply = reply
        self.c_namespace = \
            "" if namespace.header.lower() == "xproto" \
            else get_namespace(namespace)
        self.accessors = []
        self.parameter_list = ParameterList()

        self.c_name = "xcb" \
            + (("_" + get_namespace(namespace)) if namespace.is_ext else "") \
            + "_" + self.request_name

    def add(self, param):
        self.parameter_list.add(param)

    def make_wrapped(self):
        self.parameter_list.make_wrapped()

    def make_class(self, header_writer, source_writer):
        cppcookie = CppCookie(self.namespace, self.is_void, self.request.name, self.reply, self.parameter_list)

        if self.is_void:
            void_functions = cppcookie.make_void_functions(header_writer, source_writer)
            if len(void_functions) == 0:
                header_writer(_void_request_function(get_namespace(self.namespace), self.request_name, self.c_name, self.parameter_list))
                source_writer(_void_request_function_impl(get_namespace(self.namespace), self.request_name, self.c_name, self.parameter_list))

        else:
            cppreply = CppReply(self.namespace, self.request.name, cppcookie, self.reply, self.accessors, self.parameter_list)
            cppreply.make(header_writer, source_writer)
            header_writer("\n")
            header_writer(_reply_request_function(self.request_name, self.parameter_list))
            source_writer(_reply_request_function_impl(self.request_name, self.parameter_list))

    def make_object_class_inline(self, is_connection, source_writer, class_name=""):
        member = ""
        method_name = self.name
        if not is_connection:
            member = ",get_resource()"
            method_name = replace_class(method_name, class_name)
            
        if class_name == "":
            class_name = "interface"

        if self.is_void:
            source_writer(_inline_void_class_impl(class_name, self.request_name, method_name, member, get_namespace(self.namespace), self.parameter_list.protos(self.parameter_list.has_defaults, False), self.parameter_list.comma()+self.parameter_list.calls(False)))
            return _inline_void_class(self.request_name, method_name, member, get_namespace(self.namespace), self.parameter_list)
        else:
            source_writer(_inline_reply_class_impl(class_name, self.request_name, method_name, member, get_namespace(self.namespace), self.parameter_list.protos(self.parameter_list.has_defaults, False), self.parameter_list.comma()+self.parameter_list.calls(False)))
            return _inline_reply_class(self.request_name, method_name, member, get_namespace(self.namespace), self.parameter_list)
