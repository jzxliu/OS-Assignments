#NOTE: You will need to change this path if you run this on your own machine.
# You can do so without modifying this makefile by running:
# 	IMAGE_PATH=<your path> make tests
IMAGE_PATH ?= /u/csc369h/fall/pub/tutorials/t8/diskimages

readimage: readimage.c ext2.h
	gcc -Werror -Wall -g -o readimage readimage.c

# If the tests are successful, you will see no output
# If one tests fails the remaining tests will not run
tests: readimage
	./readimage ${IMAGE_PATH}/emptydisk.img | diff -w emptydisk.expected -
	./readimage ${IMAGE_PATH}/onefile.img   | diff -w onefile.expected   -
	./readimage ${IMAGE_PATH}/largefile.img | diff -w largefile.expected -
	./readimage ${IMAGE_PATH}/twolevel.img  | diff -w twolevel.expected  -

clean:
	rm -f readimage
