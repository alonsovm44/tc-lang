# 1. Generate C
./tigc tests/free/kernel.tc -o tests/free/kernel.c --freestanding

# 2. Compile (disable PIE and stack protection)
gcc -ffreestanding -nostdlib -fno-pie -fno-stack-protector -c tests/free/kernel.c -o tests/free/kernel.o

# 3. Link to PE executable (Windows format)
ld -T tests/free/linker.ld tests/free/kernel.o -o tests/free/kernel.exe

# 4. Convert to flat binary
objcopy -O binary tests/free/kernel.exe tests/free/kernel.bin

# 5. Run in QEMU
qemu-system-x86_64 -kernel tests/free/kernel.bin