# libKPEG

A simple, easy-to-use JPEG library, written in C++11.

This is a work in progress. Expect loads of bugs. There are no dependencies
to build this project.

Only the decoder has been implemented till now. `libKPEG` decodes a JFIF image file and 
saves the uncompressed image to a PPM image in the same directory and while encoding,
takes as input a PPM image file and converts it to a corresponding JFIF file.

My goal of writing this library is to gain a better technical knowledge about
JPEG image compression and also a self learning programming exercise. I have
intentionally used naive programming constructs for better clarity, sacrificing
speed.

**NOTE:** _Only 8-bit Sequential Baseline DCT, grayscale/RGB, no subsampling (4:4:4) is supported as of now._

# Features Supported

### Encoder

* None, no work done here yet

### Decoder

* 8-bit Sequential Baseline, DCT, grayscale/RGB, no subsampling (4:4:4)


Most images should work without any problems. And if some don't, they will eventually.
You can check out the `misc/screenshots` directory for some screenshots I took with
test images.


# Reference materials

1. The official JPEG standard, ITU-T.81 ([pdf](https://www.w3.org/Graphics/JPEG/itu-t81.pdf))
2. The official JFIF standard ([pdf](https://www.w3.org/Graphics/JPEG/jfif3.pdf) )
3. Calvin Hass' excellent articles ([blog posts](http://www.impulseadventure.com/photo/))
4. Cristi Cuturicu's JPEG Guide ([txt](http://www.opennet.ru/docs/formats/jpeg.txt))


# Building

```
$ git clone https://github.com/TheIllusionistMirage/libKPEG
$ git mkdir build && cd build
$ cmake ..
$ make
```

# Author
[Koushtav Chakrabarty](https://github.com/TheIllusionistMirage) ( aka TheIllusionistMirage )

# License
Not decided yet
