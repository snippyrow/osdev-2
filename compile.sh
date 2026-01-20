echo "Compiling.."
# Compile all assembly files
for file in Source/Bootload/*; do
    echo "Building file $file"
    filename=$(basename "$file")
    savename="${filename%.*}.bin"
    nasm -felf32 "$file" -f bin -o "Temp/$savename"
done