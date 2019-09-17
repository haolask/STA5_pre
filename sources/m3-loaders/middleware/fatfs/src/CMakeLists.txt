set(fatfs_SRCS
	diskio.c
	ff.c
	syscall.c
)

add_library(fatfs OBJECT ${fatfs_SRCS})

