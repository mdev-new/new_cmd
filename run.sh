clear
if [ "$1" = "release" ]; then
flags="-Os -flto -s -Wl,--gc-sections"
else
flags="-Og -ggdb"
fi
shutup="-fpermissive -Wno-narrowing -Wno-unused-result -Wno-format"
g++ src/*.cpp src/*.cc $flags $shutup -std=gnu++17
./a.out testing.bat
