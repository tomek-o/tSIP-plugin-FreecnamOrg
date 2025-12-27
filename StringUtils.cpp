#include "StringUtils.h"
#include <sstream>
#include <stdio.h>

std::string UIntToString (unsigned int a) {
    std::ostringstream temp;
    temp<<a;
    return temp.str();
}

std::string IntToString (int a) {
    std::ostringstream temp;
    temp<<a;
    return temp.str();
}

int StrToUIntInRange(const std::string& str, unsigned int &ret, unsigned int min, unsigned int max) {
    unsigned int val;
    if (sscanf(str.c_str(), "%u", &val) != 1) {
        return -1;
    }
    if (val < min || val > max)
        return -2;
    ret = val;
    return 0;
}

int StrToUShortInRange(const std::string& str, unsigned short &ret, unsigned short min, unsigned short max) {
    unsigned int val;
    if (sscanf(str.c_str(), "%u", &val) != 1) {
        return -1;
    }
    if (val < min || val > max)
        return -2;
    ret = val;
    return 0;
}

int StrToUInt(const std::string& str, unsigned int &ret) {
    if (sscanf(str.c_str(), "%u", &ret) != 1) {
        return -1;
    }
    return 0;
}

int StrToUShort(const std::string& str, unsigned short &ret) {
    unsigned int val;
    if (sscanf(str.c_str(), "%u", &val) != 1) {
        return -1;
    }
    if (val > 0xFFFF)
        return -2;
    ret = val;
    return 0;
}

int StrToIntDef(const std::string& str, int defVal) {
    int ret = defVal;
    if (sscanf(str.c_str(), "%d", &ret) != 1) {
        return defVal;
    }
    return ret;
}
