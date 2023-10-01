#include <iostream>
#include <vector>
#include <filesystem>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <cctype>

namespace fs = std::filesystem;

std::string str_tolower(std::string text)
{
    // Convert str to lowercase for case-insensitive search
    for (auto& letter : text) {letter = tolower(letter);}
    return text;
}

// Function to check if a file is the one being searched for
void checkFile(std::filesystem::directory_entry entry, bool caseInsensitive, std::string filename, int pipefd[2] = nullptr)
{
    // Check if the entry is a regular file
    bool isFile = entry.is_regular_file();

    // Get the filename from the entry
    std::string pathname = fs::path(entry).filename();

    // Convert the filename to lowercase if case-insensitive search is enabled
    if (caseInsensitive) {
        pathname = str_tolower(pathname);
    }

    // If the entry is a file and its name matches the search filename
    if (isFile && (pathname == filename)) {
        // Construct the absolute path of the file
        std::string absolutePath = fs::current_path().string() + entry.path().string().substr(1);

        // Prepare the output string
        std::string output = std::to_string(getpid()) + ": " + filename + ": " + absolutePath + '\n';

        // If a pipe is provided, write the output to the pipe, otherwise print it to stdout
        if (pipefd) {
            write(pipefd[1], output.c_str(), output.size());
        } else {
            std::cout << output;
        }
    }
}

// Function to search for a file in a directory
void findFile(std::string searchPath, std::string filename, bool recursive, bool caseInsensitive, int pipefd[2] = nullptr)
{
    // Convert the filename to lowercase if case-insensitive search is enabled
    if (caseInsensitive) {
        filename = str_tolower(filename);
    }

    // Create a directory iterator based on whether recursive search is enabled or not
    if (recursive) {
        // Recursive search
        for (auto const& entry : fs::recursive_directory_iterator(searchPath)) {
            checkFile(entry, caseInsensitive, filename, pipefd);
        }
    } else {
        // Non recursive search
        for (auto const& entry : fs::directory_iterator(searchPath)) {
            checkFile(entry, caseInsensitive, filename, pipefd);
        }
    }
}

// Function to search for multiple files in a directory
void findFiles(std::string searchPath, std::vector<std::string> filenames, bool recursive, bool caseInsensitive)
{
    pid_t pids[filenames.size()];
    int status;
    bool isParent = true;
    // Iterate over each filename in the list of filenames to search for
    for (size_t i = 0; i < filenames.size(); ++i) {
        // Create a pipe for inter-process communication
        int pipefd[2];
        if (pipe(pipefd) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        // Fork a new process for each file search
        pids[i] = fork();
        if (pids[i] < 0) {
            std::cout << "Error forking!\n";
        } else if (pids[i] == 0) {
            isParent = false;
            // In child process: close read end of pipe and start file search
            close(pipefd[0]);
            findFile(searchPath, filenames[i], recursive, caseInsensitive, pipefd);
            close(pipefd[1]);
            exit(EXIT_SUCCESS);
        } else {
            // In parent process: close write end of pipe and read results from child process
            close(pipefd[1]);
            char buf;
            while (read(pipefd[0], &buf, 1) > 0) {
                std::cout << buf;
            }
            close(pipefd[0]);
        }
    }
    // In parent process: wait for all child processes to finish
    if (isParent) {
        for (size_t i = 0; i < filenames.size(); ++i) {
            waitpid(pids[i], &status, 0);
        }
    }
}

int main(int argc, char* argv[])
{
    std::vector<std::string> filenames;
    std::string searchPath;
    bool recursive = false;
    bool caseInsensitive = false;

    // Process command-line arguments using getopt and set options accordingly
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

    // Get the search path and filenames from the remaining command-line arguments
    if (optind < argc) {
        searchPath = argv[optind++];
        while (optind < argc) {
            filenames.push_back(argv[optind++]);
        }
    }

    // Check if searchPath exists
    if (std::filesystem::directory_entry(searchPath).exists() == false) {
        std::cout << "Error: Directory does not exist!\n";
        return -1;
    }

    // Call the appropriate function depending on the number of filenames
    if (filenames.size() == 1) {
        findFile(searchPath, filenames[0], recursive, caseInsensitive);
    } else if (filenames.size() > 1) {
        findFiles(searchPath, filenames, recursive, caseInsensitive);
    } else {
        std::cout << "Error: Not enough arguments.\n";
    }
    return 0;
}

