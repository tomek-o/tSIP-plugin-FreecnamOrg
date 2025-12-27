#ifndef CustomConfH
#define CustomConfH

#include <string>

namespace Json
{
    class Value;
}

struct CustomConf
{
    unsigned int buttonId;
    unsigned int numberLengthMin;
    unsigned int numberLengthMax;
    CustomConf(void);
    void toJson(Json::Value &jv) const;
    void fromJson(const Json::Value &jv);
};

extern CustomConf customConf;

#endif // CustomConfH
