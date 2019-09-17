#add_subdirectory(fatfs/src)
add_subdirectory(i2c_service/src)
add_subdirectory(spi_service/src)
add_subdirectory(liblzg/src)
if (EARLY_TUNER_FTR)
add_subdirectory(st-etal)
endif (EARLY_TUNER_FTR)
add_subdirectory(rpmsg/src)
add_subdirectory(hmac/src)

if (EARLY_TUNER_FTR)
set(EARLY_TUNER_TARGET "$<TARGET_OBJECTS:etalcore>")
endif (EARLY_TUNER_FTR)

add_library(middleware STATIC
	$<TARGET_OBJECTS:i2c_service>
	$<TARGET_OBJECTS:spi_service>
	$<TARGET_OBJECTS:liblzg>
	${EARLY_TUNER_TARGET}
	$<TARGET_OBJECTS:rpmsg>
	$<TARGET_OBJECTS:hmac>
	#	$<TARGET_OBJECTS:fatfs>
)

install(TARGETS middleware DESTINATION lib${LIB_SUFFIX})

