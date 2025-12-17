#ifndef ITZSTD_H
#define ITZSTD_H

#include <stddef.h>
#include <stdint.h>

#define SUBDIV            5
#define BITS_PER_PIXEL    3
#define COMPRESSION_LEVEL 5

typedef struct ImageData
{
	int width;
	int height;
	int channels;
} imageData_t;

typedef struct TilesData
{
	int nbPerSide;
	int width;
	int height;
} tilesData_t;

typedef struct ByteBuf
{
	uint8_t *buf;
	size_t   size;
} byteBuf_t;

#endif
