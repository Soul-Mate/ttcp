#ifndef TTCP_COMMON_H
#define TTCP_COMMON_H
#include <iostream>
#include <utility>
#include <cstring>
#include <cassert>
#include <string>

class nocopyable
{
protected:
    nocopyable() = default;
    ~nocopyable() = default;
private:
    nocopyable(const nocopyable&) = delete;
    nocopyable& operator=(const nocopyable&) = delete;
};

struct copyable
{

};

class StringArg: copyable
{
public:
    StringArg() : __arg(nullptr) {};

    explicit StringArg(const char* arg) {
        __arg = new char[ std::strlen(arg) + 1];
        std::strcpy(__arg, arg);
    }

    ~StringArg() {
        if (__arg == nullptr) {
            return;
        }
        delete []__arg;
    }

    StringArg(const StringArg& that) {
        __arg = new char[ std::strlen(that.__arg) + 1];
        std::strcpy(__arg, that.__arg);
    }

    StringArg& operator=(const StringArg& that) {
        if (this != &that) {
            delete []__arg;
            __arg = new char[ std::strlen(that.__arg) + 1];
            std::strcpy(__arg, that.__arg);
        } 

        return *this;
    }

    StringArg& operator=(const char* arg) {
        if (__arg == nullptr) {
            __arg = new char[ std::strlen(arg) + 1];
            std::strcpy(__arg, arg); 
            return *this;
        }

        if (strcmp(arg, __arg) == 0) {
            return *this;
        }

        delete []__arg;
        __arg = new char[ std::strlen(arg) + 1];
        std::strcpy(__arg, arg); 
        return *this;
    }

    StringArg(StringArg&& that) {
        __arg = that.__arg;
    }

    StringArg& operator=(StringArg&& that) {
        if (&that != this) {
            __arg = that.__arg;
        }
        
        return *this;
    }

    inline bool empty() const { return __arg[0] == '\0'? true : false; };

    const char* get_c_str() const { return __arg; };

    int toi() { return atoi(__arg); };

private:
    char* __arg;
};

// std::ostream& operator<< (std::ostream& os, const StringArg& s) {
//     return os << s.get_c_str();
// }

#endif /*TTCP_COMMON_H*/