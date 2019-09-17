# When using devtool, this patch is applyed, but commit can't be handle properly as it only patches the git submodule
# Commit fails and then source code extraction is not possible with devtool
# The fix removes the patch and replace it by a specific 'sed' through a prefunc in configure task

SRC_URI_remove = "file://0001-introspection.m4-prefix-pkgconfig-paths-with-PKG_CON.patch"

do_update_common_submodule() {
   cd ${S}
   git submodule init
   git submodule update
}

addtask do_update_common_submodule before do_configure after do_unpack

# Create specific function to apply above patch
patch_introspection_m4_file() {
    # [PATCH] introspection.m4: prefix pkgconfig paths with PKG_CONFIG_SYSROOT_DIR
    sed -i \
           -e "s|INTROSPECTION_SCANNER=\`\$PKG_CONFIG --variable|INTROSPECTION_SCANNER=\$PKG_CONFIG_SYSROOT_DIR\`\$PKG_CONFIG --variable|" \
           -e "s|INTROSPECTION_COMPILER=\`\$PKG_CONFIG --variable|INTROSPECTION_COMPILER=\$PKG_CONFIG_SYSROOT_DIR\`\$PKG_CONFIG --variable|" \
           -e "s|INTROSPECTION_GENERATE=\`\$PKG_CONFIG --variable|INTROSPECTION_GENERATE=\$PKG_CONFIG_SYSROOT_DIR\`\$PKG_CONFIG --variable|" \
           -e "s|INTROSPECTION_MAKEFILE=\`\$PKG_CONFIG --variable|INTROSPECTION_MAKEFILE=\$PKG_CONFIG_SYSROOT_DIR\`\$PKG_CONFIG --variable|" \
           ${S}/common/m4/introspection.m4
}

# Add specific patch function to configure prefuncs to apply the patch
do_configure[prefuncs] += " patch_introspection_m4_file"

