//
// Created by Cosmin on 15/12/2019.
//

#include "operations.h"
#include "RgbMatrix.h"
#include "utils.h"

#include <cmath>
#include <iostream>
#include <algorithm>

using namespace std;

const double PI = 3.14159265358979323846;

int Clamp(int val, int min, int max) {
    assert(min < max);

    if (val < min) {
        val = min;
    }
    if (val > max) {
        val = max;
    }
    return val;
}

void MakeSepia(RgbMatrix& m) {
    int i, j;
    #pragma omp parallel for private(i,j)
    for (i = 0; i < m.Rows(); ++i) {
        for (j = 0; j < m.Cols(); ++j) {
            float r = (m(i, j).r * 0.393f) + (m(i, j).g * 0.769f) + (m(i, j).b * 0.189f);
            float g = (m(i, j).r * 0.349f) + (m(i, j).g * 0.686f) + (m(i, j).b * 0.168f);
            float b = (m(i, j).r * 0.272f) + (m(i, j).g * 0.534f) + (m(i, j).b * 0.131f);

            m(i, j).r = uint8_t(min(r, 255.0f));
            m(i, j).g = uint8_t(min(g, 255.0f));
            m(i, j).b = uint8_t(min(b, 255.0f));
        }
    }
}

void MakeSwirl(RgbMatrix& dst, const RgbMatrix& src, double factor) {
    assert (dst.Rows() == src.Rows() && dst.Cols() == src.Cols());

    int width = src.Cols();
    int height = src.Rows();

    double cX = (double)width / 2.0f;
    double cY = (double)height / 2.0f;

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
                    originalAngle = 2.0f * PI - originalAngle;
                else
                if (relX <= 0 && relY >= 0)
                    originalAngle = PI - originalAngle;
                else
                if (relX <= 0 && relY < 0)
                    originalAngle += PI;
            }
            else
            {
                // Take care of rare special case
                if (relY >= 0)
                    originalAngle = 0.5f * PI;
                else
                    originalAngle = 1.5f * PI;
            }
            // Calculate the distance from the center of the UV using pythagorean distance
            double radius = sqrt(relX * relX + relY * relY);

            // Use any equation we want to determine how much to rotate image by
            //double newAngle = originalAngle + factor*radius;	// a progressive twist
            double newAngle = originalAngle + 1 / (factor * radius + (4.0f / PI));
            // Transform source UV coordinates back into bitmap coordinates
            int srcX = (int)(floor(radius * cos(newAngle) + 0.5f));
            int srcY = (int)(floor(radius * sin(newAngle) + 0.5f));

            srcX += cX;
            srcY += cY;
            srcY = height - srcY;

            srcX = Clamp(srcX, 0, width - 1);
            srcY = Clamp(srcY, 0, height - 1);

            // Set the pixel color
            dst(i, j) = src(srcY, srcX);
        }
    }
}

