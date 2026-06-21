
# OS dev pipeline in Tig

1) Write kernel in Tig
```
// kernel.tc
freestanding fn void kernel_main: {
    // Your kernel code
}
```

2) Generate C code
```
tigc kernel.tc -o kernel.c --freestanding
```

3) Compile to object file
```
gcc -ffreestanding -nostdlib -c kernel.c -o kernel.o
```

4) Link with bootloader
```
ld -T linker.ld kernel.o -o kernel.bin
```

5) Make ISO
```
grub-mkrescue -o kernel.iso iso/
```

6) Run in QEMU
```
qemu-system-x86_64 -cdrom kernel.iso
```
