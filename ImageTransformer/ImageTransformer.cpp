#include <iostream>
#include "bitmap.h"
#include "RgbMatrix.h"
#include <algorithm>
#include <omp.h>
#include <string>


using namespace std;

const double pi = 3.14159265358979323846;
const int base_thread_numbers = 4;

void SetNumberOfThreads(int argc, char* argv[])
{
	int numberOfThreads = base_thread_numbers;
	if (argc > 1)
	{
		try
		{
			numberOfThreads = stoi(argv[1]);
		}
		catch (...)
		{
			cout << "Program will run on base number of threads (4)" << endl;
		}
	}

	omp_set_num_threads(numberOfThreads);
}

void MakeSepia(RgbMatrix &m) {
    for (int i = 0; i < m.Rows(); ++i) {
        for (int j = 0; j < m.Cols(); ++j) {
            float r = (m(i, j).r * 0.393f) + (m(i, j).g * 0.769f) + (m(i, j).b * 0.189f);
            float g = (m(i, j).r * 0.349f) + (m(i, j).g * 0.686f) + (m(i, j).b * 0.168f);
            float b = (m(i, j).r * 0.272f) + (m(i, j).g * 0.534f) + (m(i, j).b * 0.131f);

            m(i, j).r = uint8_t(min(r, 255.0f));
            m(i, j).g = uint8_t(min(g, 255.0f));
            m(i, j).b = uint8_t(min(b, 255.0f));
        }
    }
}

void MakeBlur(RgbMatrix &pixelMatrix, int radius=2)
{
	if(radius < 2)
	{
		cout << "Radius is too small";
		return;
	}

	int i,j;
	
	#pragma omp parallel for collapse(2) private(i,j)
	for(i=0; i<pixelMatrix.Rows(); ++i)
		for(j=0; j<pixelMatrix.Cols(); ++j)
		{
			double val_red = 0, val_blue = 0, val_green = 0, val_alpha=0;
			double wsum = 0;

			for(int iy = i-radius; iy <i+radius+1; ++iy)
				for(int ix = j-radius;ix<j+radius+1; ++ix)
				{
					int x = min(pixelMatrix.Cols() - 1, max(0, ix));
					int y = min(pixelMatrix.Rows() - 1, max(0, iy));
					int dsq = (ix - j)* (ix - j) + (iy - i) * (iy - i);
					double weight = exp(-dsq / (2 * radius * radius)) / (pi * 2 * radius * radius);
					val_red += pixelMatrix(y, x).r * weight;
					val_green += pixelMatrix(y, x).g * weight;
					val_blue += pixelMatrix(y, x).b * weight;
					val_alpha += pixelMatrix(y, x).a * weight;
					wsum += weight;
				}

				pixelMatrix(i, j).r = val_red / wsum;
				pixelMatrix(i, j).g = val_green / wsum;
				pixelMatrix(i, j).b = val_blue / wsum;
				pixelMatrix(i, j).a = val_alpha / wsum;
			
		}
}

void MakeSwirl(RgbMatrix& pixelMatrix, double factor)
{
	int width = pixelMatrix.Cols();
	int height = pixelMatrix.Rows();

	double cX = (double)width / 2.0f;
	double cY = (double)height / 2.0f;

	RgbMatrix cpyMatrix(pixelMatrix);

	#pragma omp parallel for
	for (int i = 0; i < height; i++)
	{
		double relY = cY - i;
		for (int j = 0; j < width; j++)
		{
			double relX = j - cX;
			// relX and relY are points in our UV space
			// Calculate the angle our points are relative to UV origin. Everything is in radians.
			double originalAngle;

			if (relX != 0)
			{
				originalAngle = atan(abs(relY) / abs(relX));
				if (relX > 0 && relY < 0) 
					originalAngle = 2.0f * pi - originalAngle;
				else 
					if (relX <= 0 && relY >= 0) 
						originalAngle = pi - originalAngle;
					else 
						if (relX <= 0 && relY < 0) 
							originalAngle += pi;
			}
			else
			{
				// Take care of rare special case
				if (relY >= 0) 
					originalAngle = 0.5f * pi;
				else 
					originalAngle = 1.5f * pi;
			}
			// Calculate the distance from the center of the UV using pythagorean distance
			double radius = sqrt(relX * relX + relY * relY);

			// Use any equation we want to determine how much to rotate image by
			//double newAngle = originalAngle + factor*radius;	// a progressive twist
			double newAngle = originalAngle + 1 / (factor * radius + (4.0f / pi));
			// Transform source UV coordinates back into bitmap coordinates
			int srcX = (int)(floor(radius * cos(newAngle) + 0.5f));
			int srcY = (int)(floor(radius * sin(newAngle) + 0.5f));

			srcX += cX;
			srcY += cY;
			srcY = height - srcY;

			if (srcX < 0) 
				srcX = 0;
			else 
				if (srcX >= width) 
					srcX = width - 1;
			if (srcY < 0) 
				srcY = 0;
			else 
				if (srcY >= height) 
					srcY = height - 1;

			// Set the pixel color
			pixelMatrix(i, j).r = cpyMatrix(srcY, srcX).r;
			pixelMatrix(i, j).g = cpyMatrix(srcY, srcX).g;
			pixelMatrix(i, j).b = cpyMatrix(srcY, srcX).b;
			pixelMatrix(i, j).a = cpyMatrix(srcY, srcX).a;
			
		}
	}
}

int main(int argc, char* argv[]) {

	SetNumberOfThreads(argc, argv);

    bitmap bmp{"../data/sunflower.bmp"};

    RgbMatrix m{bmp};
	auto start = omp_get_wtime();
    //MakeBlur(m,10);
	MakeSwirl(m, 0.001);
	auto stop = omp_get_wtime();
	cout << "Time Taken:" << stop - start<<endl;
    m.ToBitmap(&bmp);

    bmp.save("../data/sunflower_swirl.bmp");
}
