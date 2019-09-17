ifndef _LIB_STREAMDECADAPT_MAKELIBRARY_MK_
_LIB_STREAMDECADAPT_MAKELIBRARY_MK_=y

LIB_STREAMDECADAPT_BUILDDIR=${ODIR}/libStreamDecAdapt
LIB_STREAMDECADAPT_LIBDIR=${LDIR}/libStreamDecAdapt

ALL_PREREQUISTES+=\
	lib_streamdecadapt_all

LINT_PREREQUISTES+=\
	lib_streamdecadapt_lint

CLEAN_PREREQUISTES+=\
	lib_streamdecadapt_clean

################################################################################
# Files ...
################################################################################
include ${WSPATH}/lib_streamdecadapt/Files.mk
################################################################################
# Source Dependencies if required ...
################################################################################
TMP_C_TARGET_OBJECTS+=$(patsubst %.c, %.o, $(addprefix ${LIB_STREAMDECADAPT_BUILDDIR}/,$(notdir ${LIB_STREAMDECADAPT_C_SOURCES})))
TMP_CPP_TARGET_OBJECTS+=$(patsubst %.cpp, %.o, $(addprefix ${LIB_STREAMDECADAPT_BUILDDIR}/,$(notdir ${LIB_STREAMDECADAPT_CPP_SOURCES})))
TMP_TARGET_DEPENDENCIES= $(TMP_C_TARGET_OBJECTS:.o=.d) $(TMP_CPP_TARGET_OBJECTS:.o=.d)
-include ${TMP_TARGET_DEPENDENCIES}
LIB_STREAMDECADAPT_LINTS+=$(patsubst %.c, %.lint, $(addprefix ${LIB_STREAMDECADAPT_BUILDDIR}/,$(notdir ${LIB_STREAMDECADAPT_C_SOURCES})))
################################################################################
# Targets ...
################################################################################
${LIB_STREAMDECADAPT_BUILDDIR}: 
	${MKDIR} ${LIB_STREAMDECADAPT_BUILDDIR}

ifndef CONFIG_TARGET_CPU_BUILDCFG_OMIT_COMPILING_LIB_STREAMDECADAPT

${LIB_STREAMDECADAPT_LIBDIR}/libStreamDecAdapt.a: ${LIB_STREAMDECADAPT_DEPENDENCIES} ${TARGET_CONFIG_PATH}/target_config.mak
	${MKDIR} ${LIB_STREAMDECADAPT_LIBDIR}
	${MAKE} \
		-f ${WSPATH}/lib_streamdecadapt/Makefile.libStreamDecAdapt \
		WSPATH=${WSPATH} \
		LIBPATH=${WSPATH}/lib_streamdecadapt \
		ODIR=${LIB_STREAMDECADAPT_BUILDDIR} \
		LDIR=${LIB_STREAMDECADAPT_LIBDIR} \
		LIBDIR=${LIBDIR} \
		TARGET_CONFIG_PATH=${TARGET_CONFIG_PATH} \
		TARGETPATH=${TARGETPATH} \
		${LIB_STREAMDECADAPT_LIBDIR}/libStreamDecAdapt.a

lib_streamdecadapt_all: ${LIB_STREAMDECADAPT_LIBS}

lib_streamdecadapt_clean:
	${RM} ${LIB_STREAMDECADAPT_LIBDIR}/libStreamDecAdapt.a

lib_streamdecadapt_lint: ${LIB_STREAMDECADAPT_SOURCES}
	${MAKE} \
		-f ${WSPATH}/lib_streamdecadapt/Makefile.libStreamDecAdapt \
		WSPATH=${WSPATH} \
		LIBPATH=${WSPATH}/lib_streamdecadapt \
		ODIR=${LIB_STREAMDECADAPT_BUILDDIR} \
		LDIR=${LIB_STREAMDECADAPT_LIBDIR} \
		LIBDIR=${LIBDIR} \
		TARGET_CONFIG_PATH=${TARGET_CONFIG_PATH} \
		TARGETPATH=${TARGETPATH} \
		${LIB_STREAMDECADAPT_LINTS}

endif #CONFIG_TARGET_CPU_BUILDCFG_OMIT_COMPILING_LIB_STREAMDECADAPT

endif #_LIB_STREAMDECADAPT_MAKELIBRARY_MK_
