set(liblzg_SRCS
	checksum.c
	decode.c
	version.c
)

add_library(liblzg OBJECT ${liblzg_SRCS})
