#define _EXPORTING
#include "..\tSIP\tSIP\phone\Phone.h"
#include "..\tSIP\tSIP\phone\PhoneSettings.h"
#include "..\tSIP\tSIP\phone\PhoneCapabilities.h"

#include "main.h"
#include "naett.h"
#include "Log.h"
#include "CustomConf.h"
#include "Utils.h"
#include "StringUtils.h"
#include "Mutex.h"
#include "ScopedLock.h"
#include <winsock.h>
#include <time.h>
#include <assert.h>
#include <map>
#include <fstream>
#include <json/json.h>

namespace {

Mutex mutex;

struct CacheEntry
{
    std::string number;
    std::string description;
    HANDLE threadHandle;
    CacheEntry(void):
        threadHandle(NULL)
    {
    }
};
std::map<std::string, CacheEntry> cache;

const struct S_PHONE_DLL_INTERFACE dll_interface =
{DLL_INTERFACE_MAJOR_VERSION, DLL_INTERFACE_MINOR_VERSION};

// callback ptrs
CALLBACK_LOG lpLogFn = NULL;
CALLBACK_CONNECT lpConnectFn = NULL;
CALLBACK_KEY lpKeyFn = NULL;
CALLBACK_RUN_SCRIPT_ASYNC lpRunScriptAsyncFn = NULL;

void *callbackCookie;	///< used by upper class to distinguish library instances when receiving callbacks

int RunScriptAsync(const char* script) {
	if (lpRunScriptAsyncFn) {
		return lpRunScriptAsyncFn(callbackCookie, script);
	}
	return -1;
}

std::string LuaStringEscape(const std::string& input)
{
    std::string out;

    for (char c : input)
    {
        switch (c)
        {
            case '\\': out += "\\\\"; break;
            case '"':  out += "\\\""; break;
            case '\'': out += "\\\'"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            case '\0': out += "\\0";  break;
            default:
                out += c;
        }
    }
    return out;
}

void SetDescription(const std::string &text) {
    std::string script = std::string("SetButtonCaption(") +
        UIntToString(customConf.buttonId) + ", '" + LuaStringEscape(text) + "')";
    //LOG("script = %s", script.c_str());
    RunScriptAsync(script.c_str());
}


}


DWORD WINAPI ThreadProc(LPVOID data)
{
	CacheEntry *entry = reinterpret_cast<CacheEntry*>(data);

	std::string URL = std::string("https://freecnam.org/dip?q=") + entry->number;
    naettReq* req = naettRequest(URL.c_str(), naettMethod("GET"), naettHeader("accept", "*/*"));
    naettRes* res = naettMake(req);

    while (!naettComplete(res)) {
        Sleep(50);
    }

    int status = naettGetStatus(res);

    if (status < 0) {
        LOG("Request failed: %d", status);
        SetDescription("");
        entry->threadHandle = NULL;
        return 1;
    }

    int bodyLength = 0;
    const char* body = reinterpret_cast<const char*>(naettGetBody(res, &bodyLength));
    LOG("Got a %d, %d bytes of type '%s':", naettGetStatus(res), bodyLength, naettGetHeader(res, "Content-Type"));

    int httpStatus = naettGetStatus(res);

    entry->description = std::string(body, bodyLength);
    LOG("%s", entry->description.c_str());
    SetDescription(entry->description);

    naettClose(res);
    naettFree(req);

    entry->threadHandle = NULL;

    if (httpStatus != 200) {
        cache.erase(entry->number);
    }

    return 0;
}

/** get handle to dll without knowing its name
*/
HMODULE GetCurrentModule()
{
    MEMORY_BASIC_INFORMATION mbi;
    static int dummy;
    VirtualQuery( &dummy, &mbi, sizeof(mbi) );
    return reinterpret_cast<HMODULE>(mbi.AllocationBase);
}

