LICENSE = "Proprietary"

inherit st-image-license-summary

image_copy_image_license() {
    rm -f ${IMGDEPLOYDIR}/${IMAGE_NAME}.license
    for file in ${EULA_FILE_ST}; do
        if [ -f $file ]; then
            cat $file >> ${IMGDEPLOYDIR}/${IMAGE_NAME}.license
        else
            bbwarn "'$(basename $file)' license part not found"
        fi
    done
    cd ${IMGDEPLOYDIR}; ln -sf ${IMAGE_NAME}.license ${IMAGE_LINK_NAME}.license
}
IMAGE_POSTPROCESS_COMMAND_prepend = "image_copy_image_license;"
