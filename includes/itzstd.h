#ifndef ITZSTD_H
#define ITZSTD_H

#include <stddef.h>
#include <stdint.h>

#define SUBDIV            0
#define BITS_PER_PIXEL    8
#define MARGIN            0
#define COMPRESSION_LEVEL 3

typedef struct ImageData
{
	int width;
	int height;
	int channels;
} imageData_t;

typedef struct TilesData
{
	uint16_t nbPerSide;
	uint32_t width;
	uint32_t height;
} tilesData_t;

typedef struct ByteBuf
{
	uint8_t *buf;
	size_t   size;
} byteBuf_t;

typedef struct Opts
{
	uint8_t subdivs;
	uint8_t bpp;
	uint8_t margin;
	uint8_t compression;
} opts_t;

#endif
