#!/bin/bash
case $1 in
    suspend)
        systemctl stop rvc
    ;;
    resume)
        systemctl start rvc
    ;;
    *)
    ;;
esac

