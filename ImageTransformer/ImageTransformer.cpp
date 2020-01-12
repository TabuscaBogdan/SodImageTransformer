#include <iostream>
#include <iomanip>
#include <string>
#include <variant>

#include <omp.h>
#include <mpi.h>

#include "RgbMatrix.h"
#include "operations.h"
#include "job.h"
#include "utils.h"
#include "sub_job.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb/stb_image_write.h"

#define STB_IMAGE_IMPLEMENTATION

#include "stb/stb_image.h"

using namespace std;

void SetNumberOfThreads(int numThreads) {
    omp_set_num_threads(numThreads);
}

void log(const string& func, const string& img, int nt, double time) {
//    cout << "Time taken for " << nt << " threads"  << " on '" << func << "' on '" << img << "':" << time << '\n';
    ofstream fout("../output_data/log.csv", ios::app);
    assert(fout);

    const char* SEP = ",";

    fout << fixed << setprecision(5);

#ifndef NDEBUG
    fout << "Debug" << SEP;
#else
    fout << "Release" << SEP;
#endif

    fout << nt << SEP <<
         func << SEP <<
         img << SEP <<
         time << '\n';
}

int main_single_machine(int argc, char* argv[]) {
    UNUSED(argc);
    UNUSED(argv);

    const string DATA_DIR      = "../data/";
    const string FUNCS[]       = {"sepia", "blur", "swirl"};
    const string IMAGE         = "600x600";
    const string SRC_EXT       = ".bmp";
    const string DST_EXT       = ".png";
    const int    RUNS_PER_FUNC = 3;
    const bool   SAVE          = false;
    assert(DST_EXT == ".png"); // Because we currently only save as PNG.

    const string srcImg = DATA_DIR + IMAGE + SRC_EXT;

//    cout << "Reading image...\n";
    RgbMatrix m(srcImg.c_str());
//    cout << "Done reading image.\n";
    RgbMatrix mDst(m.Rows(), m.Cols());
//    cout << "Done creating output buffer.\n";

    for (int nt : {1, 2, 4, 8}) {
        SetNumberOfThreads(nt);

        double start = -1;
        double stop  = -1;

        vector<string> dstImgs;
        for (auto func : FUNCS) {
            dstImgs.push_back(DATA_DIR + IMAGE + "_" + func + DST_EXT);
        }

        for (int run = 0; run < RUNS_PER_FUNC; ++run) {
            // Sepia
            start = omp_get_wtime();
            mDst  = m;
            MakeSepia(mDst);
            stop = omp_get_wtime();
            if (SAVE) {
                mDst.SaveAsPng(dstImgs[0].c_str());
            }
            log(FUNCS[0], IMAGE, nt, stop - start);

            // Blur
            start = omp_get_wtime();
//            MakeBlur(mDst, m, 5);
//             TODO
            stop = omp_get_wtime();
            if (SAVE) {
                mDst.SaveAsPng(dstImgs[1].c_str());
            }
            log(FUNCS[1], IMAGE, nt, stop - start);

            // Swirl
            start = omp_get_wtime();
            MakeSwirl(mDst, m, 0.001);
            stop = omp_get_wtime();
            if (SAVE) {
                mDst.SaveAsPng(dstImgs[2].c_str());
            }
            log(FUNCS[2], IMAGE, nt, stop - start);
        }
    }

    return 0;
}

int main_multi_machine(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int id, procCount;
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &procCount);

    printf("id = %d, count = %d\n", id, procCount);

    if (id == 0) {
//        Operation op((SepiaParams()));
        Operation op(BlurParams(5, 50));

        const char* DIR  = "../data/";
        const char* BASE = "1920x1080";
        const char* EXT  = ".bmp";

        const string inPath  = SSTR(DIR << BASE << EXT);
        const string outPath = SSTR(DIR << BASE << op.ToString() << ".png");

        cout << "inPath = " << inPath << '\n';
        cout << "outPath = " << outPath << '\n';

        RgbMatrix m(inPath);

        Job job(m, op);

        double startTime = MPI_Wtime();

        std::vector<MasterSubJob> jobs = job.ComputeJobSplits(procCount);

        for (int workerId = 1; workerId < procCount; ++workerId) {
            jobs[workerId].SendInput(workerId);
        }
        printf("Master sent job inputs to all slaves\n");

        jobs[0].ExecuteLocal();
        for (int workerId = 1; workerId < procCount; ++workerId) {
            jobs[workerId].RecvOutput(workerId);
        }
        printf("Master recv'd all outputs from all slaves\n");

        job.JoinSubJobOutputs(jobs);

        double endTime = MPI_Wtime();

        log(op.ToString(), BASE, procCount, endTime - startTime);

        job.Output.SaveAsPng(outPath.c_str());
    } else {
        SlaveSubJob job;

        double startTime = MPI_Wtime();
        job.RecvInput(0);
        double afterRecvTime = MPI_Wtime();
        job.ExecuteLocal();
        double afterExecTime = MPI_Wtime();
        job.SendOutput(0);
        double doneTime = MPI_Wtime();

        cout << "Recv (wrong): " << afterRecvTime -  startTime << " sec" << '\n';
        cout << "Exec: " << afterExecTime - afterRecvTime << " sec" << '\n';
        cout << "Send (wrong): " <<  doneTime - afterExecTime << " sec" << '\n';
        cout << "Total: " << doneTime - startTime << " sec" << '\n';
    }

    MPI_Finalize();
    return 0;
}

int main(int argc, char* argv[]) {
    return main_multi_machine(argc, argv);
}