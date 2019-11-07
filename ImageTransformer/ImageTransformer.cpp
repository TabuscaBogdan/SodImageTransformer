#include <iostream>
#include "bitmap.h"


int main()
{
	bitmap bmp{ "C:\\Faculta\\Sod\\BitmapSunflower.bmp" };
	for (int i = 0; i < bmp.getWidth(); ++i)
	{
		for(int j=0;j<bmp.getHeight(); ++j)
		{
			rgb32 *pixel = bmp.getPixel(i, j);
			pixel->a = 25;
			pixel->r = 25;

			bmp.setPixel(pixel, i, j);
		}
		
	}
	bmp.save("C:\\Faculta\\Sod\\BitmapSunflowerCPY.bmp");
}
