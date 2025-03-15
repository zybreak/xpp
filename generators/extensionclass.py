from utils import get_namespace

class ExtensionClass(object):
    def __init__(self, namespace):
        self.namespace = namespace

    def make_class(self, header_writer, source_writer):
        ns = get_namespace(self.namespace)
        if self.namespace.is_ext:
            base = " : public xpp::generic::extension "
            ctor = "    extension(xcb_connection_t *c) : xpp::generic::extension(c, &xcb_%s_id) {}\n" % ns
        else:
            base = " "
            ctor = ""

        header_writer(
'''\
    //export class interface;

    //namespace event { export class dispatcher; }
    //namespace error { export class dispatcher; }

    export class extension%(base)s{
      public:
    %(ctor)s\
        //using interface = xpp::%(ns)s::interface<Derived>;
        using event_dispatcher = xpp::%(ns)s::event::dispatcher;
        using error_dispatcher = xpp::%(ns)s::error::dispatcher;
    };\
''' % {
    "base":base,
   "ctor":ctor,
   "ns":ns
})
