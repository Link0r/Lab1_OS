#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

ssize_t readLine(int fd, std::string &line) {
    line.clear();
    char buf[256];
    int bufPos = 0;
    int bufLen = 0;

    while (true) {
        if (bufPos >= bufLen) {
            bufLen = read(fd, buf, sizeof(buf));
            bufPos = 0;
            if (bufLen <= 0) {
                if (line.empty()) return bufLen;
                else return (ssize_t)line.size();5
            }
        }
        char c = buf[bufPos++];
        if (c == '\n') return (ssize_t)line.size();
        line += c;
    }
}

int main() {
    const char *prompt1 = "Enter file name: ";
    write(STDOUT_FILENO, prompt1, strlen(prompt1));

    std::string fileName;
    ssize_t n = readLine(STDIN_FILENO, fileName);
    if (n <= 0) {
        const char *err = "Error: cannot read file name\n";
        write(STDERR_FILENO, err, strlen(err));
        return 1;
    }

    int pipe1[2];
    if (pipe(pipe1) < 0) {
        const char *err = "pipe1 error\n";
        write(STDERR_FILENO, err, strlen(err));
        return 1;
    }

    int pipe2[2];
    pipe(pipe2);

    pid_t pid = fork();
    if (pid < 0) {
        const char *err = "fork error\n";
        write(STDERR_FILENO, err, strlen(err));
        return 1;
    }

    if (pid == 0) {
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);
        dup2(pipe1[0], STDIN_FILENO);
        close(pipe1[0]);
        char *argv[] = { (char*)"./child", (char*)fileName.c_str(), nullptr };
        char *envp[] = { nullptr };
        execve("./child", argv, envp);
        const char *err = "execve error: child not found\n";
        write(STDERR_FILENO, err, strlen(err));
        _exit(1);
    }

    close(pipe1[0]);
    close(pipe2[0]);
    close(pipe2[1]);

    const char *prompt2 = "Enter numbers (empty line to finish):\n";
    write(STDOUT_FILENO, prompt2, strlen(prompt2));

    std::string line;
    while (true) {
        const char *prompt3 = "> ";
        write(STDOUT_FILENO, prompt3, strlen(prompt3));
        n = readLine(STDIN_FILENO, line);
        if (n < 0) {
            const char *err = "Read error\n";
            write(STDERR_FILENO, err, strlen(err));
            break;
        }
        if (n == 0 && line.empty()) break;
        if (line.empty()) break;
        line += '\n';
        write(pipe1[1], line.c_str(), line.size());
    }
    close(pipe1[1]);
    int status;
    waitpid(pid, &status, 0);
    const char *done = "Child finished.\n";
    write(STDOUT_FILENO, done, strlen(done));
    return 0;
}