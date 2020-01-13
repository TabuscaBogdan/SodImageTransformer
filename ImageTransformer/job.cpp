//
// Created by Cosmin on 15/12/2019.
//

#include <mpi.h>
#include "job.h"
#include "utils.h"
#include "sub_image_dim.h"

using namespace std;

MasterSubJob Job::ComputeJobDims(const SubImageDim& toProcess, int lastRow, int lastCol) const {
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
    return MasterSubJob(JobDims(input, toProcess, output), *this);
}

std::vector<MasterSubJob> Job::ComputeJobSplits(int workersCount) {
    const int rows = Image.Rows();
    const int cols = Image.Cols();

    const int lastRow = rows - 1;
    const int lastCol = cols - 1;

    cout << "[Job::ComputeJobSplits]: rows = " << rows << ", cols = " << cols << '\n';

    assert(rows > workersCount);
    int rowsPerWorker = CeilDiv(rows, workersCount);

    std::vector<MasterSubJob> jobs;
    jobs.reserve(workersCount);

    for (int worker = 0; worker < workersCount; ++worker) {
        int jobFirstRow = rowsPerWorker * (worker);
        int jobLastRow  = rowsPerWorker * (worker + 1) - 1;

        jobFirstRow = min(jobFirstRow, lastRow);
        jobLastRow  = min(jobLastRow, lastRow);

        SubImageDim toProcess(jobFirstRow, 0, jobLastRow, lastCol);
        MasterSubJob job = ComputeJobDims(toProcess, lastRow, lastCol);
        jobs.emplace_back(std::move(job));

        cout << "job: " << toProcess.ToString() << '\n';
    }
    return jobs;
}

void Job::Execute(int procCount) {
    vector<MasterSubJob> jobs = ComputeJobSplits(procCount);

    // reqs[0] is unused
    vector<MPI_Request> reqs(procCount, MPI_REQUEST_NULL);

    for (int workerId = 1; workerId < procCount; ++workerId) {
        MPI_Request reqUnused = MPI_REQUEST_NULL;
        jobs[workerId].SendInput(workerId, Async, &reqUnused); // we only care about the return requests
//        MPI_Request_free(&reqUnused);
    }
    printf("Master sent job inputs to all slaves\n");

    jobs[0].ExecuteLocal();

    Output.Resize(Image.Rows(), Image.Cols());
    Output.CopyFrom(jobs[0].Output);

    for (int workerId = 1; workerId < procCount; ++workerId) {
        jobs[workerId].RecvOutput(workerId, Async, &reqs[workerId]);
    }
    printf("Master recv'd all outputs from all slaves\n");


    if (Async) {
        int doneIndex;
        MPI_Status doneStatus;
        const int slavesCount = procCount - 1;

        for (int doneCount = 0; doneCount < slavesCount; ++doneCount) {
            MPI_Waitany(reqs.size(), &reqs[0], &doneIndex, &doneStatus);
            // TODO
            Output.CopyFrom(jobs[doneIndex].Output);
//            MPI_Request_free(&reqs[doneIndex]);
        }
    } else {
        // all jobs are done at this time if we run sync
        for (const MasterSubJob& job : jobs) {
            Output.CopyFrom(job.Output);
        }
    }
}
