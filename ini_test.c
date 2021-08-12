
#include <stdio.h>
#include "file_ini.h"

void usage(const char *file)
{
    printf("%s [ini]\n", file);
}

int main(int argc, char **argv)
{
    stFIHandle *hIni = NULL;

    if (argc > 1) {
        hIni = (stFIHandle *)fiFileRead(argv[1]);
        if (hIni) {
//              fiPut(hIni, "TEST", "T1", "1");
//              fiPut(hIni, "TEST", "T2", "2");
//              fiPut(hIni, "TEST", "T3", "3");
//              fiPut(hIni, "TEST", "T1", "3");

            fiShow(hIni);
            fiDestroy(hIni);
        }
    }
    else {
        usage(argv[0]);
    }

    return 0;
}

