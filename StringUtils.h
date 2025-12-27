#ifndef StringUtilsH
#define StringUtilsH

#include <string>

std::string UIntToString (unsigned int a);
std::string IntToString (int a);
int StrToUIntInRange(const std::string& str, unsigned int &ret, unsigned int min, unsigned int max);
int StrToUShortInRange(const std::string& str, unsigned short &ret, unsigned short min, unsigned short max);
int StrToUInt(const std::string& str, unsigned int &ret);
int StrToUShort(const std::string& str, unsigned short &ret);
int StrToIntDef(const std::string& str, int defVal);

#endif
