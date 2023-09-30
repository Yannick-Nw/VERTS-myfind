#include <iostream>
#include <vector>
#include <filesystem>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <cctype>

namespace fs = std::filesystem;

std::string str_tolower(std::string s){
    // Convert str to lowercase for insensiv search
    for (auto& x : s) {x = tolower(x);}
    return s;
}

void checkFile(std::filesystem::directory_entry entry, bool caseInsensitive, std::string filename){
    // Check if a file is the one searched for
    bool isFile = entry.is_regular_file();
    std::string pathname = fs::path(entry).filename();
    if (caseInsensitive){
        pathname = str_tolower(pathname);
    }
    if (isFile && (pathname == filename)){
        std::string absolutePath = fs::current_path().string() + entry.path().string().substr(1);
        std::cout << getpid() << ": " << filename << ": " << absolutePath << '\n';
    }
}

void findFile(std::string searchpath, std::string filename, bool recursive, bool caseInsensitive){
    // Child process searching one file
    if (caseInsensitive){
        filename = str_tolower(filename);
    }

    if (recursive){
        // Recursive search
        for (auto const& entry : fs::recursive_directory_iterator(searchpath)){
            checkFile(entry, caseInsensitive, filename);
        }
    } else {
        // Non recursive search
        for (auto const& entry : fs::directory_iterator(searchpath)){
            checkFile(entry, caseInsensitive, filename);
        }    
    }
}

void findFiles(std::string searchpath, std::vector<std::string> filenames, bool recursive, bool caseInsensitive){
    // Start search and folk a child process for each file
    pid_t pids[filenames.size()];
    int status;
    bool isParent = true;
    for (size_t i = 0; i < filenames.size(); ++i) {
        pids[i] = fork();
        if (pids[i] < 0) {
            std::cout << "Error forking!\n";
        } else if (pids[i] == 0) {
            isParent = false;
            findFile(searchpath, filenames[i], recursive, caseInsensitive);
        }
    }
    // Wait for child processes to finish
    if (isParent) {
        for (size_t i = 0; i < filenames.size(); ++i) {
            waitpid(pids[i], &status, 0);
        }
    }
}

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

    // Check if searchpath exists
    if (std::filesystem::directory_entry(searchpath).exists() == false){
        std::cout << "Error: Directory does not exist!\n";
        return -1;
    }

    if (filenames.size() == 1){
        findFile(searchpath, filenames[0], recursive, caseInsensitive);
    } else if (filenames.size() > 1) {
        findFiles(searchpath, filenames, recursive, caseInsensitive);
    } else {
        std::cout << "Error: Not enough arguments.\n";
    }
    return 0;
}

