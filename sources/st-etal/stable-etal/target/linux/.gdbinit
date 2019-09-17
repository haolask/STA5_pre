# This line configures the connection between GDB running on the host
# and the gdbserver running on the target board
# Ethernet connection is supported
# The last number after the ':' is the TCP port number, it is an
# arbitrary TCP port number but it must match the port number specified
# on the gdbserver command line on the target
target extended-remote 10.50.186.100:10000

# This is the normal case, that is APP_ETAL_TEST build

# Tell GDB to transfer the executable to the target.
# The format of the command is:
# remote put <local file name> <remote file name>
remote put /home/belardi/ETAL_remote/target/linux/o/etaltest etaltest
#remote put /home/belardi/ETAL_remote/target/linux/TDA7707_OM_v3.8.0.boot TDA7707_OM_v3.8.0.boot
#remote get boot_cmost_STAR-T.h /home/belardi/ETAL/lib_boot/include/boot_cmost_STAR-T.h 
# Tell GDB what to debug on the target.
# The format of the command is:
# set remote exec-file <remote file name>
set remote exec-file etaltest
# Tell GDB where to find the sources.
# The format of the command is:
# file <remote file name>
file /home/belardi/ETAL_remote/target/linux/o/etaltest

# Additionally, the line below should be uncommented only for COMM_DRIVER_EXTERNAL builds
#remote put <insert your path here>/ETAL/tools/MDR_protocol/MDR_protocol_layer MDR_protocol_layer

# For APP_OSALCORE_TESTS uncomment the following lines instead

#remote put <insert your path here>/ETAL/target/linux/o/osaltest osaltest
#set remote exec-file osaltest
#file <insert your path here>/ETAL/target/linux/o/osaltest

