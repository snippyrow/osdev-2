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
# dd if=Temp/init.bin of=main.img bs=512 count=63
# dd if=Temp/osystem.bin of=main.img bs=512 seek=1

# Attach font to sector 100. In the future, create a filesystem, make file I/O and put it into a bin.
# dd if=Assets/font.bin of=main.img bs=512 seek=100

# Append zeroes to prevent read errors from BIOS
# dd if=/dev/zero bs=1 count=200000 >> main.img

# Add compiler to PATH: export PATH="/usr/local/i386elfgcc/bin:$PATH"


## NEW FAT32 FS
# Create the FAT32 image placeholder
dd if=/dev/zero of=myfat32.img bs=1M count=256 status=progress

# Format to FAT32
sudo mkfs.fat -F 32 -n "MYDISK" myfat32.img
# Replace the first 512 bytes of the newly generated image with the compiled MBR
# MBR already contains all the necessary FAT32 stuff.
dd if=Temp/init.bin of=myfat32.img bs=1 count=512 conv=notrunc
# Append the second-stage kernel *outside* of the FAT32 FS. Lightweight option.
cat Temp/osystem.bin >> myfat32.img

# Append some zeroes at the end to prevent the BIOSfrom throwing errors.
dd if=/dev/zero bs=1 count=200000 >> myfat32.img
# Truncate to a multiple of 512 (within one sector)
truncate -s %512 myfat32.img

# mount filesystem if not done: sudo mount -o loop myfat32.img /mnt/osdev-fat32
#sudo cp Temp/osystem.bin /mnt/osdev-fat32

# To run it as qemu, use qemu-system-x86_64 -enable-kvm -drive file=myfat32.img

# To convert the raw image to a *.vdi, use VBoxManage convertfromraw myfat32.img myfat32.vdi --format VDI