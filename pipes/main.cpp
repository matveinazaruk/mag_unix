#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/wait.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iterator>

char ** toProcessArgs(std::vector<std::string> &args) {
    auto processArgs = new char *[args.size() + 1];
    for (int i = 0; i < args.size(); i++) {
        processArgs[i] = strdup(args[i].c_str());
    }
    processArgs[args.size()] = NULL;
    return processArgs;
}

void printUsage() {
    std::cout << "\nUsage: pipeline command1 | command2 | ... | commandN\n";
}

int main(int argc, char ** argv) {
    
    if (argc < 2) {
        std::cerr << "ARGUMENTS ERROR: Argument list can't be empty." << std::endl;
        printUsage();
        return 2;
    }

    int childrenCount = 1;

    std::vector<std::vector<std::string>> args;
    args.push_back(std::vector<std::string>());
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "|") == 0) {
            if (args[childrenCount - 1].size() == 0) {
                std::cerr << "ARGUMENTS ERROR: There sould be at least 1 command beetween pipes" << std::endl;
                printUsage();
                return 2;
            }
            childrenCount++;
            args.push_back(std::vector<std::string>());
        } else {
            std::string arg (argv[i]);
            args[childrenCount - 1].push_back(arg);
        }
    }

    if (args[childrenCount - 1].size() < 1) {
        std::cerr << "ARGUMENTS ERROR: There sould be at least 1 command after last pipe" << std::endl;
        printUsage();
        return 2;
    }

    auto pipefds = new int * [childrenCount - 1];
    for (int i = 0; i < childrenCount - 1; i++) {
        pipefds[i] = new int [2];
    }

    for (int i = 0; i < childrenCount - 1; ++i) {
        if (pipe(pipefds[i]) == -1) {
            perror("Pipe creating error");
            return 1;
        }
        if (fcntl(pipefds[i][1], F_SETPIPE_SZ, 100000000) == -1) {
            perror("Pipe fcntl error");
        }
    }

    pid_t * children = new pid_t [childrenCount];

    for (int i = 0; i < childrenCount; ++i) {
        pid_t pid = fork();

        if (pid == 0) {
            if (i != 0) {
                if (dup2(pipefds[i - 1][0], 0) == -1) {
                    perror("Duplicate standard input error");
                }
                if (close(pipefds[i - 1][1]) == -1) {
                    perror("Close pipe file descriptor for writing error.");
                }
            }
            if (i != childrenCount - 1) {
                if (dup2(pipefds[i][1], 1) == -1) {
                    perror("Duplicate standard output error");
                }
                if (close(pipefds[i][0]) == -1) {
                    perror("Close pipe file descriptor for reading error.");
                }
            }

            for (int j = i + 1; j < childrenCount - 1; ++j) {
                if (close(pipefds[j][0]) == -1) {
                    perror("Close pipe file descriptor for reading error.");
                }
                if (close(pipefds[j][1]) == -1) {
                    perror("Close pipe file descriptor for writing error.");
                }
            }

            auto processArgs = toProcessArgs(args[i]);
            if (execvp(args[i][0].c_str(), processArgs) == -1) {
                children[i] = -1;
                delete [] processArgs;
                std::cerr << "Error executing '";
                std::for_each(args[i].begin(), args[i].end(), [](std::string &arg) { std::cerr << arg << " ";});
                std::cerr << "'\n";
                perror("");
                break;
            }
        }

        if (i != 0) {
            if (close(pipefds[i - 1][1]) == -1) {
                perror("Close pipe file descriptor for writing error.");
            }
        }

        children[i] = pid;
    }

    int exitCode = 0;
    int status;
    for (int i = 0; i < childrenCount; ++i) {
        if (children[i] != -1) {
            waitpid(children[i], &status, 0);
        }
        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) != 0) {
                exitCode = 1;
            }
        }
    }

    for (int i = 0; i < childrenCount - 1; ++i) {
        close(pipefds[i][0]);
    }

    delete[] children;
    for (int i = 0; i < childrenCount - 1; i++) {
        delete pipefds[i];
    }

    delete[] pipefds;

    return exitCode;
}
