#!/bin/sh
VERSION=6+1

if [ ! -e hex ]; then
	mkdir hex
fi

make LIGHTPACK_HW=4
mv Lightpack_hw4.hex hex/Lightpack_hw4_${VERSION}.hex
make clean LIGHTPACK_HW=4
make LIGHTPACK_HW=5
mv Lightpack_hw5.hex hex/Lightpack_hw5_${VERSION}.hex
make clean LIGHTPACK_HW=5
make LIGHTPACK_HW=6
mv Lightpack_hw6.hex hex/Lightpack_hw6_${VERSION}.hex
make clean LIGHTPACK_HW=6
make LIGHTPACK_HW=7
mv Lightpack_hw7.hex hex/Lightpack_hw7_${VERSION}.hex
make clean LIGHTPACK_HW=7
