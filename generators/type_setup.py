"""
Type setup
"""
from accessor import Accessor
from utils import _n, _t, _cpp, _ext, _n_item

def _c_helper_absolute_name(prefix, field=None):
    """
    turn prefix, which is a list of tuples (name, separator, Type obj) into a string
    representing a valid name in C (based on the context)
    if field is not None, append the field name as well
    """
    prefix_str = ''
    for name, sep, obj in prefix:
        prefix_str += name
        if '' == sep:
            sep = '->'
            if ((obj.is_bitcase and obj.has_name) or  # named bitcase
                    (obj.is_switch and len(obj.parents) > 1)):
                sep = '.'
        prefix_str += sep
    if field is not None:
        prefix_str += _cpp(field.field_name)
    return prefix_str


def _c_helper_field_mapping(complex_type, prefix, flat=False):
    """
    generate absolute names, based on prefix, for all fields starting from complex_type
    if flat == True, nested complex types are not taken into account
    """
    all_fields = {}
    if complex_type.is_switch:
        for b in complex_type.bitcases:
            if b.type.has_name:
                bitcase_prefix = prefix + [(b.type.name[-1], '.', b.type)]
            else:
                bitcase_prefix = prefix

            if (True == flat and not b.type.has_name) or False == flat:
                all_fields.update(_c_helper_field_mapping(b.type, bitcase_prefix, flat))
    else:
        for f in complex_type.fields:
            fname = _c_helper_absolute_name(prefix, f)
            if f.field_name in all_fields:
                raise Exception("field name %s has been registered before" % f.field_name)

            all_fields[f.field_name] = (fname, f)
            if f.type.is_container and flat == False:
                if f.type.is_bitcase and not f.type.has_name:
                    new_prefix = prefix
                elif f.type.is_switch and len(f.type.parents) > 1:
                    # nested switch gets another separator
                    new_prefix = prefix + [(f.c_field_name, '.', f.type)]
                else:
                    new_prefix = prefix + [(f.c_field_name, '->', f.type)]
                all_fields.update(_c_helper_field_mapping(f.type, new_prefix, flat))

    return all_fields


def _c_complex(module):
    """
    Helper function for handling all structure types.
    Called for all structs, requests, replies, events, errors.
    """

    struct_fields = []
    maxtypelen = 0

    for field in module.fields:
        if not field.type.fixed_size() and not module.is_switch and not module.is_union:
            continue
        if field.wire:
            struct_fields.append(field)

    for field in struct_fields:
        length = len(field.c_field_type)
        # account for '*' pointer_spec
        if not field.type.fixed_size() and not module.is_union:
            length += 1
        maxtypelen = max(maxtypelen, length)


