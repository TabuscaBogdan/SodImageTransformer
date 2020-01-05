//
// Created by Cosmin on 15/12/2019.
//

#ifndef SODIMAGETRANSFORMER_OPERATIONS_H
#define SODIMAGETRANSFORMER_OPERATIONS_H

#include "RgbMatrix.h"
#include "sub_job.h"
#include "utils.h"

#include <cmath>

int Clamp(int val, int min, int max);
void MakeSepia(RgbMatrix &m);
void MakeSwirl(RgbMatrix& dst, const RgbMatrix& src, double factor);

template <typename SRC_MATRIX>
void MakeBlur(RgbSubMatrix& dst, const SRC_MATRIX& src, int radius) {
    constexpr double PI = 3.14159265358979323846;

    if (radius < 2) {
        std::cout << "Radius is too small";
        return;
    }

    // Precompute the weights matrix.
    std::vector<std::vector<double>> weights(radius + 1, std::vector<double>(radius + 1));
    for (int i = 0; i <= radius; ++i) {
        for (int j = 0; j <= radius; ++j) {
            int dsq = i*i + j*j;
            double weight = exp(-dsq / (2 * radius * radius)) / (PI * 2 * radius * radius);

            weights[i][j] = weight;
        }
    }

    int i, j;

#pragma omp parallel for collapse(2) private(i, j)
    for (i = dst.MinRow(); i <= dst.MaxRow(); ++i) {
        for (j = dst.MinCol(); j <= dst.MaxCol(); ++j) {
            double val_red = 0, val_blue = 0, val_green = 0, val_alpha = 0;
            double wsum = 0;

            for (int di = -radius; di < radius + 1; ++di) {
                for (int dj = -radius; dj < radius + 1; ++dj) {
                    int x = Clamp(j + dj, src.MinCol(), src.MaxCol());
                    int y = Clamp(i + di, src.MinRow(), src.MaxRow());
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


template <typename SRC_MATRIX>
void Execute(RgbSubMatrix& dst, const SRC_MATRIX& src, const Operation& op, const SubImageDim& outputDim) {
    dst.Resize(outputDim);

    switch (op.OpType) {
        case Operation::BLUR: {
            const BlurParams& p = std::get<BlurParams>(op.OpParams);
            MakeBlur(dst, src, p.R);
            break;
        }
        case Operation::SEPIA: {
            const SepiaParams& p = std::get<SepiaParams>(op.OpParams);
            printf("TODO SEPIA\n");
            UNUSED(p);
//            Output = Input;
//            MakeSepia(Output);
            break;
        }
        default: {
            printf("Unknown operation type\n");
            break;
        }
    }
}


#endif //SODIMAGETRANSFORMER_OPERATIONS_H
