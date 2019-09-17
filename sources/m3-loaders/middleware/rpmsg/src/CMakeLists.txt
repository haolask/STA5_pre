set(rpmsg_SRCS
	MessageQCopy.c
	NameMap.c
	VirtQueue.c
	resource.c
	sta_rpmsg.c
)

add_library(rpmsg OBJECT ${rpmsg_SRCS})
