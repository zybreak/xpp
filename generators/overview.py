#!/usr/bin/env python
# vim: set ts=4 sw=4:

"""
This script converts XCB XML files into cpp code
"""

output = {
    'open': lambda module: open(module),
    'close': lambda module: close(module),
    'simple': lambda xtype, name: simple(xtype, name),
    'enum': lambda xtype, name: enum(xtype, name),
    'struct': lambda xtype, name: struct(xtype, name),
    'union': lambda xtype, name: union(xtype, name),
    'request': lambda xtype, name: request(xtype, name),
    'event': lambda xtype, name: event(xtype, name),
    'error': lambda xtype, name: error(xtype, name),
    'eventstruct': lambda xtype, name: eventstruct(xtype, name),
}

import getopt
import sys

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
import json

# Import the module class
from xcbgen.state import Module
from xcbgen.xtypes import *

_module : Module

def open(module: Module):
    global _module
    _module = module
    print("Open ->          %s" % module.namespace.path)

def close(module: Module):
    print("Close ->         %s" % module.namespace.path)

def simple(xtype: SimpleType, name: tuple[str]):
    c_type_setup(_module.namespace, xtype, name, ())
    print("Simple ->        %s \"%s\"" % (Type.type_name_to_str(xtype.name), "_".join(name)))

def enum(xtype: Enum, name: tuple[str]):
    c_type_setup(_module.namespace, xtype, name, ())
    print("Enum ->          %s \"%s\" { %s }" % (Type.type_name_to_str(xtype.name), "_".join(name), ", ".join(list(map(lambda f: "\"%s\": %s" % (f[0], f[1]), xtype.values)))))

def struct(xtype: Struct, name: tuple[str]):
    c_type_setup(_module.namespace, xtype, name, ())
    print("Struct ->        typeName: %s,\t\t\t name: \"%s\"" % (Type.type_name_to_str(xtype.name), "_".join(name)))

def union(xtype: Union, name: tuple[str]):
    c_type_setup(_module.namespace, xtype, name, ())
    print("Union ->         typeName: %s,\t\t\t name: \"%s\"" % (Type.type_name_to_str(xtype.name), "_".join(name)))

def request(xtype: Request, name: tuple[str]):
    c_type_setup(_module.namespace, xtype, name, ('request',))
    
    def arguments(fields: list[Field]):
        [print("    (%s) \"%s\": %s" % (str(f.type), f.field_name, Type.type_name_to_str(f.field_type))) for f in fields if f.visible]
        

    print("Request ->       %s \"%s\" {" % ("_".join(xtype.reply.name) if xtype.reply else "void", Type.type_name_to_str(xtype.name)))
    arguments(xtype.fields)
    print("}")

    if xtype.reply:
        c_type_setup(_module.namespace, xtype.reply, name, ('reply',))

        # Request prototypes
        #self._cpp_request_helper(xtype, name, False)

        print("Reply <-     %s {" % "_".join(xtype.reply.name))
        # Reply accessors
        arguments(xtype.reply.fields)
        #for (accessor) in c_accessors(_module.namespace, xtype.reply, name + ('reply',), name):
        #    print("     %s %s" % (accessor.return_type, accessor.member))
        print("}")

def event(xtype: Event, name: tuple[str]):
    c_type_setup(_module.namespace, xtype, name, ('event',))
    print("Event ->         typeName: %s,\t\t\t name: \"%s\"" % (Type.type_name_to_str(xtype.name), "_".join(name)))

def error(xtype: Error, name: tuple[str]):
    c_type_setup(_module.namespace, xtype, name, ('error',))
    print("Error ->         %s \"%s\"" % (Type.type_name_to_str(xtype.name), "_".join(name)))

def eventstruct(xtype: EventStruct, name: tuple[str]):
    c_type_setup(_module.namespace, xtype, name, ())
    print("EventStruct ->   typeName: %s,\t\t\t name: \"%s\"" % (Type.type_name_to_str(xtype.name), "_".join(name)))


# Main routine starts here
if __name__ == "__main__":
    # Parse the xml header
    mod = Module(sys.argv[1], output)

    # Build type-registry and resolve type dependencies
    mod.register()
    mod.resolve()

    # Output the code
    mod.generate()
