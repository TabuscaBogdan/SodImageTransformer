//
// Created by Cosmin on 15/12/2019.
//

#include "job.h"
#include "utils.h"

using namespace std;

JobDims Job::ComputeJobDims(const SubImageDim& toProcess, int lastRow, int lastCol) const {
    SubImageDim input;
    SubImageDim output;

    switch (Op.OpType) {
        case Operation::BLUR:
        {
            int r = std::get<BlurParams>(Op.OpParams).R;
            // Input is same as the area to process, but expanded by R in all directions
            input = toProcess;
            input.Expand(r, lastRow, lastCol);

            output = toProcess;
            break;
        }
        case Operation::SEPIA:
        {
            input = toProcess;
            output = toProcess;
            break;
        }
        default:
            assert(false);
    }
    return JobDims(input, toProcess, output);
}

std::vector<MasterSubJob> Job::ComputeJobSplits(int workersCount) {
    const int rows = Image.Rows();
    const int cols = Image.Cols();

    const int lastRow = rows - 1;
    const int lastCol = cols - 1;

    assert(rows > workersCount);
    int rowsPerWorker = CeilDiv(rows, workersCount);

    std::vector<MasterSubJob> jobs;
    jobs.reserve(workersCount);

    for (int worker = 0; worker < workersCount; ++worker) {
        int jobFirstRow = rowsPerWorker * (worker);
        int jobLastRow  = rowsPerWorker * (worker + 1) - 1;

        jobFirstRow = min(jobFirstRow, lastRow);
        jobLastRow  = min(jobLastRow, lastCol);

        SubImageDim toProcess(jobFirstRow, 0, jobLastRow, 0);
        jobs.emplace_back(ComputeJobDims(toProcess, lastRow, lastCol));
    }
    return jobs;
}
