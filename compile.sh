echo "Compiling.."
CFLAGS="-ffreestanding -g -m32"
# Compile all assembly files

# Bootloader goes separate
nasm -felf32 "Source/Bootload/init.s" -f bin -o "Temp/init.bin"
# Compile the kernel entry as an object file (to link for C++)
nasm -f elf32 "Source/Bootload/entry.s" -o "Temp/object/entry.o"

# Compile all library/source files, make them into object files for linking.
find Source -type f -name "*.cpp" | while IFS= read -r file; do
    name=$(basename "$file" .cpp)
    i386-elf-gcc -I ./Source/Lib $CFLAGS -c "$file" -o "Temp/object/$name.o"
done

# Find all object files
OBJS=""
for f in $(find Temp/object -type f -name "*.o" ! -name "entry.o"); do
    OBJS="$OBJS $f"
done

echo $OBJS
# Linking stage (Make the input files an array of names) and use _start as the start symbol
# Not-so fun quirk: entry.o MUST be first, so filter it out in the object list and manually link it first.
i386-elf-ld -T kernel.ld -e _start -o "Temp/osystem.bin" -Ttext 0x7e00 "Temp/object/entry.o" $OBJS --oformat binary

# Append the MBR, compiled system and leading zeroes
# "main.img" is the final product of the build process.
dd if=Temp/init.bin of=main.img bs=512 count=63
dd if=Temp/osystem.bin of=main.img bs=512 seek=1

# Append zeroes to prevent read errors from BIOS
dd if=/dev/zero bs=1 count=200000 >> main.img