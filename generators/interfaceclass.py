# vim: set ts=4 sws=4 sw=4:

from utils import get_namespace

from cppevent import event_dispatcher_class
from cpperror import error_dispatcher_class

_templates = {}

_templates['interface_class'] = \
"""\
    export class interface%(base)s {
      public:
%(methods)s\
%(ctor)s
    }; // class interface
"""

_ignore_events = \
        { "XCB_PRESENT_GENERIC" }

########## INTERFACECLASS ##########

class InterfaceClass(object):
    def __init__(self):
        self.requests = []
        self.events = []
        self.errors = []

    def add(self, request):
        self.requests.append(request)

    def add_event(self, event):
        if event.opcode_name not in _ignore_events:
            self.events.append(event)

    def add_error(self, error):
        self.errors.append(error)

    def set_namespace(self, namespace):
        self.namespace = namespace

    def make_proto(self, header_writer, source_writer):
        ns = get_namespace(self.namespace)
        methods = ""
        for request in self.requests:
            methods += request.make_object_class_inline(True, source_writer) + "\n"

        typedef = []
        ctor = ""
        base = ""
        if self.namespace.is_ext:
            #typedef = [ "using extension = xpp::%s::extension;" % ns ]
            base = " : public xpp::generic::extension"
            ctor = "\
        explicit interface(xcb_connection_t *c) : xpp::generic::extension(c, &xcb_%(ext_name)s_id) {}" % { "ext_name": ns }
        else:
            ctor = """\
      protected:  
        explicit interface() = default;
        virtual xcb_connection_t* get_connection() const = 0;
            """

        #if len(typedef) > 0:
        #    typedef = "".join(["    " + s for s in typedef]) + "\n"
        #else:
        #    typedef = ""


        header_writer((_templates['interface_class'] % {
            "typedef": typedef,
            "methods": methods,
            "ext_name": ns,
            "base": base,
            "ctor": ctor
        }))
        
        # + \
        #'\n' + event_dispatcher_class(self.namespace, self.events) + \
        #'\n' + error_dispatcher_class(self.namespace, self.errors)

########## INTERFACECLASS ##########
