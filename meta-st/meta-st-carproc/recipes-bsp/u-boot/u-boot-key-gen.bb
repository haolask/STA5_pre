LICENSE = "GPLv2+"

DEPENDS = "openssl-native"

inherit deploy

do_deploy () {
    mkdir -p ${UBOOT_SIGN_KEYDIR}
    if [ -z "$(ls -A ${UBOOT_SIGN_KEYDIR})" ]; then
	openssl genpkey -algorithm RSA -out ${DEPLOY_DIR_IMAGE}/keys/dev.key -pkeyopt rsa_keygen_bits:2048 -pkeyopt rsa_keygen_pubexp:65537
	openssl req -batch -new -x509 -key ${DEPLOY_DIR_IMAGE}/keys/dev.key -out ${DEPLOY_DIR_IMAGE}/keys/dev.crt
    fi
}

BBCLASSEXTEND += "native nativesdk"

do_deploy[nostamp] = "1"

addtask deploy before do_build after do_install
