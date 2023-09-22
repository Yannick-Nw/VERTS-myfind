#include <iostream>
//#include <stdlib.h>
#include <vector>
#include <unistd.h>

int main(int argc, char* argv[]) {
    std::vector<std::string> filenames;
    std::string searchpath;
    bool recursive = false;
    bool caseInsensitive = false;

    int opt;
    while ((opt = getopt(argc, argv, "Ri")) != -1) {
        switch (opt) {
            case 'R':
                recursive = true;
                break;
            case 'i':
                caseInsensitive = true;
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " [-R] [-i] searchpath filename1 [filename2]...[filenameN]\n";
                return 1;
        }
    }

    if (optind < argc) {
        searchpath = argv[optind++];
        while (optind < argc) {
            filenames.push_back(argv[optind++]);
        }
    }

    find_files(searchpath, filenames, recursive, caseInsensitive);

    //std::cout << recursive << caseInsensitive << "\n";
    return 0;
}

