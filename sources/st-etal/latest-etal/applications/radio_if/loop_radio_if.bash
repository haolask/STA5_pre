#!/bin/bash

cleanup(){
	rm -f /tmp/tempfile
	return $?
}

ctrl_c(){
	echo -en "\n*** Exiting radio_if ***\n"
	cleanup
	exit $?
}

trap ctrl_c SIGINT
while true; do radio_if; done