#include <iostream>
//#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
/* Include Datei für getopt(); getopt.h geht auch, ist aber älter. */
#include <unistd.h>

int main(int argc, char *argv[]){
    int opt = 0;
    bool recursive = false;
    bool caseInsensitive = false;

    while ((opt = getopt(argc, argv, "Ri")) != EOF)
    {
        switch (opt)
        {
            case 'R':
                recursive = true;
                break;
            case 'i':
                caseInsensitive = true;
                break;
            default:
                assert(0);
        }
    }

    return 0;
}
