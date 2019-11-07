#pragma once
#include <cstdint>
#include <fstream>

typedef struct
{
	uint8_t r, g, b, a;
} rgb32;
#pragma pack(2)
typedef struct
{
	uint16_t bfType;
	uint32_t bfSize;
	uint16_t bfReserved1;
	uint16_t bfReserved2;
	uint32_t bfOffBits;
} bitmap_file_header;
#pragma pack()
#pragma pack(2)
typedef struct
{
	uint32_t biSize;
	int32_t biWidth;
	int32_t biHeight;
	uint16_t biPlanes;
	uint16_t biBitCount;
	uint32_t biCompression;
	uint32_t biSizeImage;
	int16_t biXPelsPerMeter;
	int16_t biYPelsPerMeter;
	uint32_t biClrUsed;
	uint32_t biClrImportant;
} bitmap_info_header;
#pragma pack()
#pragma pack(2)
typedef struct
{
	bitmap_file_header fHeader;
	bitmap_info_header iHeader;
} bitmap_header;
#pragma pack()

class bitmap
{
private:
	bitmap_header header;
	uint8_t* pixels;
public:
	bitmap(const char* path);
	~bitmap();
	void save(const char* path);
	rgb32* getPixel(uint32_t x, uint32_t y) const;
	void setPixel(rgb32* pixel, uint32_t x, uint32_t y);
	uint32_t getWidth() const;
	uint32_t getHeight() const;
};