#include <iostream>
#include "PixelColoring.h"

rgb32 ***PixelMatrix(bitmap bmp)
{
	int rows = bmp.getHeight();
	int columns = bmp.getWidth();
	int i, j;

	rgb32*** pixelMatrix = new rgb32**[rows];
	for(i=0; i<rows; ++i)
	{
		pixelMatrix[i] = new rgb32 * [columns];
	}

	for(i=0; i<rows; ++i)
		for(j=0; j<columns; ++j)
		{
			rgb32* pixel = new rgb32;
			memcpy(pixel, bmp.getPixel(i, j), sizeof(rgb32));
			pixelMatrix[i][j] = pixel;
		}

	return  pixelMatrix;
}

void MakeSepia(rgb32*** pixelMatrix, int rows, int columns)
{
	int i, j;
	for(i=0;i<rows;++i)
		for(j=0;j<columns;++j)
		{
			pixelMatrix[i][j]->r = unsigned char(pixelMatrix[i][j]->r * 0.393f)+ unsigned char(pixelMatrix[i][j]->g * 0.769f) + unsigned char(pixelMatrix[i][j]->b * 0.189f);
			pixelMatrix[i][j]->g = unsigned char(pixelMatrix[i][j]->r * 0.349f) + unsigned char(pixelMatrix[i][j]->g * 0.686f) + unsigned char(pixelMatrix[i][j]->b * 0.168f);
			pixelMatrix[i][j]->b = unsigned char(pixelMatrix[i][j]->r * 0.272f) + unsigned char(pixelMatrix[i][j]->g * 0.534f) + unsigned char(pixelMatrix[i][j]->b * 0.131f);
		}
}

int main()
{
	bitmap bmp{ "C:\\Faculta\\Sod\\BitmapSunflower.bmp" };
	int rows, columns;
	rgb32*** pixelMatrix;

	rows = bmp.getHeight();
	columns = bmp.getWidth();
	pixelMatrix = PixelMatrix(bmp);

	MakeSepia(pixelMatrix, rows, columns);


	bmp.save("C:\\Faculta\\Sod\\BitmapSunflowerCPY.bmp",pixelMatrix);
}
