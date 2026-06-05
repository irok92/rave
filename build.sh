clear
inotifywait -q -m -r -e close_write,moved_to ./src |
while read -r directory event filename; do
    clear
    echo Compiling..
    clang++ -Isrc/cli -Isrc/lang  src/cli/*.cpp src/lang/*.cpp -o rave-cli
    echo Done
done
