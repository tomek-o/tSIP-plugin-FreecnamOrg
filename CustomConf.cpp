#include "CustomConf.h"
#include <json/json.h>

CustomConf customConf;

CustomConf::CustomConf(void):
    buttonId(15),
    numberLengthMin(10),
    numberLengthMax(10)
{
}

void CustomConf::toJson(Json::Value &jv) const
{
    jv = Json::Value(Json::objectValue);
    jv["buttonId"] = buttonId;
    jv["numberLengthMin"] = numberLengthMin;
    jv["numberLengthMax"] = numberLengthMax;
}

void CustomConf::fromJson(const Json::Value &jv)
{
    if (jv.type() != Json::objectValue)
        return;
    jv.getUInt("buttonId", buttonId);
    jv.getUInt("numberLengthMin", numberLengthMin);
    jv.getUInt("numberLengthMax", numberLengthMax);
}
