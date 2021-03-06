//
// Created by Cosmin on 11/11/2019.
//

#ifndef SODIMAGETRANSFORMER_RGBMATRIX_H
#define SODIMAGETRANSFORMER_RGBMATRIX_H
#pragma once

#include <vector>
#include <cstring>
#include <cassert>

#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#include "bitmap.h"

const int NUM_CHANNELS = 3;

class RgbMatrix {
public:
    RgbMatrix() : RgbMatrix(0, 0) {}
    RgbMatrix(int rows, int cols) {
        Resize(rows, cols);
    }
    explicit RgbMatrix(const bitmap& bm) {
        Resize(bm.getWidth(), bm.getHeight());

        for (int row = 0; row < _rows; ++row) {
            for (int col = 0; col < _cols; ++col) {
                rgb32* pixel = bm.getPixel(row, col);
                (*this)(row, col) = *pixel;
            }
        }
    }
    explicit RgbMatrix(const char* path) {
        ReadFromFile(path);
    }
    RgbMatrix(const RgbMatrix&) = default;
    RgbMatrix& operator= (const RgbMatrix&) = default;

    void ReadFromFile(const char* path) {
        int width, height, bpp;
        uint8_t* rgbImg = stbi_load(path, &width, &height, &bpp, NUM_CHANNELS);

        assert(bpp == NUM_CHANNELS);

        Resize(height, width);
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                (*this)(i, j).r = rgbImg[(i * width + j) * NUM_CHANNELS + 0];
                (*this)(i, j).g = rgbImg[(i * width + j) * NUM_CHANNELS + 1];
                (*this)(i, j).b = rgbImg[(i * width + j) * NUM_CHANNELS + 2];
                (*this)(i, j).a = 0;
            }
        }

        stbi_image_free(rgbImg);
    }

    void SaveAsPng(const char* path) const {
        int width = Cols();
        int height = Rows();

        std::vector<uint8_t> data;
        data.reserve(width * height * NUM_CHANNELS);

        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                data.push_back((*this)(i, j).r);
                data.push_back((*this)(i, j).g);
                data.push_back((*this)(i, j).b);
            }
        }

        int stride = width * NUM_CHANNELS;
        stbi_write_png(path, width, height, NUM_CHANNELS, &data[0], stride);
    }

    void ToBitmap(bitmap* bm) const {
        assert(bm->getWidth() == _rows);
        assert(bm->getHeight() == _cols);

        for (int row = 0; row < _rows; ++row) {
            for (int col = 0; col < _cols; ++col) {
                auto pixel = (*this)(row, col);
                bm->setPixel(&pixel, row, col);
            }
        }
    }

    int Rows() const {
        return _rows;
    }

    int Cols() const {
        return _cols;
    }

    void Resize(int rows, int cols) {
        _rows = rows;
        _cols = cols;
        _data.resize(rows * cols);
    }

    void Zero() {
        rgb32 zeroValue = rgb32();
        std::fill(_data.begin(), _data.end(), zeroValue);
    }

public:
    const rgb32& operator()(int i, int j) const {
        assert(0 <= i && i < _rows);
        assert(0 <= j && j < _cols);

        auto index = i * _cols + j;
        assert(0 <= index && index < _data.size());

        return _data[index];
}

    rgb32& operator()(int i, int j) {
        assert(0 <= i && i < _rows);
        assert(0 <= j && j < _cols);

        auto index = i * _cols + j;
        assert(0 <= index && index < _data.size());

        return _data[index];
    }

private:
    std::vector<rgb32> _data;
    int _rows;
    int _cols;
};


#endif //SODIMAGETRANSFORMER_RGBMATRIX_H
