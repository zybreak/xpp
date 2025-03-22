# vim: set ts=4 sws=4 sw=4:

from sys import stderr
from copy import deepcopy

from utils import get_namespace

class ObjectClass(object):
    def __init__(self, name):
        self.name = name
        self.requests = []

    def add(self, request):
        if (len(request.parameter_list.parameter) > 0
                and request.parameter_list.parameter[0].c_type == self.c_name):
            request_copy = deepcopy(request)
            request_copy.parameter_list.parameter.pop(0)
            request_copy.make_wrapped()
            self.requests.append(request_copy)

    def set_namespace(self, namespace):
        self.namespace = namespace
        name = (get_namespace(namespace) + "_") if namespace.is_ext else ""
        self.c_name = "xcb_%s_t" % (name + self.name.lower())

    def make_inline(self, header_writer, source_writer):
        ns = get_namespace(self.namespace)
        name = self.name.lower()
        c_name = self.c_name
        methods = ""
        
        if name == "event":
            #stderr.write('renaming ObjectClass %s to %s in namespace %s for %s\n' % (name, 'Event', ns, c_name))
            #name = "Event"
            return "// ignored ObjectClass %s" % name

        for request in self.requests:
            methods += request.make_object_class_inline(False, source_writer, name) + "\n"

        if methods != "":
            header_writer(
"""\
    export class %(name)s : public xpp::generic::resource<%(c_name)s> {
      public:
        using res = %(c_name)s;
        using base = xpp::generic::resource<%(c_name)s>;
        
        virtual ~%(name)s() = default;
        
        %(name)s(xcb_connection_t *c, base::Create create, base::Destroy destroy) : xpp::generic::resource<%(c_name)s>(c, create, destroy) {}
        
        %(name)s(xcb_connection_t *c, %(c_name)s const &resource_id) : xpp::generic::resource<%(c_name)s>(c, resource_id) {}

%(methods)s
    }; // class %(name)s
""" % {
    "name": name,
    "c_name": c_name,
    "methods": methods
})
