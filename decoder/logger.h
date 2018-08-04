// wujian@2018

#ifndef LOG_H
#define LOG_H

#include <sstream>
#include <cstring>
#include <iostream>
#include <ctime>


class Logger {
public:
    Logger(std::string type, const char *function, const char *file, size_t line): 
            type_(type), function_(function), 
            line_(line), file_(GetFileName(file)) {}
    
    ~Logger() {
        std::string info = oss_.str();
        while (!info.empty() && info[info.length() - 1] == '\n')
            info.resize(info.length() - 1);
        Log(info);
    }

    void Log(std::string msg) {
        std::ostringstream prefix;
        prefix << TimeNow() << " " << type_ << "(" 
            << function_ << "():" << file_ << ":" << line_ << ")";
        std::cerr << prefix.str().c_str() << " " << msg.c_str() << std::endl;
        if (type_ == "ASSERT" || type_ == "FAIL")
            abort();
    }

    std::ostream &Stream() { return oss_; }

private:
    std::ostringstream oss_;

    std::string type_;   // ERROR, INFO, WARN, ASSERT
    const char *function_;
    const char *file_;
    size_t line_;

    const char* GetFileName(const char *path) {
        int pos = strlen(path) - 1;
        while (pos >= 0) {
            if (path[pos] == '/') break;
            pos--;
        }
        return path + pos + 1;
    }

    const std::string TimeNow() {
        std::ostringstream time_format;
        time_t time_now = time(0);
        tm *tm = localtime(&time_now);
        time_format << 1900 + tm->tm_year << "/" 
                    << 1 + tm->tm_mon << "/" << tm->tm_mday
                    << " " << tm->tm_hour << ":" << tm->tm_min
                    << ":" << tm->tm_sec;
        return time_format.str();
    }
};


#define LOG_WARN   Logger("WARN", __FUNCTION__, __FILE__, __LINE__).Stream()
#define LOG_INFO   Logger("INFO", __FUNCTION__, __FILE__, __LINE__).Stream()
#define LOG_FAIL   Logger("FAIL", __FUNCTION__, __FILE__, __LINE__).Stream()

#define ASSERT(cond) do { if (cond) (void)0; else \
    Logger("ASSERT", __FUNCTION__, __FILE__, __LINE__).Stream() \
    << "Assert '" << #cond << "' failed!"; } while(0)


#endif