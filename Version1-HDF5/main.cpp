#include <iostream>
#include <hdf5-julia.h>

using namespace std;

#define INIT_JULIA jl_init(); using_modules();  if (jl_exception_occurred()) {install_dependacy(); using_modules();jl_exception_clear();}

int main()
{
    INIT_JULIA

    const string PATH{"/home/saleh/jul/"};
    const string NAME{"t.hd5"};

    HDF5File myFile{NAME, "file1", PATH};

    for (auto row : myFile.rows()) {
        //TODO:

        row.lsnum;
        row.nbnum;
        row.channelid;

        for(size_t i = 0, total = row.data.lenght(); i < total; ++i){
            //TODO:

            row.data[i];
        }
    }

    myFile.closeFile();


    jl_atexit_hook(0);

    return 0;
}



