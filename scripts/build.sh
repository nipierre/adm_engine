set -e # Exit immediately if a command exits with a non-zero status

echo "=== Build adm-engine ==="

mkdir build && cd build
cmake ..
make
sudo make install
cd ../
