#!/bin/bash

images=`find test/ -type f | egrep "\.gif$"`
for image in $images; do 
	bmp_image=`echo "$image" | sed -r "s%\.gif$%.bmp%g"`
	./gif2bmp -i "$image" -o "$bmp_image"
done
