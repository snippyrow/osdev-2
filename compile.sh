echo "Compiling.."
# Compile all assembly files

# Bootloader goes separate
# nasm -felf32 "Source/Bootload/init.s" -f bin -o "Temp/init.bin"
for file in Source/Bootload/*; do
    echo "Building file $file"
    filename=$(basename "$file")
    savename="${filename%.*}.bin"
    nasm -felf32 "$file" -f bin -o "Temp/$savename"
done