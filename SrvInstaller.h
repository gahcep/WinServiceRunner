/*
	Installer for windows services.
	Not supported:
		* service chain (dependency)
		* service args
*/

#pragma once

#ifndef SRVINSTALLER_H
#define SRVINSTALLER_H

/* LIBRARY_API is defined in "Project Settings" -> "C/C++" -> "Preprocessor" -> "Preprocessor definitions" */
#ifdef LIBRARY_API
#    define DLL_IMPORT
#    define LIBRARY_API __declspec(dllexport)
#else
#    define DLL_IMPORT __declspec(dllimport)
#    define LIBRARY_API __declspec(dllimport)
#endif

/* Support for both ANSI and UNICODE encoding 
 * Set if use in C# via PInvoke (C#'s System.String is Unicode-based)
*/
#define UNICODE
#ifdef UNICODE
#    define STRING_ENCODING wchar_t
#else
#    define STRING_ENCODING char
#endif

#include <windows.h>
#include <string>
#include <memory>

#pragma comment(lib, "advapi32.lib")

using namespace std;

#define INSTALLER_OP_SUCCESS				    0
#define INSTALLER_ERROR_OPEN_SCMANAGER		    0x100
#define INSTALLER_ERROR_CREATE_SERVICE		    0x101
#define INSTALLER_ERROR_OPEN_SERVICE		    0x102
#define INSTALLER_ERROR_QUERY_SERVICE		    0x103
#define INSTALLER_ERROR_START_SERVICE           0x104
#define INSTALLER_ERROR_STOP_SERVICE            0x105
#define INSTALLER_ERROR_PAUSE_SERVICE           0x106
#define INSTALLER_ERROR_CONTINUE_SERVICE        0x107
#define INSTALLER_ERROR_INVALID_STATE           0x108
#define INSTALLER_ERROR_TRANSITION_FAILURE      0x109
#define INSTALLER_ERROR_WRONG_ACTION            0x110
#define INSTALLER_ERROR_INVALID_HANDLE          0x111
#define INSTALLER_ERROR_ACCESS_DENIED           0x112
#define INSTALLER_ERROR_DELETE_SERVICE          0x113
#define INSTALLER_ERROR_INVALID_NAME            0x114
#define INSTALLER_ERROR_SERVICE_DOES_NOT_EXIST  0x115

/* Custom deleter for std::unique_ptr<SC_HANDLE> */
class SCDeleter
{
	// Redefine inner type via 'pointer' field
	typedef SC_HANDLE pointer;

public: 
	
	SCDeleter() {};

	void operator()(pointer handle) const
	{
		CloseServiceHandle(handle);
	}
};

typedef unique_ptr<SC_HANDLE, SCDeleter> unique_sch;

extern "C" {

	LIBRARY_API int ServiceQueryStateByName(const STRING_ENCODING* srvName, DWORD& scState);
	DLL_IMPORT int ServiceQueryState(unique_sch& scHandlePtr, DWORD& scState);
	DLL_IMPORT int ServiceQueryStatus(unique_sch& scHandlePtr, SERVICE_STATUS& scStatus);
	DLL_IMPORT int ServiceWaitForState(unique_sch& scHandlePtr, DWORD pendingState, DWORD finalState);
	LIBRARY_API bool ServiceExists(const STRING_ENCODING* srvName);
	DLL_IMPORT int ServiceTryOpen(const STRING_ENCODING* srvName);
	LIBRARY_API int ServiceInstall(const STRING_ENCODING* srvName, const STRING_ENCODING* srvFullName, const STRING_ENCODING* srvPath);
	LIBRARY_API int ServiceUninstall(const STRING_ENCODING* srvName);
	LIBRARY_API int ServiceStart(const STRING_ENCODING* srvName);
	DLL_IMPORT int ServiceMoveToState(const STRING_ENCODING* srvName, DWORD actionForState, DWORD pendingState, DWORD finalState);
	LIBRARY_API int ServiceStop(const STRING_ENCODING* serviceName);
	LIBRARY_API int ServicePause(const STRING_ENCODING* serviceName);
	LIBRARY_API int ServiceResume(const STRING_ENCODING* serviceName);

}
#endif