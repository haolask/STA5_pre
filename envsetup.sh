#!/bin/bash -

_META_LAYER_ROOT=meta-st
_META_LAYER_NAME=${_META_LAYER_ROOT}/meta-st-carproc
_BUILDSYSTEM=poky

export ST_OE_DISTRO_CODENAME="rocko"
export ST_OE_META_LAYER_NAME="$_META_LAYER_NAME"


# Define default settings for STM envsetup call
_DISTRO=${DISTRO-"poky-st"}
_MACHINE=${MY_MACHINE-}

#Fix bitbake issue with morty on Ubuntu < 16.04
export LANG="en_US.UTF-8"

#----------------------------------------------
# Set VIEWROOT for oe sdk baseline
#
VIEWROOT=$PWD
while test ! -d "$VIEWROOT/${_META_LAYER_ROOT}" && test "$VIEWROOT" != "/"
do
	VIEWROOT=$(dirname $VIEWROOT)
done
if test "$VIEWROOT" == "/"
then
	echo "[ERROR]: you're trying to launch the script outside oe sdk tree"
	return 1
fi

_SITECONFSAMPLE_PATH="$VIEWROOT/$_META_LAYER_NAME/conf/template/$_DISTRO"

######################################################
# FUNCTION / ALIAS
# --
#

######################################################
# Envsetup help
stoe_carproc_help() {
    echo ""
    echo "=================================================="
    # Import here the help usage available with OpenSDK envsetup.sh:
    _stoe_help_usage
    echo "=================================================="
    # Import here the options available with OpenSDK envsetup.sh:
    _stoe_help_option
    echo "  --gnss"
    echo "      Enable GNSS-TESEO support in baseline"
    echo "  --tuner"
    echo "      Enable ETAL tuner support in baseline"
    echo "  --distro"
    echo "      Let user choose the distro to compile (default is poky-st)"
    echo "  --nfs"
    echo "      Enable NFS support for root filesystem (Developper usage)"
    echo "  --initramfs"
    echo "      Enable Initramfs to start the current image - set INITRAMFS_IMAGE variable in your env to use different image than default (core-image-st-carproc-initramfs) (Demo feature)"
    echo "  --weston-shell"
    echo "      Let user choose the weston shell (default is desktop-shell)"
    echo "  --optee"
    echo "      Enable or disable the optee BL32 image"
    echo ""
    echo "=================================================="
    # Import here the extra settings available with OpenSDK envsetup.sh:
    _stoe_help_extra
    echo ""
    echo "=================================================="
    echo ""
}

######################################################
# provide full list of STOE utilities available
_stoe_carproc_utilities() {
    echo "=================================================="
    # Import here the functions available with OpenSDK envsetup.sh:
    _stoe_utilities | sed -n '/==================================================/!p'
    echo "  stoe_clean_images       : clean images folder (tmp-glibc/deploy/images)"
    echo "  stoe_clean_repo         : reset hard for all git modules and remove all local generated files"
    echo "  stoe_update_repo        : update all git with latest commits"
    echo "  stoe_list_tasks <pack>  : list all tasks"
    echo "  stoe_git_show           : list all current SHA1 for each git repo"
    echo "=================================================="
}

