#!/usr/bin/env python
# vim: set ts=4 sw=4:

"""
This script converts XCB XML files into cpp code
"""

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


class Client(object):

    # global variable to keep track of serializers and
    # switch data types due to weird dependencies
    finished_switch: list[str] = []

    _cpp_request_names = []
    _cpp_request_objects = {}

    # see c_open()
    _interface_class = InterfaceClass()

    _cpp_events = []
    _cpp_errors = []

    _object_classes = {}

    _hlines: list[str] = []
    _hlevel = 0
    _ns = None

    header_file = None
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

        self._h('module;')

        if self._ns.header.lower() == 'xkb':
            self._h('#define explicit _explicit')
        self._h('#include <xcb/' + _get_xcb_include(self, self._ns.header.lower()) + '>')
        if self._ns.header.lower() == 'xkb':
            self._h('#undef _explicit')
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

        self._h('')
        self._h(ExtensionClass(self._ns).make_class())

        for cpp_event in self._cpp_events:
            self._h(cpp_event.make_class())

        self._h('')

        for cpp_error in self._cpp_errors:
            self._h(cpp_error.make_class())

        self._h('')

        for name in self._cpp_request_names:
            self._h("%s", self._cpp_request_objects[name].make_class())

        self._h('')

        for key in self._object_classes:
            self._h(self._object_classes[key].make_inline())

        self._h('')
        self._h(self._interface_class.make_proto())

        self._h('')
        self._h("} // namespace xpp::%s" % get_namespace(self._ns))

        self._h('')

        # Write header file
        def write_header(hfile):
            for list in self._hlines:
                for line in list:
                    hfile.write(line)
                    hfile.write('\n')

        if (self.header_file == None):
            write_header(sys.stdout)
        else:
            with open(self.header_file, "w", encoding="utf-8") as file:
                write_header(file)
     
                
    def cpp_simple(self, module, name):
        """
        Exported function that handles cardinal type declarations.
        These are types which are typedef'd to one of the CARDx's, char, float, etc.
        """
        c_type_setup(self, module, name, ())

        if module.name != name:
            # Typedef
            self._h_setlevel(0)


    def cpp_request(self, module, name):
        """
        Exported function that handles request declarations.
        """

        c_type_setup(self, module, name, ('request',))

        if module.reply:

            c_type_setup(self, module.reply, name, ('reply',))

            # Request prototypes
            self._cpp_request_helper(module, name, False)

            # Reply accessors
            c_accessors(self, module.reply, name + ('reply',), name)

        else:
            # Request prototypes
            self._cpp_request_helper(module, name, True)

    def cpp_event(self, module, name):
        """
        Exported function that handles event declarations.
        """

        c_type_setup(self, module, name, ('event',))

        opcode = _n(name, self._ns).upper()
        c_name = _t(module.name + ('event',), self._ns)

        cpp_event = CppEvent(module.opcodes[name], opcode, c_name, self._ns, name, module.fields)
        self._cpp_events.append(cpp_event)
        self._interface_class.add_event(cpp_event)

    def cpp_error(self, module, name):
        """
        Exported function that handles error declarations.
        """
        c_type_setup(self, module, name, ('error',))

        opcode_name = _n(name, self._ns).upper()
        c_name = _t(module.name + ('error',), self._ns)
        cpp_error = CppError(module, self._ns, name, c_name, module.opcodes[name], opcode_name)
        self._cpp_errors.append(cpp_error)
        self._interface_class.add_error(cpp_error)

    def _h(self, fmt, *args):
        """
        Writes the given line to the header file.
        """
        self._hlines[self._hlevel].append(fmt % args)


    # XXX See if this level thing is really necessary.
    def _h_setlevel(self, idx):
        """
        Changes the array that header lines are written to.
        Supports writing different sections of the header file.
        """
        while len(self._hlines) <= idx:
            self._hlines.append([])
        self._hlevel = idx

    def parse_arguments(self):
        # Check for the argument that specifies path to the xcbgen python package.
        try:
            opts, args = getopt.getopt(sys.argv[1:], 'p:h:')

            if len(args) == 0:
                raise getopt.GetoptError('Missing filename')

        except getopt.GetoptError as err:
            print(err)
            print('Usage: c_client.py [-p python_module_path] [-h output_header_file] file.xml')
            sys.exit(1)

        for (opt, arg) in opts:
            if opt == '-p':
                sys.path.insert(1, arg)
            elif opt == '-h':
                self.header_file = arg
        
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
    client = Client()

    # Must create an "output" dictionary before any xcbgen imports.
    output = {
        'open': lambda module: client.c_open(module),
        'close': lambda module: client.c_close(module),
        'simple': lambda module, name: client.cpp_simple(module, name),
        'enum': lambda x, y: None,
        'struct': lambda x, y: None,
        'union': lambda x, y: None,
        'request': lambda module, name: client.cpp_request(module, name),
        'event': lambda module, name: client.cpp_event(module, name),
        'error': lambda module, name: client.cpp_error(module, name),
        'eventstruct': lambda x, y: None,
    }

    # Import the module class
    try:
        from xcbgen.state import Module
        from xcbgen.xtypes import *
    except ImportError:
        print('''
    Failed to load the xcbgen Python package!
    Make sure that xcb/proto installed it on your Python path.
    If not, you will need to create a .pth file or define $PYTHONPATH
    to extend the path.
    Refer to the README file in xcb/proto for more info.
    ''')
        raise

    client.parse_arguments()
    client.generate()
