./tigc dogfood/kernel/kernel.tc -o dogfood/kernel/kernel.c --freestanding
gcc -ffreestanding -nostdlib -fno-pie -fno-stack-protector -fno-asynchronous-unwind-tables -c dogfood/kernel/kernel.c -o dogfood/kernel/kernel.old -T dogfood/kernel/linker.ld dogfood/kernel/kernel.o -o dogfood/kernel/kernel.exe
objcopy -O binary dogfood/kernel/kernel.exe dogfood/kernel/kernel.bin
