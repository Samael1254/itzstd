#define STB_IMAGE_IMPLEMENTATION

#include "itzstd.h"
#include "stb_image.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <zstd.h>

void help()
{
	fprintf(stderr, "Usage: itzstd [OPTION]... INFILE [OUTDIR]\n");
	fprintf(stderr, "Converts a jpeg or png image into zstd compressed tiles\n\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -n subdivs		number of subdivisions (default: 5). Produces 2^(2n) tiles\n");
	fprintf(stderr, "  -b bpp			bits per pixel of the resulting data\n");
	fprintf(stderr, "  -m margin		add a margin to each tile, containing pixels from neighbors\n");
	fprintf(stderr, "  -c compression	the zstd compression level\n");
	fprintf(stderr, "  -h 				display this help\n");
}

void readOpts(int argc, char **argv, opts_t *opts)
{
	int opt;
	do
	{
		opt = getopt(argc, argv, ":n:b:m:c:h");
		switch (opt)
		{
		case -1:
			break;
		case 'n':
			opts->subdivs = atoi(optarg);
			break;
		case 'b':
			opts->bpp = atoi(optarg);
			break;
		case 'm':
			opts->margin = atoi(optarg);
			break;
		case 'c':
			opts->compression = atoi(optarg);
			break;
		case 'h':
			help();
			exit(EXIT_SUCCESS);
		case ':':
			fprintf(stderr, "error: option requires argument: %s\n", argv[optind - 1]);
			exit(EXIT_FAILURE);
		default:
			fprintf(stderr, "error: unknown option: %s\n", argv[optind - 1]);
			exit(EXIT_FAILURE);
		}
	} while (opt != -1);
}

void readArgs(int argc, char **argv, char *infile, char *outdir)
{
	if (argc <= optind)
	{
		fprintf(stderr, "error: missing input file\n");
		exit(EXIT_FAILURE);
	}
	if (argc > optind + 2)
	{
		fprintf(stderr, "error: too many arguments\n");
		exit(EXIT_FAILURE);
	}

	if (strlen(argv[optind]) > 255)
	{
		fprintf(stderr, "error: input file path too long\n");
		exit(EXIT_FAILURE);
	}
	strcpy(infile, argv[optind]);

	if (argc <= optind + 1)
		return;
	if (strlen(argv[optind + 1]) > 255)
	{
		fprintf(stderr, "error: output directory path too long\n");
		exit(EXIT_FAILURE);
	}
	strcpy(outdir, argv[optind + 1]);
}

const uint8_t *loadImage(imageData_t *imgData, const char *infile)
{
	printf("Loading image %s\n", infile);
	const uint8_t *img = stbi_load(infile, &imgData->width, &imgData->height, &imgData->channels, 1);
	if (!img)
	{
		fprintf(stderr, "error: could not load image: %s\n", infile);
		exit(EXIT_FAILURE);
	}
	printf("Loaded image %s: %dx%d (%d channels)\n", infile, imgData->width, imgData->height, imgData->channels);

	return img;
}

tilesData_t computeTilesData(imageData_t imgData, uint8_t subdivs)
{
	tilesData_t tilesData;
	tilesData.nbPerSide = 1 << subdivs;
	tilesData.width = (imgData.width + tilesData.nbPerSide - 1) / tilesData.nbPerSide;
	tilesData.height = (imgData.height + tilesData.nbPerSide - 1) / tilesData.nbPerSide;

	printf("Subdivisions: %d (%dx%d for %d total tiles)\n", subdivs, tilesData.nbPerSide, tilesData.nbPerSide,
	       tilesData.nbPerSide * tilesData.nbPerSide);
	printf("Target tile size: %dx%d\n", tilesData.width, tilesData.height);

	return tilesData;
}

byteBuf_t packPixelData(const uint8_t *img, uint8_t bpp, uint8_t margin, tilesData_t tilesData, imageData_t imgData,
                        int tx, int ty)

