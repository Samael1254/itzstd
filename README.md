### Image to Tiled ZSTD converter (ITZSTD)

ITZSTD is a tool to convert a PNG or JPEG image into a set of tiles compressed in the [ZSTD](https://github.com/facebook/zstd) format.

Splitting an image in such a format can be useful to proccess large amounts of data stored in an image at runtime since decompressing ZSTD is much faster than PNG, and the multiple tiles allow for parallel processing. In some cases (low-entropy images) the total file size might even be smaller than the PNG image.

So far, only greyscale images are supported (1 channel).

#### 📦 Installation

To compile the binary, just clone the repository and make the project:
```
git clone https://github.com/Samael1254/itzstd
cd itzstd
make
```

#### ⚙️ Usage

After adding the binary to your PATH, you can execute it like so:
```
itzstd [options] inputFile [outputDir]
```
InputFile is the path of the image you want to convert.

OutputDir the directory where you want to save the tiles (the tile files will have outputDir as a prefix).

If outputDir is not provided, the tiles will be created in the current directory.

The command accepts the following options:

- `-h`: command help.

- `-n subdivisions`: the number of times the image will be subdivided in 4. This produces 2^(2n) tiles.

- `-b bitsPerPixel`: the number bits per pixel you want in the compressed data.

#### ⚙️ Acknowledgements

This program uses [stb_image.h](https://github.com/nothings/stb/blob/master/stb_image.h) to load images.

#### ⏩ Future improvements

- Allow user to change the compression level
- Allow cropped tiles at the end
