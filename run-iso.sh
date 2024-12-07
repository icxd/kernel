#!/bin/bash

qemu-system-i386 -drive file=bin/kernel.iso,format=raw -no-reboot -no-shutdown -serial stdio -d int -D qemu.log