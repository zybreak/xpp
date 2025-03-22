#!/usr/bin/env python
# vim: set ts=4 sw=4:

"""
This script converts XCB XML files into cpp code
"""

# Must create an "output" dictionary before any xcbgen imports.
output = {
    'open': lambda module: client.c_open(module),
    'close': lambda module: client.c_close(module),
    'simple': lambda xtype, name: client.cpp_simple(xtype, name),
    'enum': lambda xtype, name: None,
    'struct': lambda xtype, name: None,
    'union': lambda xtype, name: None,
    'request': lambda xtype, name: client.cpp_request(xtype, name),
    'event': lambda xtype, name: client.cpp_event(xtype, name),
    'error': lambda xtype, name: client.cpp_error(xtype, name),
    'eventstruct': lambda xtype, name: None,
}

import getopt
from utils import get_namespace, _n_item, _ext, _n, _t
from cppevent import CppEvent
from cpperror import CppError
from parameter import Parameter
from cpprequest import CppRequest
from objectclass import ObjectClass
from interfaceclass import InterfaceClass
from extensionclass import ExtensionClass
from resource_classes import _resource_classes
from type_setup import c_type_setup, c_accessors
from xcbgen.state import Module
from xcbgen.xtypes import *

class Client(object):

    _cpp_request_names = []
    _cpp_request_objects = {}

    # see c_open()
    _interface_class = InterfaceClass()

    _cpp_events = []
    _cpp_errors = []

    _object_classes = {}

    _hlines: list[str] = []
    _clines: list[str] = []
    _hlevel = 0
    _clevel = 0
    _ns = None

    header_file = None
    source_file = None
    input_file = None

    def __init__(self):
        for i, v in enumerate(_resource_classes):
            self._object_classes[i] = ObjectClass(v)


    def _cpp_request_helper(self, module, name, is_void):
        """
        Declares a request function.
        """

        request_name = _ext(_n_item(module.name[-1]))

        param_fields = []

        for field in module.fields:
            if field.visible:
                param_fields.append(field)

        self._cpp_request_names.append(request_name)
        self._cpp_request_objects[request_name] = CppRequest(module, request_name, is_void, self._ns, module.reply)

        for field in param_fields:
            param = Parameter(field)

            self._cpp_request_objects[request_name].add(param)

        self._cpp_request_objects[request_name].make_wrapped()

        self._interface_class.add(self._cpp_request_objects[request_name])

        for key in self._object_classes:
            self._object_classes[key].set_namespace(self._ns)
            self._object_classes[key].add(self._cpp_request_objects[request_name])


    def c_open(self, module):
        """
        Exported function that handles module open.
        Opens the files and writes out the auto-generated comment,
        header file includes, etc.
        """
        self._ns = module.namespace
        self._ns.c_ext_global_name = _n(self._ns.prefix + ('id',), self._ns)

        self._interface_class.set_namespace(self._ns)

        self._h_setlevel(0)
        self._c_setlevel(0)
        
        self._c('module;')
        if self._ns.header.lower() == 'xkb':
            self._c('#define explicit _explicit')
        self._c('#include <xcb/' + _get_xcb_include(self, self._ns.header.lower()) + '>')
        if self._ns.header.lower() == 'xkb':
            self._c('#undef explicit')
        self._c('module xpp.proto.%s;', get_namespace(self._ns).lower())
        self._c('')
        self._c('import std;')
        self._c('import xpp.generic;')
        self._c('import xpp.connection;')
        self._c('')
        self._c('namespace xpp::%s {' % get_namespace(self._ns))

        self._h('module;')

        if self._ns.header.lower() == 'xkb':
            self._h('#define explicit _explicit')
        self._h('#include <xcb/' + _get_xcb_include(self, self._ns.header.lower()) + '>')
        if self._ns.header.lower() == 'xkb':
            self._h('#undef explicit')
        self._h('')
        self._h('export module xpp.proto.%s;', get_namespace(self._ns).lower())
        self._h('')
        self._h('import std;')
        self._h('')

        self._h('import xpp.generic;')

        self._h('')
        self._h('namespace xpp::%s {' % get_namespace(self._ns))


    def c_close(self, module):
        """
        Exported function that handles module close.
        Writes out all the stored content lines, then closes the files.
        """

        #self._h('// extension')
        #ExtensionClass(self._ns).make_class(self._h, self._c)

        #self._h('// events')
        #for cpp_event in self._cpp_events:
        #    cpp_event.make_class(self._h, self._c)

        #self._h('// errors')
        #for cpp_error in self._cpp_errors:
        #    cpp_error.make_class(self._h, self._c)

        self._h('// ################ REQUESTS  ################')
        for name in self._cpp_request_names:
            self._cpp_request_objects[name].make_class(self._h, self._c)

        self._h('// ########### OBJECT CLASSES ################# ')
        for key in self._object_classes:
            self._object_classes[key].make_inline(self._h, self._c)

        self._h('// ################ INTERFACES ##################')
        self._interface_class.make_proto(self._h, self._c)

        self._h('')
        self._h("} // namespace xpp::%s" % get_namespace(self._ns))

        self._h('')


        self._c('')
        self._c("} // namespace xpp::%s" % get_namespace(self._ns))

        # Write header file
        def write_file(file, lines):
            for list in lines:
                for line in list:
                    file.write(line)
                    file.write('\n')

        if (self.header_file == None):
            write_file(sys.stdout, self._hlines)
        else:
            with open(self.header_file, "w", encoding="utf-8") as file:
                write_file(file, self._hlines)

        if (self.source_file == None):
            write_file(sys.stdout, self._clines)
        else:
            with open(self.source_file, "w", encoding="utf-8") as file:
                write_file(file, self._clines)


    def cpp_simple(self, xtype, name):
        """
        Exported function that handles cardinal type declarations.
        These are types which are typedef'd to one of the CARDx's, char, float, etc.
        """
        c_type_setup(self._ns, xtype, name, ())

        if xtype.name != name:
            # Typedef
            self._h_setlevel(0)


    def cpp_request(self, xtype, name):
        """
        Exported function that handles request declarations.
        """

        c_type_setup(self._ns, xtype, name, ('request',))

        if xtype.reply:

            c_type_setup(self._ns, xtype.reply, name, ('reply',))

            # Request prototypes
            self._cpp_request_helper(xtype, name, False)

            # Reply accessors
            request_name = _ext(_n_item(xtype.reply.name[-1]))

            if request_name in client._cpp_request_objects:
                client._cpp_request_objects[request_name].accessors.extend(c_accessors(self._ns, xtype.reply, name + ('reply',), name))

        else:
            # Request prototypes
            self._cpp_request_helper(xtype, name, True)

    def cpp_event(self, xtype, name):
        """
        Exported function that handles event declarations.
        """

        c_type_setup(self._ns, xtype, name, ('event',))

        opcode = _n(name, self._ns).upper()
        c_name = _t(xtype.name + ('event',), self._ns)

        cpp_event = CppEvent(xtype.opcodes[name], opcode, c_name, self._ns, name, xtype.fields)
        self._cpp_events.append(cpp_event)
        self._interface_class.add_event(cpp_event)

    def cpp_error(self, xtype, name):
        """
        Exported function that handles error declarations.
        """
        c_type_setup(self._ns, xtype, name, ('error',))

        opcode_name = _n(name, self._ns).upper()
        c_name = _t(xtype.name + ('error',), self._ns)
        cpp_error = CppError(xtype, self._ns, name, c_name, xtype.opcodes[name], opcode_name)
        self._cpp_errors.append(cpp_error)
        self._interface_class.add_error(cpp_error)

    def _h(self, fmt, *args):
        """
        Writes the given line to the header file.
        """
        self._hlines[self._hlevel].append(fmt % args)


    def _c(self, fmt, *args):
        """
        Writes the given line to the source file.
        """
        self._clines[self._clevel].append(fmt % args)
        
        
    # XXX See if this level thing is really necessary.
    def _h_setlevel(self, idx):
        """
        Changes the array that header lines are written to.
        Supports writing different sections of the header file.
        """
        while len(self._hlines) <= idx:
            self._hlines.append([])
        self._hlevel = idx
        
    def _c_setlevel(self, idx):
        """
        Changes the array that source lines are written to.
        Supports writing to different sections of the source file.
        """
        while len(self._clines) <= idx:
            self._clines.append([])
        self._clevel = idx

    def parse_arguments(self):
        # Check for the argument that specifies path to the xcbgen python package.
        try:
            opts, args = getopt.getopt(sys.argv[1:], 'p:h:c:')

            if len(args) == 0:
                raise getopt.GetoptError('Missing filename')

        except getopt.GetoptError as err:
            print(err)
            print('Usage: c_client.py [-p python_module_path] [-h output_header_file] [-c output_source_file] file.xml')
            sys.exit(1)

        for (opt, arg) in opts:
            if opt == '-p':
                sys.path.insert(1, arg)
            elif opt == '-h':
                self.header_file = arg
            elif opt == '-c':
                self.source_file = arg
        
        self.input_file = args[0]
        

    def generate(self):
        # Parse the xml header
        module = Module(self.input_file, output)

        # Build type-registry and resolve type dependencies
        module.register()
        module.resolve()

        # Output the code
        module.generate()


# Jump to the bottom of this file for the main routine

_xcb_includes = {
    "xproto": "xcb.h"
}


def _get_xcb_include(client, ns):
    return _xcb_includes.get(ns, client._ns.file.replace(".xml", ".h"))


# Main routine starts here
if __name__ == "__main__":
    # Import the module class
    client = Client()
    client.parse_arguments()
    client.generate()