def c_type_setup(client, module, name, postfix):
    """
    Sets up all the C-related state by adding additional data fields to
    all Field and Type objects.  Here is where we figure out most of our
    variable and function names.

    Recurses into child fields and list member types.
    """
    # Do all the various names in advance

    module.c_type = _t(name + postfix, client._ns)
    module.c_wiretype = 'char' if module.c_type == 'void' else module.c_type

    module.c_iterator_type = _t(name + ('iterator',), client._ns)
    module.c_next_name = _n(name + ('next',), client._ns)
    module.c_end_name = _n(name + ('end',), client._ns)

    module.c_request_name = _n(name, client._ns)
    module.c_checked_name = _n(name, client._ns)
    module.c_unchecked_name = _n(name, client._ns)
    module.c_reply_name = _n(name + ('reply',), client._ns)
    module.c_reply_type = _t(name + ('reply',), client._ns)
    module.c_cookie_type = _t(name + ('cookie',), client._ns)
    module.c_reply_fds_name = _n(name + ('reply_fds',), client._ns)

    module.need_aux = False
    module.need_serialize = False
    module.need_sizeof = False

    module.c_aux_name = _n(name + ('aux',), client._ns)
    module.c_aux_checked_name = _n(name, client._ns)
    module.c_aux_unchecked_name = _n(name, client._ns)
    module.c_serialize_name = _n(name + ('serialize',), client._ns)
    module.c_unserialize_name = _n(name + ('unserialize',), client._ns)
    module.c_unpack_name = _n(name + ('unpack',), client._ns)
    module.c_sizeof_name = _n(name + ('sizeof',), client._ns)

    # special case: structs where variable size fields are followed by fixed size fields
    module.var_followed_by_fixed_fields = False

    if module.is_switch:
        module.need_serialize = True
        module.c_container = 'struct'
        for bitcase in module.bitcases:
            bitcase.c_field_name = _cpp(bitcase.field_name)
            bitcase_name = bitcase.field_type if bitcase.type.has_name else name
            c_type_setup(client, bitcase.type, bitcase_name, ())

    elif module.is_container:

        module.c_container = 'union' if module.is_union else 'struct'
        prev_varsized_field = None
        prev_varsized_offset = 0
        first_field_after_varsized = None

        for field in module.fields:
            c_type_setup(client, field.type, field.field_type, ())
            if field.type.is_list:
                c_type_setup(client, field.type.member, field.field_type, ())
                if field.type.nmemb is None:
                    module.need_sizeof = True

            field.c_field_type = _t(field.field_type, client._ns)
            field.c_field_const_type = ('' if field.type.nmemb == 1 else 'const ') + field.c_field_type
            field.c_field_name = _cpp(field.field_name)
            field.c_subscript = '[%d]' % field.type.nmemb if (field.type.nmemb and field.type.nmemb > 1) else ''
            field.c_pointer = ' ' if field.type.nmemb == 1 else '*'

            # correct the c_pointer field for variable size non-list types
            if not field.type.fixed_size() and field.c_pointer == ' ':
                field.c_pointer = '*'
            if field.type.is_list and not field.type.member.fixed_size():
                field.c_pointer = '*'

            if field.type.is_switch:
                field.c_pointer = '*'
                field.c_field_const_type = 'const ' + field.c_field_type
                module.need_aux = True
            elif not field.type.fixed_size() and not field.type.is_bitcase:
                module.need_sizeof = True

            field.c_iterator_type = _t(field.field_type + ('iterator',), client._ns)  # xcb_fieldtype_iterator_t
            field.c_iterator_name = _n(name + (field.field_name, 'iterator'), client._ns)  # xcb_container_field_iterator
            field.c_accessor_name = _n(name + (field.field_name,), client._ns)  # xcb_container_field
            field.c_length_name = _n(name + (field.field_name, 'length'), client._ns)  # xcb_container_field_length
            field.c_end_name = _n(name + (field.field_name, 'end'), client._ns)  # xcb_container_field_end

            field.prev_varsized_field = prev_varsized_field
            field.prev_varsized_offset = prev_varsized_offset

            if prev_varsized_offset == 0:
                first_field_after_varsized = field
            field.first_field_after_varsized = first_field_after_varsized

            if field.type.fixed_size():
                prev_varsized_offset += field.type.size
                # special case: intermixed fixed and variable size fields
                if prev_varsized_field is not None and not field.type.is_pad and field.wire:
                    if not module.is_union:
                        module.need_serialize = True
                        module.var_followed_by_fixed_fields = True
            else:
                module.last_varsized_field = field
                prev_varsized_field = field
                prev_varsized_offset = 0

            if module.var_followed_by_fixed_fields:
                if field.type.fixed_size():
                    field.prev_varsized_field = None

    if module.need_serialize:
        # when _unserialize() is wanted, create _sizeof() as well for consistency reasons
        module.need_sizeof = True

    # as switch does never appear at toplevel,
    # continue here with type construction
    if module.is_switch:
        if module.c_type not in client.finished_switch:
            client.finished_switch.append(module.c_type)
            # special: switch C structs get pointer fields for variable-sized members
            _c_complex(module)
            for bitcase in module.bitcases:
                bitcase_name = bitcase.type.name if bitcase.type.has_name else name
                c_accessors(client, bitcase.type, bitcase_name, bitcase_name)


