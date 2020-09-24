set -e # Exit immediately if a command exits with a non-zero status

echo "=== Build libear ==="
git clone --recursive https://github.com/ebu/libear.git && cd libear/
mkdir build && cd build
cmake ..
make
sudo make install
cd ../../

echo "=== Build libadm ==="
git clone https://github.com/IRT-Open-Source/libadm.git && cd libadm/
git checkout 0.11.0
mkdir build && cd build
cmake ..
make
sudo make install
cd ../../

echo "=== Build libbw64 ==="
git clone https://github.com/IRT-Open-Source/libbw64.git && cd libbw64/
git checkout 0.10.0
mkdir build && cd build
cmake ..
make
sudo make install
cd ../../
