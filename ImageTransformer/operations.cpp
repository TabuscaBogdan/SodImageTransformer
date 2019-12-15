//
// Created by Cosmin on 15/12/2019.
//

#include "operations.h"

#include <iostream>
#include <algorithm>
#include <cmath>

#include "RgbMatrix.h"

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

void MakeBlur(RgbMatrix& dst, const RgbMatrix& src, int radius) {
    assert (dst.Rows() == src.Rows() && dst.Cols() == src.Cols());

    if (radius < 2) {
        cout << "Radius is too small";
        return;
    }

    // Precompute the weights matrix.
    vector<vector<double>> weights(radius + 1, vector<double>(radius + 1));
    for (int i = 0; i <= radius; ++i) {
        for (int j = 0; j <= radius; ++j) {
            int dsq = i*i + j*j;
            double weight = exp(-dsq / (2 * radius * radius)) / (PI * 2 * radius * radius);

            weights[i][j] = weight;
        }
    }

    int i, j;

    #pragma omp parallel for collapse(2) private(i, j)
    for (i = 0; i < src.Rows(); ++i) {
        for (j = 0; j < src.Cols(); ++j) {
            double val_red = 0, val_blue = 0, val_green = 0, val_alpha = 0;
            double wsum = 0;

            for (int di = -radius; di < radius + 1; ++di) {
                for (int dj = -radius; dj < radius + 1; ++dj) {
                    int x = Clamp(j + dj, 0, src.Cols() - 1);
                    int y = Clamp(i + di, 0, src.Rows() - 1);
                    double weight = weights[abs(di)][abs(dj)];

                    val_red += src(y, x).r * weight;
                    val_green += src(y, x).g * weight;
                    val_blue += src(y, x).b * weight;
                    val_alpha += src(y, x).a * weight;
                    wsum += weight;
                }
            }

            dst(i, j).r = val_red / wsum;
            dst(i, j).g = val_green / wsum;
            dst(i, j).b = val_blue / wsum;
            dst(i, j).a = val_alpha / wsum;
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
