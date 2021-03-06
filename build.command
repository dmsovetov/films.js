cd "$(dirname $0)"

curl -L https://github.com/mongodb/mongo-c-driver/releases/download/1.0.2/mongo-c-driver-1.0.2.tar.gz | tar zx
cd mongo-c-driver-1.0.2

./configure --prefix=$(pwd)/../src/libmongo --enable-sasl=no --enable-tests=no --enable-examples=no --enable-html-docs=no --enable-man-pages=no --enable-static=yes --enable-shared=no
make -j8
make install

cd ../
rm -rf mongo-c-driver-1.0.2

node-gyp rebuild