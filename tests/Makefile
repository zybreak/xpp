LIBS=x11 \
     xcb \
     xcb-icccm \
     xcb-sync \
     xcb-xf86dri \
     xcb-xprint \
     xcb-xinput \
     xcb-shape \
     xcb-shm \
     xcb-render \
     xcb-proto \
     xcb-event \
     xcb-xfixes \
     xcb-xkb \
     xcb-dri3 \
     xcb-ewmh \
     xcb-util \
     xcb-renderutil \
     xcb-xtest \
     xcb-xevie \
     xcb-keysyms \
     xcb-image \
     xcb-composite \
     xcb-randr \
     xcb-present \
     xcb-xv \
     xcb-aux \
     xcb-record \
     xcb-dpms \
     xcb-glx \
     xcb-atom \
     xcb-damage \
     xcb-screensaver \
     xcb-xvmc \
     xcb-res \
     xcb-xinerama \
     xcb-dri2

CXXFLAGS=-std=c++17 -Wall -O0 $(shell pkg-config --cflags ${LIBS})
LDFLAGS=$(shell pkg-config --libs ${LIBS})

# CXX=clang
CXXFLAGS+=-g
# CXXFLAGS+=-Wextra
# CXXFLAGS+=-ftime-report

CPPSRCS=event.cpp \
        requests.cpp \
        iterator.cpp

all: ${CPPSRCS}

${CPPSRCS}:
	${CXX} ${LDFLAGS} ${CXXFLAGS} -o $(@:%.cpp=%) $@

xlib-test: xlib-test.cpp
	${CXX} $(shell pkg-config --libs --cflags x11 xrandr) -o $@ $<

version:

clean:
	rm -f ${CPPSRCS:%.cpp=%}

.PHONY: ${CPPSRCS} clean