######################################################
# extract requested VAR from conf files for BUILD_DIR provided
_stoe_config_read() {
    local builddir=$(realpath $1)
    local stoe_var=$2
    local findconfig=""
    if ! [[ -z $(grep -Rs "^[ \t]*$stoe_var[ \t]*=" $builddir/conf/*.conf) ]]; then
        # Config defined as "=" in conf file
        findconfig=$(grep -Rs "^[ \t]*$stoe_var[ \t]*=" $builddir/conf/*.conf)
    elif ! [[ -z $(grep -Rs "^[ \t]*$stoe_var[ \t]*?*=" $builddir/conf/*.conf) ]]; then
        # Config defined as "?=" in conf file
        findconfig=$(grep -Rs "^[ \t]*$stoe_var[ \t]*?*=" $builddir/conf/*.conf)
    elif ! [[ -z $(grep -Rs "^[#]*$stoe_var[ \t]*=" $builddir/conf/*.conf) ]]; then
        findconfig="\<disable\>"
    else
        # Config not found
        findconfig="\<no-custom-config-set\>"
    fi
    # Format config
    local formatedconfig=`echo $findconfig | sed -e 's|^.*"\(.*\)".*$|\1|g;s|\${TOPDIR}|\${builddir}|'`
    # Expand and export config
    eval echo "$formatedconfig"
}

######################################################
# alias function: display current configuration
#
stoe_config_summary() {
    local builddir=""
    if [[ $# == 0 ]]; then
        # Override builddir in case of none argument provided
        builddir=$BUILDDIR
    elif [ $(realpath $1) ]; then
        # Use provided dir as builddir
        builddir=$(realpath $1)
    else
        echo "ERROR: '$1' is not an existing BUILD_DIR."
        echo ""
        return 1
    fi
    echo ""
    echo "==========================================================================="
    echo "Configuration files have been created for the following configuration:"
    echo ""
    echo "    DISTRO            : " $(_stoe_config_read $builddir DISTRO)
    echo "    DISTRO_CODENAME   : " $ST_OE_DISTRO_CODENAME
    echo "    MACHINE           : " $(_stoe_config_read $builddir MACHINE)
    echo ""
    echo "    BUILD_DIR         : " $(basename $builddir)
    echo "    DOWNLOAD_DIR      : " $(_stoe_config_read $builddir DL_DIR)
    echo "    SSTATE_DIR        : " $(_stoe_config_read $builddir SSTATE_DIR)
    echo ""
    echo "    SOURCE_MIRROR_URL : " $(_stoe_config_read $builddir SOURCE_MIRROR_URL)
    echo "    SSTATE_MIRRORS    : " $(_stoe_config_read $builddir SSTATE_MIRRORS | sed 's|^.*\(http:.*\)/PATH.*$|\1|')
    echo ""
    echo "==========================================================================="
    echo ""
}

######################################################
# extract description for images provided
_stoe_list_images_descr() {
    for l in $1;
    do
        local image=`echo $l | sed -e 's#^.*/\([^/]*\).bb$#\1#'`
        if [ ! -z "$(grep "^SUMMARY[ \t]*=" $l)" ]; then
            local descr=`grep "^SUMMARY[ \t]*=" $l | sed -e 's/^.*"\(.*\)["\]$/\1/'`
        else
            local descr=`grep "^DESCRIPTION[ \t]*=" $l | sed -e 's/^.*"\(.*\)["\]$/\1/'`
        fi
        if [ -z "$descr" ] && [ "$2" == "ERR" ]; then
            echo ""
            echo "No description available for image: $image"
            echo "$l"
            echo ""
            return 1
        else
            printf "    %-33s  -   $descr\n" $image
        fi
    done
}

######################################################
# alias function: list all images available
#
stoe_list_images() {
    local metalayer=""
    if [[ $# == 0 ]]; then
        echo "ERROR: missing layer path."
        return 1
    elif [ -e $(realpath $1)/conf/layer.conf ] || [ $(realpath $1) = $(realpath ${VIEWROOT}/${_META_LAYER_ROOT}) ]; then
        # Use provided dir as metalayer
        metalayer=$(realpath $1)
    else
        echo "ERROR: '$1' is not an existing layer."
        echo ""
        return 1
    fi
    local err=$2
    local filter=$3

    local LIST=`find $metalayer/* -wholename "*/images/*.bb" -not -wholename "*/meta-skeleton/*" | sort`

    if [ "$filter" == "FILTER" ]; then
        local LAYERS_LIST=`find $metalayer/* -wholename "*/conf/layer.conf" -not -wholename "*/meta-skeleton/*" | sed 's#/conf/layer.conf##' | sort`
        # Filter for layer available in current bblayers.conf file
        unset LAYERS_SET
        for l in ${LAYERS_LIST}; do
            if ! [[ -z $(grep "${l#$(dirname $BUILDDIR)/}[ '\"]" $BUILDDIR/conf/bblayers.conf) ]]; then
                LAYERS_SET+=(${l})
            fi
        done
        if [ -z "${#LAYERS_SET[@]}" ]; then
            echo "[WARNING] None of the layers from $metalayer are defined in current $(basename $BUILDDIR)/conf/bblayers.conf file."
            echo
            return
        fi
        # Filter images from enabled layers
        unset IMAGE_SET
        for ITEM in ${LAYERS_SET[@]}; do
            for i in ${LIST}; do
                if [ "${i#$ITEM/}" != "$i" ]; then
                    IMAGE_SET+=(${i})
                fi
            done
        done
        if [ -z "${#IMAGE_SET[@]}" ]; then
            echo "[WARNING] From the layers of $metalayer enable in your $(basename $BUILDDIR)/conf/bblayers.conf file, there is no image available for build."
            echo
            return
        fi
        LIST="${IMAGE_SET[@]}"
    fi

    echo ""
    echo "==========================================================================="
    echo "Available images for '$metalayer' layer are:"
    echo ""
    _stoe_list_images_descr "$LIST" "$err"
    echo ""
}


######################################################
# alias function: enable ST premirror
#
stoe_source_premirror_enable() {
    if [ -e $BUILDDIR/conf/site.conf ]; then
        if [ -z "$(grep SOURCE_MIRROR_URL $BUILDDIR/conf/site.conf)" ]; then
            echo "ERROR: missing SOURCE_MIRROR_URL definition in $BUILDDIR/conf/site.conf..."
            echo "Nothing done!"
        else
            echo ">>> ENABLE SOURCE_MIRROR_URL in $BUILDDIR/conf/site.conf"
            sed -e 's|^.*\(SOURCE_MIRROR_URL.*\)$|\1|g' -i $BUILDDIR/conf/site.conf
        fi
    else
        echo "ERROR: missing site.conf in $BUILDDIR/conf/..."
        echo "Nothing done!"
    fi
}

######################################################
# alias function: disable sstate-cache mirror
#
stoe_sstate_mirror_disable() {
    if [ -e $BUILDDIR/conf/site.conf ]; then
        if [ -z "$(grep SSTATE_MIRRORS $BUILDDIR/conf/site.conf)" ]; then
            echo "[WARNING] no SSTATE_MIRRORS entry in site.conf from $BUILDDIR/conf/"
            echo "Nothing to do..."
        else
            echo ">>> DISABLE SSTATE_MIRRORS in $BUILDDIR/conf/site.conf"
            sed -e 's|^[ /t]*\(SSTATE_MIRRORS.*\)$|#\1|g' -i $BUILDDIR/conf/site.conf
        fi
    else
        echo "[WARNING] site.conf not found in $BUILDDIR/conf"
        echo "Nothing to do..."
    fi
}

######################################################
# alias function: enable sstate-cache mirror
#
stoe_sstate_mirror_enable() {
    if [ -e $BUILDDIR/conf/site.conf ]; then
        if [ -z "$(grep SSTATE_MIRRORS $BUILDDIR/conf/site.conf)" ]; then
            echo "ERROR: missing SSTATE_MIRRORS definition in $BUILDDIR/conf/site.conf..."
            echo "Nothing done!"
        else
            echo ">>> ENABLE SSTATE_MIRRORS in $BUILDDIR/conf/site.conf"
            sed -e 's|^.*\(SSTATE_MIRRORS.*\)$|\1|g' -i $BUILDDIR/conf/site.conf
        fi
    else
        echo "ERROR: missing site.conf in $BUILDDIR/conf/..."
        echo "Nothing done!"
    fi
}


######################################################
# Apply configuration to site.conf file
#
conf_siteconf()
{
    if [ -f conf/site.conf ]; then
        echo "[WARNING] site.conf already exists. Nothing done..."
        return
    fi

    _NCPU=$(grep '^processor' /proc/cpuinfo 2>/dev/null | wc -l)
    # Sanity check that we have a valid number, if not then fallback to a safe default
    [ "$_NCPU" -ge 1 ] 2>/dev/null || _NCPU=2
    if [ -e "$_TEMPLATECONF" ]; then
        _SITECONFSAMPLE_PATH="$_TEMPLATECONF"
    fi
    if [ -f ${_SITECONFSAMPLE_PATH}/site.conf.sample ]; then
        # Copy default site.conf.sample to conf/site.conf
        cp -f ${_SITECONFSAMPLE_PATH}/site.conf.sample conf/site.conf
        # Update site.conf with expected settings
        sed -e 's|##_DISTRO_CODENAME##|'"${ST_OE_DISTRO_CODENAME}"'|g' -i conf/site.conf
        sed -e 's|##_NCPU##|'"${_NCPU}"'|g' -i conf/site.conf
        # Override default settings if requested
        if ! [ -z "$FORCE_DL_CACHEPREFIX" ]; then
            sed -e 's|^[#]*DL_DIR.*|DL_DIR = "'"${FORCE_DL_CACHEPREFIX}"'/oe-downloads"|g' -i conf/site.conf
        fi
        if ! [ -z "$FORCE_SSTATE_CACHEPREFIX" ]; then
            sed -e 's|^[#]*SSTATE_DIR.*|SSTATE_DIR = "'"${FORCE_SSTATE_CACHEPREFIX}"'/oe-sstate-cache"|g' -i conf/site.conf
        else
            # By default set sstate dir at root of baseline to share it among all build folders
            sed -e 's|^[#]*SSTATE_DIR.*|SSTATE_DIR = "'"${VIEWROOT}"'/sstate-cache"|g' -i conf/site.conf
        fi
        if ! [ -z "$FORCE_SOURCE_MIRROR_URL" ]; then
            sed -e 's|^[#]*SOURCE_MIRROR_URL.*|SOURCE_MIRROR_URL = "'"${FORCE_SOURCE_MIRROR_URL}"'"|g' -i conf/site.conf
            sed -e 's|^[#]*BB_GENERATE_MIRROR_TARBALLS = \"1\"|BB_GENERATE_MIRROR_TARBALLS = \"1\"|g' -i conf/site.conf
        fi
        if ! [ -z "$FORCE_SSTATE_MIRROR_URL" ]; then
            sed -e 's|^[#]*SSTATE_MIRRORS = ".*\(/PATH;.*\)"$|SSTATE_MIRRORS = "file://\.\* '"${FORCE_SSTATE_MIRROR_URL}"'\2"|g' -i conf/site.conf
        fi
    else
        echo "[INFO] No 'site.conf.sample' file available at ${_SITECONFSAMPLE_PATH}. No customization done..."
    fi

    # Update OE default terminal in site.conf file (temporary patch for Ubuntu 14.04 LTS)
    _temp_patch__gnometerminal
}

######################################################
# Apply configuration to local.conf file
#
conf_localconf()
{
    if [ -z "$(grep '^MACHINE =' conf/local.conf)" ]; then
        # Apply selected MACHINE in local conf file
        sed -e 's/^\(MACHINE.*\)$/#\1\nMACHINE = "'"$MACHINE"'"/' -i conf/local.conf
    else
        echo "[WARNING] MACHINE is already set in local.conf. Nothing done..."
    fi
    if [ -z "$(grep '^DISTRO =' conf/local.conf)" ]; then
        # Apply selected DISTRO in local conf file
        sed -e 's/^\(DISTRO.*\)$/#\1\nDISTRO = "'"$DISTRO"'"/' -i conf/local.conf
    else
        echo "[WARNING] DISTRO is already set in local.conf. Nothing done..."
    fi
}

######################################################
# Apply configuration to bblayer.conf file
#
conf_bblayerconf()
{
    local _MACH_CONF=`find ${VIEWROOT}/${_META_LAYER_ROOT} -name ${MACHINE}.conf`
    local _BSP_LAYER_REQUIRED=`echo ${_MACH_CONF} | sed -n 's/.*\(meta-.*\)\(\/conf\/machine\/\).*/\1/p'`
    local _LAYERS=`\grep '^#@NEEDED_BSPLAYERS:' $_MACH_CONF`
    local _BSP=`echo ${_LAYERS} |cut -f 2 -d ':'`
    if [ -n "${_BSP}" ]; then
        cat >> conf/bblayers.conf <<EOF
# BSP dependencies"
EOF
        for bsp in $_BSP; do
            bsp_to_add=`echo $bsp | tr -d ' '`
            cat >> conf/bblayers.conf <<EOF
BBLAYERS =+ "${VIEWROOT}/$bsp_to_add"
EOF
        done
    fi
    if [ -n "$_BSP_LAYER_REQUIRED" -a \
        "${_LAYERS#*${_BSP_LAYER_REQUIRED}}" = "${_LAYERS}" -a \
        "$(grep "${_BSP_LAYER_REQUIRED}" conf/bblayers.conf)" == "" ]; then
        cat >> conf/bblayers.conf <<EOF

# specific bsp selected
BBLAYERS =+ "${VIEWROOT}/$_META_LAYER_ROOT/$_BSP_LAYER_REQUIRED"
EOF
    fi
}

######################################################
# get folder to use for template.conf files
#
get_templateconf()
{
    if [ "$DISTRO" == "nodistro" ]; then
        #for nodistro choice use default sample files from openembedded-core
        echo ""
        echo "[WARNING] Using default openembedded template configuration files for '$DISTRO' setting."
        echo ""
        _TEMPLATECONF=""
    else
        #extract bsp path
        local distro_path=$(\find ${VIEWROOT}/$_META_LAYER_ROOT/ -name "$DISTRO.conf" | sed 's|\(.*\)/conf/distro/\(.*\)|\1|')
        if [ -z "$distro_path" ]; then
            echo ""
            echo "ERROR: No '$DISTRO.conf' file available in $_META_LAYER_ROOT"
            echo ""
            return 1
        fi
        if [ -f $distro_path/conf/template/$DISTRO/bblayers.conf.sample ]; then
            _TEMPLATECONF=$distro_path/conf/template/$DISTRO
        else
            echo "[WARNING] default template configuration files not found in $_META_LAYER_ROOT layer: using default ones from poky"
            _TEMPLATECONF=""
        fi
    fi
}


######################################################
# Check last modified time for bblayers.conf from list of builddir provided and
# provide builddir that contains the latest bblayers.conf modified
#
_default_config_get() {
    local list=$1
    TmpFile=$(mktemp)
    for l in $list
    do
        [ -f ${VIEWROOT}/$l/conf/bblayers.conf ] && echo $(stat -c %Y ${VIEWROOT}/$l/conf/bblayers.conf) $l >> $TmpFile
    done
    cat $TmpFile | sort -r | head -n1 | cut -d' ' -f2
    rm -f $TmpFile
}

######################################################
# Init timestamp on bblayers.conf for builddir set
#
_default_config_set() {
    [ -f $BUILDDIR/conf/bblayers.conf ] && touch $BUILDDIR/conf/bblayers.conf
    export ST_OE_ROOT_DIR=`realpath $PWD/`
    export ST_OE_BUILD_DIR=$ST_OE_ROOT_DIR/$BUILD_DIR
    export ST_OE_SOURCES_DIR=$ST_OE_ROOT_DIR/sources
}


######################################################
# Format DISTRO and MACHINE list from configuration file list applying the specific _FORMAT_PATTERN:
#  <CONFIG-NAME>|<_FORMAT_PATTERN>|<CONFIG-DESCRIPTION>
#
_choice_formated_configs() {
    TmpFile=$(mktemp)
    local choices=$(find ${VIEWROOT}/$_META_LAYER_ROOT/ -wholename "*/conf/$1/*.conf" 2>/dev/null | sort | uniq)
    for ITEM in $choices
    do
        if [[ -z "$(grep '#@DESCRIPTION' $ITEM)" ]]; then
            echo ""
            echo "ERROR: No '#@DESCRIPTION' field available in $__CONGIG file:"
            echo "$ITEM"
            echo ""
            rm -f $TmpFile
            return 1
        fi
    done
    unset ITEM
    echo "$(echo $choices | xargs grep -H "#@DESCRIPTION" | sed 's|^.*/\(.*\)\.conf:#@DESCRIPTION:[ \t]*\(.*$\)|\1::'"${_FORMAT_PATTERN}"'\2|')" >> $TmpFile
    echo "$(cat $TmpFile | column -t -s "::")"
    rm -f $TmpFile
}

######################################################
# Format BUILD_DIR list from applying the specific _FORMAT_PATTERN:
#  <DIR-NAME>|<_FORMAT_PATTERN>|<DISTRO-value and MACHINE-value>
#
_choice_formated_dirs() {
    TmpFile=$(mktemp)
    for dir in $1
    do
        echo -e "${dir}${_FORMAT_PATTERN} :: DISTRO is '$(_stoe_config_read ${VIEWROOT}/$dir DISTRO)' and MACHINE is '$(_stoe_config_read ${VIEWROOT}/$dir MACHINE)'" >> $TmpFile
    done
    # Add new build config option
    echo "NEW${_FORMAT_PATTERN} :: *** SET NEW DISTRO AND MACHINE BUILD CONFIG ***" >> $TmpFile
    echo "$(cat $TmpFile | column -t -s "::")"
    rm -f $TmpFile
}

######################################################
# Make selection for <TARGET> requested from <LISTING> provided using shell or ui choice
#
_choice_shell() {
    local choice_name=$1
    local choice_list=$2
    local default_choice=$3
    #format list to have display aligned on column with '-' separation between name and description
    local options=$(echo "${choice_list}" | column -t -s "::")
    #change separator from 'space' to 'end of line' for 'select' command
    old_IFS=$IFS
    IFS=$'\n'
    local i=1
    unset LAUNCH_MENU_CHOICES
    for opt in $options; do
        printf "%3.3s. %s\n" $i $opt
        LAUNCH_MENU_CHOICES=(${LAUNCH_MENU_CHOICES[@]} $opt)
        i=$(($i+1))
    done
    IFS=$old_IFS
    # Item selection from list
    local selection=""
    # Init default_choice if not already provided
    [ -z "${default_choice}" ] && default_choice=1
    while [ -z "$selection" ]; do
        echo -n "Which one would you like? [${default_choice}] "
        read -t $READTIMEOUT answer
        # Check that user has answered before timeout, else break
        test "$?" -gt "128" && break

        if [ -z "$answer" ]; then
            selection=${LAUNCH_MENU_CHOICES[0]}
            break
        fi
        if [[ $answer =~ ^[0-9]+$ ]]; then
            if [ $answer -gt 0 ] && [ $answer -le ${#LAUNCH_MENU_CHOICES[@]} ]; then
                selection=${LAUNCH_MENU_CHOICES[$(($answer-1))]}
                break
            fi
        fi
        echo "Invalid choice: $answer"
        echo "Please use numeric value between '1' and '$(echo "$options" | wc -l)'"
    done
    eval ${choice_name}=$(echo $selection | cut -d' ' -f1)
}



choice() {
    local __TARGET=$1
    local choices="$2"
    local default_choice=$3
    echo "[$__TARGET configuration]"
    if [[ $(echo "$choices" | wc -l) -eq 1 ]]; then
        eval $__TARGET=$(echo $choices | awk -F''"${_FORMAT_PATTERN}"'' '{print $1}')
    else
        _choice_shell $__TARGET "$choices" $default_choice
    fi
    echo "Selected $__TARGET: $(eval echo \$$__TARGET)"
    echo ""
}


######################################################
# alias function: display current configuration
#
stoe_carproc_config_summary() {
    test -e $VIEWROOT/$1 || { echo "ERROR: '$VIEWROOT/$1' is not an existing BUILD_DIR."; return 1;}

    TmpSummary=$(mktemp)
    stoe_config_summary $VIEWROOT/$1 > $TmpSummary

    # Update NUMBER_THREADS and PARRALLEL_MAKE values
    local _NCPU=$(grep '^processor' /proc/cpuinfo 2>/dev/null | wc -l)
    sed -e 's|^\([ \t]*\)BB_NUMBER_THREADS\([ \t]*:\).*$|\1BB_NUMBER_THREADS\2  '"$_NCPU"'|' -i $TmpSummary
    sed -e 's|^\([ \t]*\)PARALLEL_MAKE\([ \t]*:\).*$|\1PARALLEL_MAKE\2  -j '"$_NCPU"'|' -i $TmpSummary

    cat $TmpSummary
    rm $TmpSummary
}

######################################################
# alias function: remove all the image present
#
stoe_clean_images() {
    _PWD_PREVIOUS=$(pwd)
    if [ -e "$BUILD_DIR/tmp-glibc/deploy/images" ];
    then
        echo "   ---------"
        echo "** Removing $BUILD_DIR/tmp-glibc/deploy/images   ---------"
        rm -rvf $BUILD_DIR/tmp-glibc/deploy/images
        echo "** Done   ---------"
        echo "   ---------"
    else
        echo "   ---------"
        echo "** Nothing to be done   ---------"
        echo "   ---------"
    fi
    cd $_PWD_PREVIOUS
}

######################################################
# alias function: git clean on each git project
#
stoe_clean_repo() {
    _PWD_PREVIOUS=$(pwd)
    cd $VIEWROOT
    echo ""
    echo "Cleaning all repo ...."
    echo ""
    repo forall -c 'echo $PWD; git reset --hard; git clean -fdx'
    echo ""
    echo "Repo clean done ...."
    cd $_PWD_PREVIOUS
}

######################################################
# alias function: git fetch on each git of the project
#
stoe_update_repo() {
    _PWD_PREVIOUS=$(pwd)
    cd $VIEWROOT
    echo ""
    echo "Updating all git repo ..."
    echo ""
    repo forall -c 'echo $PWD; git fetch --all;echo ""'
    echo ""
    echo "Repo update done ..."
    echo ""
    cd $_PWD_PREVIOUS
}

######################################################
# alias function: list task of specific package via bitbake
#
stoe_list_tasks() {
    if [ $# -eq 1 ];
    then
        _ST_PACKAGE=$1
        echo "List of Task for $1:"
        bitbake -c listtasks $_ST_PACKAGE | grep "^do" | sed -e "s/do_/  /"
    else
        echo "[ERROR]: bad number of argument"
        echo "[USAGE]: stoe_list_tasks <package>"

    fi
}

######################################################
# alias function: list SHA1 of each git modules
#
stoe_git_show() {
    _PWD_PREVIOUS=$(pwd)
    cd $VIEWROOT
    echo ""
    echo "Listing current SHA1  ...."
    echo ""
    repo forall -c 'echo $PWD; git log --pretty=tform:%H -1 ;echo ""'
    echo ""
    echo "Done ...."
    cd $_PWD_PREVIOUS
}

######################################################
# alias function: apply specific patch for linux component
#
_stoe_carproc_add_checkpatch_hooks() {
        echo ""
        echo "=================================================="
        echo "Patching hooks in Linux & U-boot components ..."
		for hook in `ls $VIEWROOT/$_META_LAYER_NAME/scripts/git-hooks/*`;
		do
			if [ -d $ST_OE_SOURCES_DIR/linux-xlnx/.git ]; then ln -sf $hook $ST_OE_SOURCES_DIR/linux-xlnx/.git/hooks > /dev/null; fi
			if [ -d $ST_OE_SOURCES_DIR/linux/.git ]; then ln -sf $hook $ST_OE_SOURCES_DIR/linux/.git/hooks > /dev/null; fi
			if [ -d $ST_OE_SOURCES_DIR/u-boot/.git ]; then ln -sf $hook $ST_OE_SOURCES_DIR/u-boot/.git/hooks > /dev/null; fi
		done
        echo ""
        echo "Patching done..."
        echo ""
}

######################################################
# Choose gnss configuration
#
stoe_set_gnss_options() {

	if [ "$(grep "GNSS_TESEO_PACKAGE_PATH" conf/local.conf)" == "" -a $_GNSS_CHOICE -ne 1 ]; then
		return;
	fi
	gnss_package=$(grep "GNSS_TESEO_PACKAGE_PATH" conf/local.conf | rev | cut -d'"' -f2 | rev)
	gnss_option=$(grep "GNSS_TESEO_OPTION" conf/local.conf | rev | cut -d'"' -f2 | rev)

	echo -e "=================================================="
	echo -e "GNSS "
	echo -e "Current configuration: "
	echo -e "\tPackage source: \t$gnss_package"
	echo -e "\tCompilation option: \t$gnss_option"

	if [ $_GNSS_CHOICE -ne 0 ]; then
		echo -e "Package configuration [Lib sources (L)/(d) Dev Sources]? "
		read _config
		if [ "$_config" == "D" -o "$_config" == "d" ]; then
			GNSS_VERSION="PREFERRED_VERSION_gnss-teseo=\"dev-st\""
		else
			GNSS_VERSION=""
		fi
		echo -e "Enable Dead Reckoning (y/N) ?"
		read _option
		if [ "$_option" == "Y" -o "$_option" == "y" ]; then
			GNSS_TESEO_OPTION="DR"
		else
			GNSS_TESEO_OPTION=""
		fi

		if [ "$GNSS_VERSION" != "" ];then
			if [ "$(grep "$GNSS_VERSION" conf/local.conf)" == "" ]; then
				echo "$GNSS_VERSION" >> conf/local.conf
			fi
		else
			sed -i '/PREFERRED_VERSION_gnss-teseo.*$/d' conf/local.conf
		fi

		if [ "$(grep "GNSS_TESEO_OPTION" conf/local.conf)" == "" ];then
			echo "GNSS_TESEO_OPTION ?= \"$GNSS_TESEO_OPTION\"" >> conf/local.conf
		else
			sed -i "s#GNSS_TESEO_OPTION ?= \".*\"#GNSS_TESEO_OPTION ?= \"$GNSS_TESEO_OPTION\"#" conf/local.conf
		fi

		echo -e "------------------------\n"
		echo -e "Applied GNSS configuration: "
		echo -e "\tPackage source: \t$GNSS_TESEO_PACKAGE_PATH"
		echo -e "\tCompilation option: \t$GNSS_TESEO_OPTION"
	fi
	echo -e "\n=================================================="
}

######################################################
# Choose tuner configuration
#
stoe_set_tuner_options() {

	if [ "$(grep "ETAL_OPTION" conf/local.conf)" == "" -a $_TUNER_CHOICE -ne 1 ]; then
		return;
	fi

	etal_option=$(grep "ETAL_OPTION"  conf/local.conf| rev | cut -d'"' -f2 | rev)

	echo -e "TUNER"
	echo -e "Current configuration: "
	echo -e "\tPackage option: \t$etal_option"
	if [ $_TUNER_CHOICE -ne 0 ]; then
		echo "Package options:"
		options=("DAB" "HD" "DAB_HOST_SD")
		PS3="Please select Package option number: "
		select _option in "${options[@]}" "NONE"; do
			case $_option in
				"${options[0]}"|"${options[1]}"|"${options[2]}")
					ETAL_OPTION=$_option
					break
					;;
				"NONE")
					ETAL_OPTION=""
					break
					;;
				*)
					echo "invalid package option number"
					;;
			esac
		done

		if [ "$(grep "ETAL_OPTION" conf/local.conf)" == "" ];then
			echo "ETAL_OPTION ?= \"$ETAL_OPTION\"" >> conf/local.conf
		else
			sed -i "s#ETAL_OPTION ?= \".*\"#ETAL_OPTION ?= \"$ETAL_OPTION\"#" conf/local.conf
		fi
		echo -e "------------------------\n"
		echo -e "Applied TUNER configuration: "
		echo -e "\tPackage option: \t$ETAL_OPTION"
	fi
	echo -e "\n=================================================="
}


######################################################
# Enable NFS configuration
#
stoe_set_nfs_options() {
	if [ "$(grep "^ROOTFS_DEVICE" conf/local.conf)" == "" -a $_NFS_CHOICE -ne 1 ]; then
		return;
	fi

	nfs_option=$(grep "^ROOTFS_DEVICE"  conf/local.conf| rev | cut -d'"' -f2 | rev)

	echo -e "Current NFS configuration: "
	if [ "$nfs_option" == "" ]; then
		echo -e "\tDISABLED"
	else
		echo -e "\tENABLED"
	fi

	if [ $_NFS_CHOICE -ne 0 ]; then
		initramfs_option=$(grep "^INITRAMFS_IMAGE"  conf/local.conf| rev | cut -d'"' -f2 | rev)
		if [ "$initramfs_option" != "" ]; then
		    echo "INITRAMFS is enabled. It is not (yet) compatible with NFS"
		    return
		fi
		echo -e "Use NFS (Y/n)?"
		read _option
		if [ "$_option" == "N" -o "$_option" == "n" ]; then
			sed -i '/ROOTFS_DEVICE ?= .*$/d' conf/local.conf
			sed -i '/IMAGE_FSTYPES ?= .*$/d' conf/local.conf
		else
			ROOTFS_DEVICE="NFS"
			IMAGE_FSTYPES="tar.gz"

			if [ "$(grep "ROOTFS_DEVICE" conf/local.conf)" == "" ];then
				echo "ROOTFS_DEVICE ?= \"$ROOTFS_DEVICE\"" >> conf/local.conf
			else
				sed -i "s#ROOTFS_DEVICE ?= \".*\"#ROOTFS_DEVICE ?= \"$ROOTFS_DEVICE\"#" conf/local.conf
			fi
			if [ "$(grep "IMAGE_FSTYPES" conf/local.conf)" == "" ];then
				echo "IMAGE_FSTYPES ?= \"$IMAGE_FSTYPES\"" >> conf/local.conf
			else
				sed -i "s#IMAGE_FSTYPES ?= \".*\"#IMAGE_FSTYPES ?= \"$IMAGE_FSTYPES\"#" conf/local.conf
			fi
		fi

	fi
	echo -e "\n=================================================="
}

######################################################
# Enable initramfs configuration
#
stoe_set_initramfs_options() {
	if [ "$(grep "^INITRAMFS_IMAGE" conf/local.conf)" == "" -a $_INITRAMFS_CHOICE -ne 1 ]; then
		return;
	fi

	initramfs_option=$(grep "^INITRAMFS_IMAGE"  conf/local.conf| rev | cut -d'"' -f2 | rev)

	echo -e "Current INITRAMFS configuration: "
	if [ "$initramfs_option" == "" ]; then
		echo -e "\tDISABLED"
	else
		echo -e "\tENABLED"
	fi

	if [ $_INITRAMFS_CHOICE -ne 0 ]; then
		nfs_option=$(grep "^ROOTFS_DEVICE"  conf/local.conf| rev | cut -d'"' -f2 | rev)
		if [ "$nfs_option" != "" ]; then
		    echo "NFS is enabled. It is not (yet) compatible with initramfs"
		    return
		fi
		echo -e "Use INITRAMFS (Y/n)?"
		read _option
		if [ "$_option" == "N" -o "$_option" == "n" ]; then
			sed -i '/INITRAMFS_IMAGE = .*$/d' conf/local.conf
			sed -i '/INITRAMFS_IMAGE_BUNDLE = .*$/d' conf/local.conf
			sed -i '/IMAGE_FSTYPES = "squashfs"$/d' conf/local.conf
			sed -i '/IMAGE_OVERHEAD_FACTOR.*$/d' conf/local.conf
		else
			INITRAMFS_IMAGE=${FORCE_INITRAMFS_IMAGE-"core-image-st-carproc-initramfs"}
			INITRAMFS_IMAGE_BUNDLE=${FORCE_INITRAMFS_IMAGE_BUNDLE-"1"}
			INITRAMFS_IMAGE_OVERHEAD_FACTOR=${FORCE_INITRAMFS_IMAGE_OVERHEAD_FACTOR-"1"}
			INITRAMFS_IMAGE_FSTYPES=${FORCE_INITRAMFS_IMAGE_FSTYPES-"squashfs"}

			if [ "$(grep "INITRAMFS_IMAGE =" conf/local.conf)" == "" ];then
				echo "INITRAMFS_IMAGE = \"$INITRAMFS_IMAGE\"" >> conf/local.conf
			else
				sed -i "s#INITRAMFS_IMAGE = \".*\"#INITRAMFS_IMAGE = \"$INITRAMFS_IMAGE\"#" conf/local.conf
			fi
			if [ "$(grep "INITRAMFS_IMAGE_BUNDLE =" conf/local.conf)" == "" ];then
				echo "INITRAMFS_IMAGE_BUNDLE = \"$INITRAMFS_IMAGE_BUNDLE\"" >> conf/local.conf
			else
				sed -i "s#INITRAMFS_IMAGE_BUNDLE = \".*\"#INITRAMFS_IMAGE_BUNDLE = \"$INITRAMFS_IMAGE_BUNDLE\"#" conf/local.conf
			fi
			if [ "$(grep "IMAGE_FSTYPES =" conf/local.conf)" == "" ];then
				echo "IMAGE_FSTYPES = \"$INITRAMFS_IMAGE_FSTYPES\"" >> conf/local.conf
			else
				sed -i "s#IMAGE_FSTYPES = \".*\"#IMAGE_FSTYPES = \"$INITRAMFS_IMAGE_FSTYPES\"#" conf/local.conf
			fi
			if [ "$(grep "IMAGE_OVERHEAD_FACTOR =" conf/local.conf)" == "" ];then
				echo "IMAGE_OVERHEAD_FACTOR = \"$INITRAMFS_IMAGE_OVERHEAD_FACTOR\"" >> conf/local.conf
			else
				sed -i "s#IMAGE_OVERHEAD_FACTOR = \".*\"#IMAGE_OVERHEAD_FACTOR = \"$INITRAMFS_IMAGE_OVERHEAD_FACTOR\"#" conf/local.conf
			fi
		fi

	fi
	echo -e "\n=================================================="
}

######################################################
# Enable WESTON SHELL configuration
#
stoe_set_weston_shell_options() {

	weston_shell_option=$(grep "desktop_shell\|ivi_shell_ivi_ctrl\|ivi_shell_hmi_ctrl"  conf/local.conf| rev | cut -d'"' -f2 | rev | sed 's/://g')
	if [ "$weston_shell_option" == "" ]; then
		weston_shell_option="desktop_shell"
	fi

	echo -e "Current WESTON SHELL configuration: "
	echo -e "    $weston_shell_option"
 
	sed -i "/STWESTONSHELL/d" conf/local.conf
	
	if [ $_WESTON_SHELL_CHOICE -ne 0 ]; then
		echo -e "Use 1.ivi-shell-hmi-ctrl, 2.ivi-shell-ivi-ctrl, 3.desktop-shell (1/2/3)?"
		read _option

		if [ "$_option" == "1" ]; then
			weston_shell_option="ivi_shell_hmi_ctrl"
		elif [ "$_option" == "2" ]; then
			weston_shell_option="ivi_shell_ivi_ctrl"
		elif [ "$_option" == "3" ]; then
			weston_shell_option="desktop_shell"
		else
			echo -e "ERROR : Incorrect choice of weston shell !"
		fi
	fi
        echo "STWESTONSHELL= \"$weston_shell_option\" " >> conf/local.conf

	echo -e "\n=================================================="
}


######################################################
# Enable Optee configuration (disabled by defaut)
#
stoe_set_optee_options() {
	if [ "$(grep "^BL32_SP" conf/local.conf)" == "" -a $_OPTEE_CHOICE -ne 1 ]; then
		return;
	fi

	optee_option=$(grep "^BL32_SP"  conf/local.conf| rev | cut -d'"' -f2 | rev)

	echo -e "Current OPTEE configuration: "
	if [ "$optee_option" == "" -o "$optee_option" == "sp_min" ]; then
		echo -e "\tDISABLED"
	else
		echo -e "\tENABLED"
	fi

	if [ $_OPTEE_CHOICE -ne 0 ]; then
		echo -e "Use OPTEE (Y/n)?"
		read _option
		if [ "$_option" == "N" -o "$_option" == "n" ]; then
			sed -i '/BL32_SP ?= .*$/d' conf/local.conf
		else
			BL32_SP="optee"

			if [ "$(grep "BL32_SP" conf/local.conf)" == "" ];then
				echo "BL32_SP ?= \"$BL32_SP\"" >> conf/local.conf
			else
				sed -i "s#BL32_SP ?= \".*\"#BL32_SP ?= \"$BL32_SP\"#" conf/local.conf
			fi
		fi

	fi
	echo -e "\n=================================================="
}


######################################################
# Set tag name in baseline
#
stoe_set_tag_name() {
	if [ -d $VIEWROOT/.repo/manifests ]; then
		cd $VIEWROOT/.repo/manifests
		CARPROC_TAG=$(git describe --tags)
		if [[ "$CARPROC_TAG" = *"$(git log --oneline -1 | cut -d" " -f1)" ]]; then
		    #Current SW has no tag, Use the branch name instead
		    CARPROC_TAG="$(git rev-parse --abbrev-ref HEAD)"
		fi
		cd -

		if [ "$(grep "CARPROC_TAG" conf/local.conf)" == "" ];then
			echo "CARPROC_TAG = \"$CARPROC_TAG\"" >> conf/local.conf
		else
			sed -i "s#CARPROC_TAG = \".*\"#CARPROC_TAG = \"$CARPROC_TAG\"#" conf/local.conf
		fi
	else
		#Not in a git baseline, remove tag information
		sed -i '/CARPROC_TAG = .*$/d' conf/local.conf
	fi
}

######################################################
# Set flag to generate a tarball for flashing at end of build
#
stoe_set_flashing_package_option() {
	if [ ! -z $GENERATE_FLASHING_PACKAGE ]; then
		cd $VIEWROOT/.repo/manifests
		GENERATE_FLASHING_PACKAGE="yes"
		cd -

		if [ "$(grep "GENERATE_FLASHING_PACKAGE" conf/local.conf)" == "" ];then
			echo "GENERATE_FLASHING_PACKAGE = \"$GENERATE_FLASHING_PACKAGE\"" >> conf/local.conf
		else
			sed -i "s#GENERATE_FLASHING_PACKAGE = \".*\"#GENERATE_FLASHING_PACKAGE = \"$GENERATE_FLASHING_PACKAGE\"#" conf/local.conf
		fi
	else
		#Not in a git baseline, remove tag information
		sed -i '/GENERATE_FLASHING_PACKAGE = .*$/d' conf/local.conf
	fi
}

######################################################
# TOOGLE external source FORCE build
#
stoe_tgle_extsrc_build() {
	if [ "$(grep "^ST_EXTERNALSRC_FORCE_BUILD" $BUILDDIR/conf/local.conf)" == "" ]; then
		echo "ST_EXTERNALSRC_FORCE_BUILD ?= \"True\"" >> $BUILDDIR/conf/local.conf
		echo -e "\nExternal source compilation is now FORCED"
	else
		sed -i '/ST_EXTERNALSRC_FORCE_BUILD ?= "True"/d' $BUILDDIR/conf/local.conf
		echo -e "\nExternal source compilation is using cache"
	fi
	echo -e "===========================================\n"
}


check_ubuntu_version (){
	which lsb_release > /dev/null
	if [ $? -eq 0 ]; then
		check_ubuntu=$(echo $(lsb_release -r | cut -f2)'<'"14.04" | bc -l)
		if [ "$check_ubuntu" == 1 ]; then
			echo "You are using $(lsb_release -d|cut -f2), please install a more recent version (at least 14.04)"
			return 1
		fi
	fi
}

patch_poky_ubuntu_version (){
	which lsb_release > /dev/null
	if [ $? -eq 0 ]; then
		if [ "$(lsb_release -r | cut -f2)" = "14.04" ]; then
			echo "Ubuntu 14.04 version detected - patching meta/lib/oe/terminal.py until poky fix this (devshell issue)"
			#https://patchwork.openembedded.org/patch/129527/
			sed -i "s#\"oe-gnome-terminal-phonehome \"#bb\.utils\.which(os\.getenv(\'PATH\')\, \"oe-gnome-terminal-phonehome\") + \" \"#" $VIEWROOT/poky/meta/lib/oe/terminal.py
		fi
	fi
}

stoe_patch_poky_cve_proxy (){
	cd $ST_OE_ROOT_DIR/poky
	patch_applied=$(git log --oneline -50 | grep "cve-check-tool: correctly exported web proxies")
	if [ "$patch_applied" == "" ]; then
	    git am $ST_OE_ROOT_DIR/$ST_OE_META_LAYER_NAME/scripts/0001-cve-check-tool-correctly-exported-web-proxies.patch
	fi
	cd -
}


stoe_patch_eSDK_carproc_toogle (){
	cd $ST_OE_ROOT_DIR/$ST_OE_META_LAYER_NAME
	patch_applied=$(git log --oneline -50 | grep "Change way to manage the external sources for eSDK build")
	if [ "$patch_applied" == "" ]; then
	    git am $ST_OE_ROOT_DIR/$ST_OE_META_LAYER_NAME/scripts/0001-envsetup-patch-Change-way-to-manage-the-external-sou.patch
	    if [ "$(find $ST_OE_ROOT_DIR/$ST_OE_META_LAYER_NAME/ -name *st.bbappend | wc -l)" = "0" ]; then
		git am $ST_OE_ROOT_DIR/$ST_OE_META_LAYER_NAME/scripts/0002-envsetup-patch-part2-Change-way-to-manage-the-extern.patch
	    fi
	fi
	cd -
	if [ "$(grep "eSDK_support" $BUILDDIR/conf/bblayers.conf)" == "" ]; then
	    echo "BBFILES += \"\${OEROOT}/meta-st/meta-st-carproc/eSDK_support/*.bbappend\"" >> $BUILDDIR/conf/bblayers.conf
	    echo "DISABLE_EXTERNAL_SRC = \"1\"" >> $BUILDDIR/conf/local.conf
	    echo -e "\neSDK Compilation patch is now enabled"
	else
	    sed -i '/DISABLE_EXTERNAL_SRC = "1"/d' $BUILDDIR/conf/local.conf
	    sed -i '/.*eSDK_support.*/d' $BUILDDIR/conf/bblayers.conf
	    echo -e "\neSDK Compilation patch is now disabled"
	fi
}


######################################################
# Patch for updating default OE terminal to avoid issue
# when trying to run bitbake -c menuconfig/devshell/devpyshell
# under Ubuntu 14.04 LTS.
# This is an already known issue:
#   https://patchwork.openembedded.org/patch/129527/
#
_temp_patch__gnometerminal() {
    if [ -f /etc/lsb-release ]; then
        if ! [ -z "$(grep 'DISTRIB_RELEASE=14.04' /etc/lsb-release)" ]; then
            echo "[INFO] Ubuntu 14.04 release detected"
            echo "[INFO] Apply temporary patch for menuconfig/devshell/devpyshell issue"
            echo "[INFO] Set OE default terminal to 'xterm' in site.conf file"
            cat >> conf/site.conf <<EOF

# =========================================================================
# Update OE default terminal (temporary patch for Ubuntu 14.04 LTS only)
# =========================================================================
OE_TERMINAL = "xterm"
EOF
        fi
    else
        echo "[WARNING] Not able to detect Linux Distrib Release: missing /etc/lsb-release file."
        echo "[WARNING] Skip checking for OE default terminal update..."
    fi
}


######################################################
# Since this script is sourced, be careful not to pollute
# caller's environment with temp variables.
#
_stoe_unset() {
    unset BUILD_DIR
    unset DISTRO
    unset MACHINE
    unset _DL_CACHEPREFIX
    unset _SSTATE_CACHEPREFIX
    unset _NCPU
    unset _SITECONFSAMPLE_PATH
    unset _INIT
    unset _QUIET
    unset _TEMPLATECONF
    # Clean env from unwanted functions
    unset -f choice
    unset -f _choice_shell
    unset -f get_templateconf
    unset -f conf_bblayerconf
    unset -f conf_localconf
    unset -f conf_siteconf
    unset -f _default_config_get
    unset -f _default_config_set
    unset -f _temp_patch__gnometerminal
}

######################################################
# Since this script is sourced, be careful not to pollute
# caller's environment with temp variables.
#
_stoe_carproc_unset() {
    unset _META_LAYER_ROOT
    unset _META_LAYER_NAME
    unset VIEWROOT
    unset _USE_STMCONF
    unset KEEP_ARGS
    unset KEEP_PARAM
    unset _HASBUILDDIR
    unset DISTRO
    unset _GNSS_CHOICE
    unset _TUNER_CHOICE
    unset _NFS_CHOICE
    unset _INITRAMFS_CHOICE
    unset _WESTON_SHELL_CHOICE
    unset _OPTEE_CHOICE
    # Clean env from unwanted functions
    unset -f _stoe_carproc_add_checkpatch_hooks
}


######################################################
# Main
# --
#

#----------------------------------------------
# Make sure script has been sourced
#
if [ "$0" = "$BASH_SOURCE" ]; then
    echo "###################################"
    echo "ERROR: YOU MUST SOURCE the script"
    echo "###################################"
    exit 1
fi

_GNSS_CHOICE=0
_TUNER_CHOICE=0
_NFS_CHOICE=0
_INITRAMFS_CHOICE=0
_WESTON_SHELL_CHOICE=0
_OPTEE_CHOICE=0
READTIMEOUT=60

unset BUILD_DIR
unset _FORMAT_PATTERN
unset BUILDDIR


#----------------------------------------------
# Options parsing
#
while test $# != 0
do
    case "$1" in
        --reset)
            _FORCE_RECONF=1
            _INIT=0
            ;;
        --help)
            stoe_help
            _stoe_utilities
            stoe_carproc_help
            _stoe_carproc_utilities
            return 1
            ;;
        --quiet)
            _QUIET=1
            ;;
        --gnss)
            _GNSS_CHOICE=1
            ;;
        --tuner)
            _TUNER_CHOICE=1
            ;;
        --nfs)
            _NFS_CHOICE=1
            ;;
        --initramfs)
            _INITRAMFS_CHOICE=1
            ;;
        --weston-shell)
            _WESTON_SHELL_CHOICE=1
            ;;
        --optee)
            _OPTEE_CHOICE=1
            ;;
        --distro)
            unset _DISTRO
            unset _MACHINE
            ;;
        -*)
            echo "Wrong parameter: $1"
            return 1
            ;;
        *)
            #change buildir directory
            if ! [[ $1 =~ ^build.* ]]; then
                echo "ERROR: '$1' : please provide BUILD_DIR with 'build' prefix."
                return 1
            fi
            #we want BUILD_DIR without any '/' at the end
            BUILD_DIR=$(echo $1 | sed 's|[/]*$||')
            ;;
    esac
    shift
