#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <algorithm>

char ** toProcessArgs(std::vector<std::string> &args) {
    auto processArgs = new char *[args.size() + 1];
    for (int i = 0; i < args.size(); i++) {
        processArgs[i] = strdup(args[i].c_str());
    }
    processArgs[args.size()] = NULL;
    return processArgs;
}

int main(int argc, char ** argv) {

    int childrenCount = 1;

    std::vector<std::vector<std::string>> args;
    args.push_back(std::vector<std::string>());
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "|") == 0) {
            args.push_back(std::vector<std::string>());
            childrenCount++;
        } else {
            std::string arg (argv[i]);
            args[childrenCount - 1].push_back(arg);
        }
    }

    int pipefds[childrenCount - 1][2];
    for (int i = 0; i < childrenCount - 1; ++i) {
        if (pipe(pipefds[i]) == -1) {
            perror("pipe");
            return 1;
        }
    }
    pid_t * children = new pid_t [childrenCount];

    for (int i = 0; i < childrenCount; ++i) {
        pid_t pid = fork();

        if (pid == 0) {
            if (i != 0) {
                dup2(pipefds[i - 1][0], 0);
                close(pipefds[i - 1][1]);
            }
            if (i != childrenCount - 1) {
                dup2(pipefds[i][1], 1);
                close(pipefds[i][0]);
            }

            for (int j = 0; j < childrenCount - 1; ++j) {
                if (j != i - 1 && j != i) {
                    close(pipefds[j][0]);
                    close(pipefds[j][1]);
                }
            }

            auto processArgs = toProcessArgs(args[i]);
            execvp(args[i][0].c_str(), processArgs);
        }

        if (i != 0) {
            close(pipefds[i - 1][1]);
        }

        children[i] = pid;
    }

    int exitCode = 0;
    int status;
    for (int i = 0; i < childrenCount; ++i) {
        waitpid(children[i], &status, 0);
        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) != 0) {
                exitCode = 1;
            }
        }
    }

    for (int i = 0; i < childrenCount - 1; ++i) {
        close(pipefds[i][0]);
    }
    delete children;
    return exitCode;
}
