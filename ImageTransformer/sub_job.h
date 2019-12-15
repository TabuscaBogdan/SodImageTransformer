//
// Created by Cosmin on 15/12/2019.
//

#ifndef SODIMAGETRANSFORMER_SUB_JOB_H
#define SODIMAGETRANSFORMER_SUB_JOB_H


#include <utility>
#include <algorithm>
#include <variant>
#include "RgbMatrix.h"

struct SubImageDim {
    std::pair<int, int> Min;
    std::pair<int, int> Max;

    SubImageDim() = default;
    SubImageDim(int minRow, int minCol, int maxRow, int maxCol) : Min(minRow, minCol), Max(maxRow, maxCol) {}
    SubImageDim(const SubImageDim&) = default;
    SubImageDim& operator = (const SubImageDim&) = default;

    void Expand(int delta, int lastRow, int lastCol) {
        Min.first  = std::max(Min.first  - delta, 0);
        Min.second = std::max(Min.second - delta, 0);
        Max.first  = std::min(Min.first  + delta, lastRow);
        Max.second = std::min(Min.second + delta, lastCol);
    }
};

/**
 * Example: for a blur operation of ray 2, a possible job dims:
 *   Input:     (2, 2), (8, 9)
 *   ToProcess: (4, 4), (6, 7)
 *   Output:    (4, 4), (6, 7)
 *
 *   The blur operation only processes a part of the matrix, but needs access
 *   to more than that, hence the additional range in the input.
 */
struct JobDims {
    SubImageDim Input;
    SubImageDim ToProcess;
    SubImageDim Output;

    JobDims() = default;
    JobDims(const SubImageDim& in, const SubImageDim& toProcess, const SubImageDim& out)
            : Input(in), ToProcess(toProcess), Output(out) {}
    JobDims(const JobDims&) = default;
    JobDims& operator = (const JobDims&) = default;
};

struct BlurParams {
    int R;

    BlurParams() : BlurParams(0) {}
    explicit BlurParams(int r) : R(r) {}
    BlurParams(const BlurParams&) = default;
    BlurParams& operator=(const BlurParams&) = default;
};

struct SepiaParams {

};

struct Operation {
    enum Type {
        BLUR,
        SEPIA,
    };

    Type                                  OpType;
    std::variant<BlurParams, SepiaParams> OpParams;
};

class Job;

struct MasterSubJob {
    JobDims    Dims;
    const Job& ImgJob;

    MasterSubJob(const JobDims& dims, const Job& job) : Dims(dims), ImgJob(job) {}

    void SendInput(int workerId);
    void RecvOutput(int workerId);
    void ExecuteLocal();
};

struct SlaveSubJob {
    JobDims   Dims;
    Operation Op;
    RgbMatrix Input;
    RgbMatrix Output;

    void RecvInput(int masterId);
    void SendOutput(int masterId);
    void ExecuteLocal();
};


#endif //SODIMAGETRANSFORMER_SUB_JOB_H
