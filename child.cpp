#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <string>

ssize_t readLine(int fd, std::string &line) {
    line.clear();
    char buf[256];
    int bufPos = 0, bufLen = 0;
    while (true) {
        if (bufPos >= bufLen) {
            bufLen = read(fd, buf, sizeof(buf));
            bufPos = 0;
            if (bufLen <= 0) {
                if (line.empty()) return bufLen;
                else return (ssize_t)line.size();
            }
        }
        char c = buf[bufPos++];
        if (c == '\n') return (ssize_t)line.size();
        line += c;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        const char *err = "Usage: child <filename>\n";
        write(STDERR_FILENO, err, strlen(err));
        return 1;
    }
    const char *fileName = argv[1];
    int fd = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        const char *err = "open error\n";
        write(STDERR_FILENO, err, strlen(err));
        return 1;
    }
    std::string line;
    while (true) {
        ssize_t n = readLine(STDIN_FILENO, line);
        if (n < 0) {
            const char *err = "Read error in child\n";
            write(STDERR_FILENO, err, strlen(err));
            break;
        }
        if (n == 0 && line.empty()) break;
        long sum = 0;
        const char *ptr = line.c_str();
        char *end = nullptr;
        bool hasNumbers = false;

        while (*ptr != '\0') {
            while (*ptr == ' ' || *ptr == '\t') ptr++;
            if (*ptr == '\0' || *ptr == '\n') break;
            long val = strtol(ptr, &end, 10);  // парсим число
            if (end == ptr) {
                ptr++;
                continue;
            }
            sum += val;
            hasNumbers = true;
            ptr = end;
        }
        if (hasNumbers) {
            std::string result = std::to_string(sum) + "\n";
            write(fd, result.c_str(), result.size());
        }
    }

    close(fd);
    return 0;
}