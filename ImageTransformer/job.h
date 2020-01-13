//
// Created by Cosmin on 15/12/2019.
//

#ifndef SODIMAGETRANSFORMER_JOB_H
#define SODIMAGETRANSFORMER_JOB_H

#include "RgbMatrix.h"
#include "sub_job.h"

struct Job {
    const RgbMatrix& Image;
    const Operation& Op;
    const bool       Async;
    RgbMatrix        Output;

    Job(const RgbMatrix& img, const Operation& op, const bool async) : Image(img), Op(op), Async(async) {}

public:

    void Execute(int procCount);

private:
    std::vector<MasterSubJob> ComputeJobSplits(int workersCount);

    MasterSubJob ComputeJobDims(const SubImageDim& toProcess, int lastRow, int lastCol) const;

};

#endif //SODIMAGETRANSFORMER_JOB_H
