#include <iostream>
#include <vector>
#include <filesystem>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <cctype>

namespace fs = std::filesystem;

std::string str_tolower(std::string text){
    // Convert str to lowercase for insensiv search
    for (auto& letter : text) { letter = tolower(letter);}
    return text;
}

void checkFile(std::filesystem::directory_entry entry, bool caseInsensitive, std::string filename, int pipefd[2] = nullptr){
    // Check if a file is the one searched for
    bool isFile = entry.is_regular_file();
    std::string pathname = fs::path(entry).filename();
    if (caseInsensitive){
        pathname = str_tolower(pathname);
    }
    if (isFile && (pathname == filename)){
        std::string absolutePath = fs::current_path().string() + entry.path().string().substr(1);
        std::string output = std::to_string(getpid()) + ": " + filename + ": " + absolutePath + '\n';
        if (pipefd) {
        write(pipefd[1], output.c_str(), output.size());
        } else {
            std::cout << output;
        }
    }
}

void findFile(std::string searchPath, std::string filename, bool recursive, bool caseInsensitive, int pipefd[2] = nullptr){
    // Child process searching one file
    if (caseInsensitive){
        filename = str_tolower(filename);
    }

    if (recursive){
        // Recursive search
        for (auto const& entry : fs::recursive_directory_iterator(searchPath)){
            checkFile(entry, caseInsensitive, filename, pipefd);
        }
    } else {
        // Non recursive search
        for (auto const& entry : fs::directory_iterator(searchPath)){
            checkFile(entry, caseInsensitive, filename, pipefd);
        }
    }
}

void findFiles(std::string searchPath, std::vector<std::string> filenames, bool recursive, bool caseInsensitive){
    // Start search and fork a child process for each file
    pid_t pids[filenames.size()];
    int status;
    bool isParent = true;
    for (size_t i = 0; i < filenames.size(); ++i) {
        int pipefd[2];
        if (pipe(pipefd) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        pids[i] = fork();
        if (pids[i] < 0) {
            std::cout << "Error forking!\n";
        } else if (pids[i] == 0) {
            isParent = false;
            close(pipefd[0]); // Schließen Sie das Lesende der Pipe im Child-Prozess
            findFile(searchPath, filenames[i], recursive, caseInsensitive, pipefd);
            close(pipefd[1]); // Schließen Sie das Schreibende der Pipe im Child-Prozess
            exit(EXIT_SUCCESS);
        } else {
            close(pipefd[1]); // Schließen Sie das Schreibende der Pipe im Parent-Prozess
            char buf;
            while (read(pipefd[0], &buf, 1) > 0) {
                std::cout << buf; // Lesen Sie die Daten aus der Pipe und geben Sie sie aus
            }
            close(pipefd[0]);
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
    std::string searchPath;
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
                std::cerr << "Usage: " << argv[0] << " [-R] [-i] searchPath filename1 [filename2]...[filenameN]\n";
                return 1;
        }
    }

    if (optind < argc) {
        searchPath = argv[optind++];
        while (optind < argc) {
            filenames.push_back(argv[optind++]);
        }
    }

    // Check if searchPath exists
    if (std::filesystem::directory_entry(searchPath).exists() == false){
        std::cout << "Error: Directory does not exist!\n";
        return -1;
    }

    if (filenames.size() == 1){
        findFile(searchPath, filenames[0], recursive, caseInsensitive);
    } else if (filenames.size() > 1) {
        findFiles(searchPath, filenames, recursive, caseInsensitive);
    } else {
        std::cout << "Error: Not enough arguments.\n";
    }
    return 0;
}

