//
// Created by Cosmin on 15/12/2019.
//

#ifndef SODIMAGETRANSFORMER_SUB_JOB_H
#define SODIMAGETRANSFORMER_SUB_JOB_H


#include <utility>
#include <algorithm>
#include <variant>
#include "RgbMatrix.h"

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

    Type OpType;
    std::variant<BlurParams, SepiaParams> OpParams;
};

class Job;

struct MasterSubJob {
    JobDims    Dims;
    const Job& ImgJob;

    MasterSubJob(const JobDims& dims, const Job& job) : Dims(dims), ImgJob(job) {}
    MasterSubJob(const MasterSubJob&) = default;
    MasterSubJob& operator = (const MasterSubJob&) = delete;

    void SendInput(int workerId);
    void RecvOutput(int workerId);
    void ExecuteLocal();
};

struct Header {
    JobDims   Dims;
    Operation Op;

    Header() = default;
    Header(const JobDims& dims, const Operation& op) : Dims(dims), Op(op) {}
    Header(const Header&) = default;
    Header& operator=(const Header&) = default;
};

struct SlaveSubJob {
    Header       Hdr;
    RgbSubMatrix Input;
    RgbSubMatrix Output;

    void RecvInput(int masterId);
    void SendOutput(int masterId);
    void ExecuteLocal();
};


#endif //SODIMAGETRANSFORMER_SUB_JOB_H