def _c_accessors_list(client, obj, field):
    """
    Declares the accessor functions for a list field.
    Declares a direct-accessor function only if the list members are fixed size.
    Declares length and get-iterator functions always.
    """
    list = field.type

    # special case: switch
    # in case of switch, 2 params have to be supplied to certain accessor functions:
    #   1. the anchestor object (request or reply)
    #   2. the (anchestor) switch object
    # the reason is that switch is either a child of a request/reply or nested in another switch,
    # so whenever we need to access a length field, we might need to refer to some anchestor type
    switch_obj = obj if obj.is_switch else None
    if obj.is_bitcase:
        switch_obj = obj.parents[-1]

    params = []
    fields = {}
    parents = obj.parents if hasattr(obj, 'parents') else [obj]
    # 'R': parents[0] is always the 'toplevel' container type
    params.append(('const %s *R' % parents[0].c_type, parents[0]))
    fields.update(_c_helper_field_mapping(parents[0], [('R', '->', parents[0])], flat=True))

    if switch_obj is not None:
        # now look where the fields are defined that are needed to evaluate
        # the switch expr, and store the parent objects in accessor_params and
        # the fields in switch_fields

        # 'S': name for the 'toplevel' switch
        toplevel_switch = parents[1]
        params.append(('const %s *S' % toplevel_switch.c_type, toplevel_switch))
        fields.update(_c_helper_field_mapping(toplevel_switch, [('S', '->', toplevel_switch)], flat=True))

        # initialize prefix for everything "below" S
        prefix_str = '/* %s */ S' % toplevel_switch.name[-1]
        prefix = [(prefix_str, '->', toplevel_switch)]

        # look for fields in the remaining containers
        for p in parents[2:] + [obj]:
            # the separator between parent and child is always '.' here,
            # because of nested switch statements
            if not p.is_bitcase or (p.is_bitcase and p.has_name):
                prefix.append((p.name[-1], '.', p))
            fields.update(_c_helper_field_mapping(p, prefix, flat=True))

    client._h_setlevel(1)

    request_name = _ext(_n_item(obj.name[-1]))

    if not request_name in client._cpp_request_objects:
        return

    if list.member.fixed_size() and not obj.is_bitcase:
        if field.c_field_type == "char":
            client._cpp_request_objects[request_name].accessors.append(
                Accessor(is_string=True,
                         member=_ext(_n_item(field.field_name)),
                         c_name=_n(obj.name, client._ns))
                
            )

        else:
            client._cpp_request_objects[request_name].accessors.append(
                Accessor(is_fixed=True,
                         member=_ext(_n_item(field.field_name)),
                         c_type=field.c_field_type,
                         return_type="",  # 'Type' if field.c_field_type == 'void' else field.c_field_type,
                         iter_name="",
                         c_name=_n(obj.name, client._ns))
            )

    else:

        if not obj.is_bitcase:
            client._cpp_request_objects[request_name].accessors.append(
                Accessor(is_variable=True,
                         member=_ext(_n_item(field.field_name)),
                         c_type=field.c_field_type,
                         return_type='Type' if field.c_field_type == 'void' else field.c_field_type,
                         iter_name=_n(field.type.name, client._ns),
                         c_name=_n(obj.name, client._ns))
            )


def c_accessors(client, obj, name, base):
    """
    Declares the accessor functions for the fields of a structure.
    """
    # no accessors for switch itobj -
    # switch always needs to be unpacked explicitly

    for field in obj.fields:
        if field.type.is_list and not field.type.fixed_size():
            _c_accessors_list(client, obj, field)
        elif field.prev_varsized_field is not None or not field.type.fixed_size():
            pass