done

# Define default settings for STM envsetup call
DISTRO=$_DISTRO
MACHINE=$_MACHINE

check_ubuntu_version


#----------------------------------------------
# Init BUILD_DIR variable
#
if [ -z "${BUILD_DIR}" ] && ! [ -z "$DISTRO" ] && ! [ -z "$MACHINE" ]; then
    # In case DISTRO and MACHINE are provided use them to init BUILD_DIR
    BUILD_DIR="build-${DISTRO//-}-$MACHINE"
fi

if [ -z "${BUILD_DIR}" ]; then
    # Get existing BUILD_DIR list from baseline
    LISTDIR=$(mktemp)
    for l in $(find ${VIEWROOT} -maxdepth 1 -wholename "*/build*"); do
        test -f ${l}/conf/local.conf && echo ${l#*${VIEWROOT}/} >> ${LISTDIR}
    done
    # Select any existing BUILD_DIR from list
    if  [ -s ${LISTDIR} ]; then
        choice BUILD_DIR "$(_choice_formated_dirs "$(cat ${LISTDIR} | sort)")" $(_default_config_get "$(cat ${LISTDIR} | sort)")
        [ -z "${BUILD_DIR}" ] && { echo "Selection escaped: exiting now..."; _stoe_unset; return 1; }
    fi
    # Reset BUILD_DIR in case for new config choice
    test "${BUILD_DIR}" == "NEW" && BUILD_DIR=""
else
    # Check if configuration files exist to force or not INIT
    test -f ${VIEWROOT}/${BUILD_DIR}/conf/bblayers.conf || _INIT=1
    test -f ${VIEWROOT}/${BUILD_DIR}/conf/local.conf || _INIT=1
fi

if [[ $_INIT -eq 1 ]] || [[ -z "${BUILD_DIR}" ]]; then
    # There is no available config in baseline: force init from scratch
    _INIT=1

    # Set DISTRO
    if [ -z "$DISTRO" ]; then
        DISTRO_CHOICES=$(_choice_formated_configs distro)
        test "$?" == "1" && { echo "$DISTRO_CHOICES"; _stoe_unset; return 1; }
        # Add nodistro option
        DISTRO_CHOICES=`echo -e "$DISTRO_CHOICES\nnodistro${_FORMAT_PATTERN}*** DEFAULT OPENEMBEDDED SETTING : DISTRO is not defined ***"`
        choice DISTRO "$DISTRO_CHOICES"
        [ -z "$DISTRO" ] && { echo "Selection escaped: exiting now..."; _stoe_unset; return 1; }
    fi

    # Set MACHINE
    if [ -z "$MACHINE" ]; then
        MACHINE_CHOICES=$(_choice_formated_configs machine)
        test "$?" == "1" && { echo "$MACHINE_CHOICES"; _stoe_unset; return 1; }
        choice MACHINE "$MACHINE_CHOICES"
        [ -z "$MACHINE" ] && { echo "Selection escaped: exiting now..."; _stoe_unset; return 1; }
    fi

    # Init BUILD_DIR if not yet set
    test -z "${BUILD_DIR}" && BUILD_DIR="build-${DISTRO//-}-$MACHINE"

    # Check if BUILD_DIR already exists to use previous config (i.e. set _INIT to 0)
    test -f ${VIEWROOT}/${BUILD_DIR}/conf/bblayers.conf && _INIT=0
    test -f ${VIEWROOT}/${BUILD_DIR}/conf/local.conf && _INIT=0

else
    # Get DISTRO and MACHINE from configuration file
    DISTRO_INIT=$(_stoe_config_read ${VIEWROOT}/${BUILD_DIR} DISTRO)
    MACHINE_INIT=$(_stoe_config_read ${VIEWROOT}/${BUILD_DIR} MACHINE)

    [[ ${DISTRO_INIT} =~ \< ]] && DISTRO_INIT="$_DISTRO"

    # Set DISTRO
    if [ -z "$DISTRO" ]; then
        DISTRO=${DISTRO_INIT}
    elif [ "$DISTRO" != "${DISTRO_INIT}" ]; then
        # User has defined a wrong DISTRO for current BUILD_DIR configuration
        echo "ERROR: DISTRO $DISTRO does not match "${DISTRO_INIT}" already set in ${BUILD_DIR}"
        _stoe_unset
        return 1
    fi

    # Set MACHINE
    if [ -z "$MACHINE" ]; then
        MACHINE=${MACHINE_INIT}
    elif [ "$MACHINE" != "${MACHINE_INIT}" ]; then
        # User has defined a wrong MACHINE for current BUILD_DIR configuration
        echo "ERROR: MACHINE $MACHINE does not match "${MACHINE_INIT}" already set in ${BUILD_DIR}"
        _stoe_unset
        return 1
    fi
fi


#----------------------------------------------
# Init timestamp for default builddir choice
#
_default_config_set


#----------------------------------------------
# Init baseline for full INIT if required
#
if [[ $_FORCE_RECONF -eq 1 ]] && [[ $_INIT -eq 0 ]]; then
    echo ""
    echo "[Removing current config from ${VIEWROOT}/${BUILD_DIR}/conf]"
    rm -fv ${VIEWROOT}/${BUILD_DIR}/conf/*.conf ${VIEWROOT}/${BUILD_DIR}/conf/*.txt
    echo ""
    # Force init to generate configuration files
    _INIT=1
fi


#----------------------------------------------
# Standard Openembedded init
#
echo -en "[source $_BUILDSYSTEM/oe-init-build-env]"
[[ $_INIT -eq 1 ]] && echo "[from nothing]"
[[ $_INIT -eq 0 ]] && echo "[with previous config]"
get_templateconf
test "$?" == "1" && { _stoe_unset; return 1; }
TEMPLATECONF=${_TEMPLATECONF} source ${VIEWROOT}/$_BUILDSYSTEM/oe-init-build-env ${BUILD_DIR} > /dev/null
test "$?" == "1" && { _stoe_unset; return 1; }

#----------------------------------------------
# Apply specific ST configurations
#
if [[ $_INIT -eq 1 ]]; then
    # Configure site.conf with specific settings
    conf_siteconf
    # Configure local.conf with specific settings
    conf_localconf
    # Update bblayer.conf with specific machine bsp layer path
    conf_bblayerconf
fi


#----------------------------------------------
# Carproc Configuration
#
#Configure GNSS-TESEO options
stoe_set_gnss_options
#Configure Tuner options
stoe_set_tuner_options

#Configure NFS options
stoe_set_nfs_options

#Configure initramfs options
stoe_set_initramfs_options

#Configure WESTON SHELL options
stoe_set_weston_shell_options

#Configure OPTEE option
stoe_set_optee_options

#Configure Tag name in rootfs
stoe_set_tag_name

#Patch poky
patch_poky_ubuntu_version

#Configure Flashing package option
stoe_set_flashing_package_option

#----------------------------------------------
# Display current configs
#
stoe_carproc_config_summary $BUILD_DIR

#----------------------------------------------
# Display available images
#
stoe_list_images $VIEWROOT/poky/meta
stoe_list_images $VIEWROOT/$_META_LAYER_NAME

#----------------------------------------------
# Clear user's environment from temporary variables
#
_stoe_carproc_unset

#----------------------------------------------
# Clear user's environment from temporary variables
#
_stoe_unset