{
	uint32_t xMax = (tilesData.width + margin + (tx > 0 && tx < tilesData.nbPerSide - 1) * margin);
	uint32_t yMax = (tilesData.height + margin + (ty > 0 && ty < tilesData.nbPerSide - 1) * margin);

	size_t    pixelCount = (size_t)xMax * yMax;
	byteBuf_t packed;
	packed.size = (pixelCount * bpp + 7) / 8;

	packed.buf = calloc(packed.size, sizeof(uint8_t));
	if (!packed.buf)
	{
		fprintf(stderr, "error: failed to allocate memory for packed buffer\n");
		stbi_image_free((void *)img);
		exit(EXIT_FAILURE);
	}

	size_t bitCursor = 0;
	for (uint32_t y = 0; y < yMax; y++)
	{
		const uint8_t *srcRow = &img[(ty * tilesData.height + y) * imgData.width + tx * tilesData.width];

		for (uint32_t x = 0; x < xMax; x++)
		{
			uint8_t v = srcRow[x] & ((1 << bpp) - 1);

			size_t byteIdx = bitCursor / 8;
			size_t bitShift = bitCursor % 8;

			packed.buf[byteIdx] |= v << bitShift;

			if (bitShift + bpp > 8)
				packed.buf[byteIdx + 1] |= v >> ((8 - bitShift));

			bitCursor += bpp;
		}
	}
	return packed;
}

byteBuf_t compress(const byteBuf_t *packed, uint8_t compression)
{
	size_t maxCompressedSize = ZSTD_compressBound(packed->size);

	byteBuf_t compressed;
	compressed.buf = malloc(sizeof(uint8_t) * maxCompressedSize);
	if (!compressed.buf)
	{
		fprintf(stderr, "error: failed to allocate memory for compressed buffer\n");
		return compressed;
	}

	compressed.size = ZSTD_compress(compressed.buf, maxCompressedSize, packed->buf, packed->size, compression);
	if (ZSTD_isError(compressed.size))
	{
		fprintf(stderr, "error: %s\n", ZSTD_getErrorName(compressed.size));
		free(compressed.buf);
		compressed.buf = NULL;
		return compressed;
	}

	return compressed;
}

int createTileFile(const byteBuf_t *compressed, opts_t opts, tilesData_t tilesData, int tx, int ty, const char *outdir)
{
	mkdir(outdir, 0755);

	char filename[256];
	if (outdir[0])
		snprintf(filename, sizeof(filename), "%s/%s_%03d_%03d.zst", outdir, outdir, tx, ty);
	else
		snprintf(filename, sizeof(filename), "tile_%03d_%03d.zst", tx, ty);
	FILE *f = fopen(filename, "wb");
	if (!f)
	{
		fprintf(stderr, "failed to write tile %s\n", filename);
		return 1;
	}
	fwrite(&tilesData.width, sizeof(int), 1, f);
	fwrite(&tilesData.height, sizeof(int), 1, f);
	fwrite(&opts.bpp, sizeof(uint8_t), 1, f);
	fwrite(&opts.compression, sizeof(uint8_t), 1, f);
	fwrite(&opts.margin, sizeof(uint8_t), 1, f);
	fwrite(compressed->buf, sizeof(uint8_t), compressed->size, f);
	fclose(f);

	return 0;
}

int main(int argc, char **argv)
{
	opts_t opts = {
	    .subdivs = SUBDIV,
	    .bpp = BITS_PER_PIXEL,
	    .margin = MARGIN,
	    .compression = COMPRESSION_LEVEL,
	};
	readOpts(argc, argv, &opts);

	char infile[256] = {};
	char outdir[256] = {};
	readArgs(argc, argv, infile, outdir);

	imageData_t    imgData;
	const uint8_t *img = loadImage(&imgData, infile);

	tilesData_t tilesData = computeTilesData(imgData, opts.subdivs);

	for (int ty = 0; ty < tilesData.nbPerSide; ty++)
	{
		for (int tx = 0; tx < tilesData.nbPerSide; tx++)
		{
			const byteBuf_t packed = packPixelData(img, opts.bpp, opts.margin, tilesData, imgData, tx, ty);

			const byteBuf_t compressed = compress(&packed, opts.compression);
			if (!compressed.buf)
			{
				stbi_image_free((void *)img);
				free((void *)packed.buf);
				return EXIT_FAILURE;
			}

			if (createTileFile(&compressed, opts, tilesData, tx, ty, outdir))
			{
				stbi_image_free((void *)img);
				free((void *)packed.buf);
				free((void *)compressed.buf);
				return EXIT_FAILURE;
			}

			free((void *)packed.buf);
			free((void *)compressed.buf);
		}
	}

	stbi_image_free((void *)img);
	printf("Done. %d tiles created in %s\n", tilesData.nbPerSide * tilesData.nbPerSide, outdir);
	return 0;
}
