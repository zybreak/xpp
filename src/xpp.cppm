module;
#include <xcb/xcb.h>

#define NEXT_TEMPLATE \
  void (&Next)(XcbIterator *)

#define NEXT_SIGNATURE \
  xpp::generic::signature<void (XcbIterator *), Next>

#define SIZEOF_TEMPLATE \
  int (&SizeOf)(const void *)

#define SIZEOF_SIGNATURE \
  xpp::generic::signature<int (const void *), SizeOf>

#define GETITERATOR_TEMPLATE \
  XcbIterator (&GetIterator)(const Reply *)

#define GETITERATOR_SIGNATURE \
  xpp::generic::signature<XcbIterator (const Reply *), GetIterator>

#define ACCESSOR_TEMPLATE \
  Data * (&Accessor)(const Reply *)

#define ACCESSOR_SIGNATURE \
  xpp::generic::signature<Data * (const Reply *), Accessor>

#define LENGTH_TEMPLATE \
  int (&Length)(const Reply *)

#define LENGTH_SIGNATURE \
  xpp::generic::signature<int (const Reply *), Length>

#define SIGNATURE(NAME) \
  xpp::generic::signature<decltype(NAME), NAME>

export module xpp;

import std;

export import xpp.generic;
export import xpp.proto.x;
export import xpp.core;
export import xpp.atom;
export import xpp.colormap;
export import xpp.cursor;
export import xpp.drawable;
export import xpp.font;
export import xpp.fontable;
export import xpp.gcontext;
export import xpp.pixmap;
export import xpp.window;
export import xpp.event;
export import xpp.connection;
