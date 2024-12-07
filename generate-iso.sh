#!/usr/bin/env sh

grub-file --is-x86-multiboot kernel.bin
if [ $? -ne 0 ]; then
    echo "Error: kernel.bin is not a multiboot kernel"
    exit 1
fi

mkdir -p isodir/boot/grub
cp kernel.bin isodir/boot/kernel.bin

echo 'set timeout=0
set default=0
menuentry "kernel" {
  multiboot /boot/kernel.bin
}' > isodir/boot/grub/grub.cfg

grub-mkrescue -o kernel.iso isodir
rm -rf isodir