#!/bin/bash

usage() {
	echo -e "Usage:"
	echo -e "\t$0"
	echo -e "\nnfs_folder value is $(echo $(dirname $BUILDDIR)/nfs_roots)"
}

which sudo-tar > /dev/null
if [ $? -eq 0 ]; then
	sudotar="sudo-tar"
	sudorm="sudo-rm"
else
	sudotar="sudo tar"
	sudorm="sudo rm"
fi

if [ $# -eq 1 ]; then
	if [ "$1" == "-h" ]; then
		usage
		exit
	fi
fi
if [ "$BUILDDIR" == "" ]; then
	"Please source your environment before ..."
	exit
fi

MACHINE=$(grep "MACHINE =" $BUILDDIR/conf/site.conf | cut -d'"' -f2)
IMAGE_COUNT=$(find $BUILDDIR/tmp/deploy/images/$MACHINE/ -name "config-*" | wc -w)


if [ "$MACHINE" == "" ]; then
	echo "Please compile the baseline first ..."
	exit
fi


if [ $IMAGE_COUNT -gt 1 ]; then
	echo -e "You have "$IMAGE_COUNT" images available, please choose the one you want to export:\n"
	cd $BUILDDIR/tmp/deploy/images/$MACHINE/
	ls -T2 -c1 config-*.txt
	cd -
	echo "? "
	read configuration
	if [ ! -e $BUILDDIR/tmp/deploy/images/$MACHINE/$configuration ]; then
		echo -e "Invalid configuration filename"
		exit
	fi
else
	cd $BUILDDIR/tmp/deploy/images/$MACHINE/
	configuration=$(ls config-*)
	cd -
fi
IMAGE=$(grep "core-image" $BUILDDIR/tmp/deploy/images/$MACHINE/$configuration | sed -e "s#.*rootfs.\(.*\)#\1#" | sed -e "s#=\s##" | rev | cut -d"." -f 2- | rev)".tar.gz"

if [ "$IMAGE" == "" -o "$IMAGE" == ".tar.gz" ]; then
	echo "rootfs image $IMAGE can't be found ..."
	nfs_option=$(grep "^ROOTFS_DEVICE"  $BUILDDIR/conf/local.conf| rev | cut -d'"' -f2 | rev)
	if [ "$nfs_option" == "" ]; then
		echo "It seems NFS was not enabled in you baseline ( execute \"source envsetup.sh --nfs\" and build image again"
	else
		echo "Even if NFS was enabled, $IMAGE can't be found"
	fi
	
	exit
fi

NFS_FOLDER="$(echo $(dirname $BUILDDIR)/nfs_rootfs)"

echo "Sources images: $(basename $BUILDDIR) -- Source again your env if incorrect"
echo "Installing rootfs in $NFS_FOLDER ..."

if [ "$NFS_FOLDER" == "/" -o "$NFS_FOLDER" == "" ]; then
	echo "Invalid nfs folder: $NFS_FOLDER"
	usage
	exit
fi

$sudorm -rf $NFS_FOLDER
mkdir -p $NFS_FOLDER
$sudotar xzf $BUILDDIR/tmp/deploy/images/$MACHINE/$IMAGE -C $NFS_FOLDER --same-owner -p --overwrite
sudo exportfs -o rw,no_root_squash,async,no_subtree_check *:$NFS_FOLDER

ip_nr=$(hostname -I | wc -w)
if [ $ip_nr -gt 1 ]; then
	echo "Multiple interaces are available, please choose the IP you'll use to share your rootfs:"
	echo $(hostname -I | tr " " "\n")
	echo "?"
	read ip_addr
	if [ "$(hostname -I | cut -d" " -f1 | grep $ip_addr)" == "" ]; then
		echo "Invalid IP"
		exit
	fi
else
	ip_addr=$(hostname -I | cut -d" " -f1 )
fi
UBOOT_ENV_CONFIG_NAME=$(grep ^UBOOT_ENV $BUILDDIR/tmp/deploy/images/$MACHINE/$configuration | rev | cut -d" " -f1 | rev)
rootfs_path="$(dirname $BUILDDIR)/nfs_rootfs"
sed -i "s#root=.*\${commonargs}#root=/dev/nfs nfsroot=$ip_addr:$rootfs_path ip=dhcp \${commonargs}#g" $BUILDDIR/tmp/deploy/images/$MACHINE/$UBOOT_ENV_CONFIG_NAME

