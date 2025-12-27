/** \file
 *  \brief Global logging unit
 */

#include <stdio.h>
#include <iostream>
#include <fstream>

#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <sys\timeb.h>
#include "Log.h"
#include "Utils.h"
#include <string>

extern void Log(const char* txt);

namespace
{
    const std::string PREFIX = Utils::ExtractFileName(Utils::GetDllPath()) + ": ";
}

CLog::CLog()
{
};

void CLog::log(const char *lpData, ...)
{
	va_list ap;
	char buf[1024]; //determines max message length

    int size = 0;

    size += snprintf(buf + size, sizeof(buf) - size, "%s", PREFIX.c_str());

	if ((int)sizeof(buf)-size-2 > 0)
	{
		va_start(ap, lpData);
		size += vsnprintf(buf + size, sizeof(buf)-size-2, lpData, ap);
		va_end(ap);
	}
	if (size > (int)sizeof(buf) - 2)
		size = sizeof(buf) - 2;
	buf[size] = '\n';
	buf[size+1] = 0;

	Log(buf);
}

