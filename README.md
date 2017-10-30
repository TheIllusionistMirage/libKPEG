# libKPEG

A simple, easy-to-use JPEG library, written in C++11.

This is a work in progress. Expect loads of bugs. There are no dependencies
save SFML-2.4.x which will be removed in the future (I'm using SFML just to
test the output of the decode process, not to decode the image itself!).

As of now, the decoded data is available in uncompressed RGB format in the `.ppm` format
(the output for `filename.jpg` is `filename.ppm`) and is also displayed in a SFML GUI window.

My goal of writing this library is to gain a better technical knowledge about
JPEG image compression and also a self learning programming exercise. I have
intentionally used naive programming constructs for better clarity, sacrificing
speed.


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