void GetPhoneInterfaceDescription(struct S_PHONE_DLL_INTERFACE* interf) {
    interf->majorVersion = dll_interface.majorVersion;
    interf->minorVersion = dll_interface.minorVersion;
}

void Log(const char* txt) {
    if (lpLogFn)
        lpLogFn(callbackCookie, const_cast<char*>(txt));
}

void SetCallbacks(void *cookie, CALLBACK_LOG lpLog, CALLBACK_CONNECT lpConnect, CALLBACK_KEY lpKey) {
    assert(cookie && lpLog && lpConnect && lpKey);
    lpLogFn = lpLog;
    lpConnectFn = lpConnect;
    lpKeyFn = lpKey;
    callbackCookie = cookie;

    Log("FreecnamOrg plugin loaded\n");
}

void GetPhoneCapabilities(struct S_PHONE_CAPABILITIES **caps) {
    static struct S_PHONE_CAPABILITIES capabilities = {
        0
    };
    *caps = &capabilities;
}

void ShowSettings(HANDLE parent) {
    MessageBox((HWND)parent, "No additional settings in the GUI - see cfg file.", "Device DLL", MB_ICONINFORMATION);
}

int Connect(void)
{
    naettInit(NULL);

    return 0;
}

int Disconnect(void)
{
    for (std::map<std::string, CacheEntry>::iterator iter = cache.begin(); iter != cache.end(); ++iter)
        TerminateThread(iter->second.threadHandle, 0);
    cache.clear();
    return 0;
}

static int GetDefaultSettings(struct S_PHONE_SETTINGS* settings) {
    settings->ring = 0;
    return 0;
}

int GetPhoneSettings(struct S_PHONE_SETTINGS* settings) {
    std::string path = Utils::GetDllPath();
    path = Utils::ReplaceFileExtension(path, ".cfg");
    if (path == "")
        return GetDefaultSettings(settings);

    Json::Value root;   // will contains the root value after parsing.
    Json::Reader reader;

    std::ifstream ifs(path.c_str());
    std::string strConfig((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ifs.close();

    bool parsingSuccessful = reader.parse( strConfig, root );
    if ( !parsingSuccessful )
        return GetDefaultSettings(settings);

    GetDefaultSettings(settings);

    customConf.fromJson(root["customConf"]);

    return 0;
}

int SavePhoneSettings(struct S_PHONE_SETTINGS* settings) {
    Json::Value root;
    Json::StyledWriter writer;

    customConf.toJson(root["customConf"]);

    std::string outputConfig = writer.write( root );

    std::string path = Utils::GetDllPath();
    path = Utils::ReplaceFileExtension(path, ".cfg");
    if (path == "")
        return -1;

    std::ofstream ofs(path.c_str());
    ofs << outputConfig;
    ofs.close();

    return 0;
}

void SetRunScriptAsyncCallback(CALLBACK_RUN_SCRIPT_ASYNC lpRunScriptAsync) {
	lpRunScriptAsyncFn = lpRunScriptAsync;
}

int SetCallState(int state, const char* display) {
    if (state && display) {
        std::string number;
        while (*display) {
            const char c = *display;
            display++;
            if (c == '@')
                break;
            if (c >= '0' && c <= '9')
                number += c;
        }
        if (number.length() >= customConf.numberLengthMin && number.length() <= customConf.numberLengthMax) {
            std::string cachedDescription;
            {
                ScopedLock<Mutex> lock(mutex);
                std::map<std::string, CacheEntry>::iterator iter = cache.find(number);
                if (iter != cache.end()) {
                    SetDescription(iter->second.description);
                } else {
                    DWORD dwtid;
                    CacheEntry &entry = cache[number];
                    entry.number = number;
                    entry.threadHandle = CreateThread(NULL, 0, ThreadProc, &entry, 0, &dwtid);
                    SetDescription("");
                }
            }
            return 0;
        }
    }
    SetDescription("");
    return 0;
}
