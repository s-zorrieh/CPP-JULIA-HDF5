#include <iostream>
#include <hdf5-julia.h>

using namespace std;

#define INIT_JULIA jl_init(); using_modules();  if (jl_exception_occurred()) {install_dependacy(); using_modules();jl_exception_clear();}

int main()
{
    INIT_JULIA

    SET_PRINTING_STATUS(false);

    const string PATH{"/home/saleh/jul/"};
    const string NAME{"t.hd5"};
    HDF5File myFile1{
                        /* FileName = */ NAME,
                        /* vname    = */ "file1",
                        /* Path     = */ PATH,
                        /* Mode     = */ "r",
                        /* store    = */ false  // If this flag was true, then all the rows had been saved in memory. Memory Leak Warning!
                    };

    for (auto row : myFile1) {
        //TODO:

        row.lsnum;
        row.nbnum;
        row.channelid;

        for(size_t i = 0, total = row.data.lenght(); i < total; ++i){ /* TODO: */  }
    }

    myFile1.closeFile();

    jl_atexit_hook(0);

    return 0;
}



