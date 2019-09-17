
PACKAGECONFIG = " \
	release \
	dbus udev evdev widgets tools libs \
	${@base_contains('DISTRO_FEATURES', 'opengl', 'gles2', '', d)} \
	jpeg libpng zlib \
	${@base_contains('DISTRO_FEATURES', 'pulseaudio', 'pulseaudio', '', d)} \
	accessibility \
	examples \
	eglfs \
	gbm \
	kms \
	"

QT_CONFIG_FLAGS += " -no-sse2 -no-opengles3 -no-mirclient "
