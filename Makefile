all:
	${MAKE} -C include/proto

examples:
	${MAKE} -C src/examples

.PHONY: ${DIRS}
