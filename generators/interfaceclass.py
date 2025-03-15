# vim: set ts=4 sws=4 sw=4:

from utils import get_namespace

from cppevent import event_dispatcher_class
from cpperror import error_dispatcher_class

_templates = {}

_templates['interface_class'] = \
"""\
    export class interface {
      protected:
        virtual xcb_connection_t* get_connection() const = 0;

      public:
    %s\

        virtual ~interface(void) {}

    #if 0
        const interface<Derived> & %s(void) {
          return *this;
        }
    #endif

    %s\
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
        #if self.namespace.is_ext:
            #typedef = [ "using extension = xpp::%s::extension;" % ns ]

        if len(typedef) > 0:
            typedef = "".join(["    " + s for s in typedef]) + "\n"
        else:
            typedef = ""


        header_writer((_templates['interface_class']
                % (typedef, ns, methods)))
        
        # + \
        #'\n' + event_dispatcher_class(self.namespace, self.events) + \
        #'\n' + error_dispatcher_class(self.namespace, self.errors)

########## INTERFACECLASS ##########
