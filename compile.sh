echo "Compiling.."
CFLAGS="-ffreestanding -g -m32"
# Compile all assembly files

# Bootloader goes separate
nasm -felf32 "Source/Bootload/init.s" -f bin -o "Temp/init.bin"

# Compile the kernel entry as an object file (to link for C++)
nasm -f elf32 "Source/Bootload/entry.s" -o "Temp/object/entry.o"

# Compilation stage for all kernel c/c++ files
for file in Source/Kernel/*; do
    echo "Building file $file"
    filename=$(basename "$file")
    savename="${filename%.*}.o"
    #i386-elf-gcc $CFLAGS -c "$file" -o "Temp/object/$savename"
done

# All required files to be compiled
i386-elf-gcc $CFLAGS -I ./Source/Lib -c "Source/Kernel/main.cpp" -o "Temp/object/main.o"
i386-elf-gcc $CFLAGS -I ./Source/Lib -c "Source/Lib/vga.cpp" -o "Temp/object/vga.o"

# Linking stage (Make the input files an array of names) and use _start as the start symbol
i386-elf-ld -e _start -o "Temp/osystem.bin" -Ttext 0x7e00 "Temp/object/entry.o" "Temp/object/main.o" "Temp/object/vga.o" --oformat binary

# Append the MBR, compiled system and leading zeroes
# "main.img" is the final product of the build process.
dd if=Temp/init.bin of=main.img bs=512 count=63
dd if=Temp/osystem.bin of=main.img bs=512 seek=1

# Append zeroes to prevent read errors from BIOS
dd if=/dev/zero bs=1 count=200000 >> main.img