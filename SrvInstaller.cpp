#include <windows.h>
#include <string>
#include <memory>
#include "SrvInstaller.h"

using namespace std;

extern "C" {

	LIBRARY_API int ServiceQueryStateByName(const STRING_ENCODING* srvName, DWORD& scState)
	{
		BOOL opResult;
		DWORD dwResult;
		SERVICE_STATUS scStatus;

		unique_sch scManager(OpenSCManager(0, 0, SC_MANAGER_CONNECT));

		if (scManager.get())
		{
#ifdef UNICODE
			unique_sch scHandle(::OpenServiceW(scManager.get(), srvName, SERVICE_ALL_ACCESS));
#else
			unique_sch scHandle(::OpenServiceA(scManager.get(), srvName, SERVICE_ALL_ACCESS));
#endif
			if (scHandle.get())
			{
				opResult = QueryServiceStatus(scHandle.get(), &scStatus);
				if (opResult == ERROR_ACCESS_DENIED)
					return INSTALLER_ERROR_ACCESS_DENIED;
				else if (opResult == ERROR_INVALID_HANDLE)
					return INSTALLER_ERROR_INVALID_HANDLE;

				scState = scStatus.dwCurrentState;
				return INSTALLER_OP_SUCCESS;
			}
			else
			{
				dwResult = GetLastError();
				if (dwResult == ERROR_ACCESS_DENIED)
					return INSTALLER_ERROR_ACCESS_DENIED;
				else if (dwResult == ERROR_INVALID_HANDLE)
					return INSTALLER_ERROR_INVALID_HANDLE;
				else if (dwResult == ERROR_INVALID_NAME)
					return INSTALLER_ERROR_INVALID_NAME;
				else if (dwResult == ERROR_SERVICE_DOES_NOT_EXIST)
					return INSTALLER_ERROR_SERVICE_DOES_NOT_EXIST;
				else
					return INSTALLER_ERROR_OPEN_SERVICE;
			}
		}
		else
		{
			if (GetLastError() == ERROR_ACCESS_DENIED)
				return INSTALLER_ERROR_ACCESS_DENIED;
			else
				return INSTALLER_ERROR_OPEN_SCMANAGER;
		}		
	}

	DLL_IMPORT int ServiceQueryState(unique_sch& scHandlePtr, DWORD& scState)
	{
		BOOL opResult;
		SERVICE_STATUS scStatus;

		opResult = QueryServiceStatus(scHandlePtr.get(), &scStatus);
		if (opResult == ERROR_ACCESS_DENIED)
			return INSTALLER_ERROR_ACCESS_DENIED;
		else if (opResult == ERROR_INVALID_HANDLE)
			return INSTALLER_ERROR_INVALID_HANDLE;

		scState = scStatus.dwCurrentState;
		return INSTALLER_OP_SUCCESS;
	}

	DLL_IMPORT int ServiceQueryStatus(unique_sch& scHandlePtr, SERVICE_STATUS& scStatus)
	{
		if (!QueryServiceStatus(scHandlePtr.get(), &scStatus))
			return INSTALLER_ERROR_QUERY_SERVICE;

		return INSTALLER_OP_SUCCESS;
	}

	DLL_IMPORT int ServiceWaitForState(unique_sch& scHandlePtr, DWORD pendingState, DWORD finalState)
	{
		int opResult;
		DWORD timeWait, timeWithoutProgress, oldCheckPoint, startTickCount;
		SERVICE_STATUS scStatus;

		// If a service is not in pending (or in final) state, don't return immediately
		bool isHandleWrongState = true;

		{
			// Service status
			opResult = ServiceQueryStatus(scHandlePtr, scStatus);
			if (opResult != INSTALLER_OP_SUCCESS)
				return opResult;

			startTickCount = GetTickCount();

			// Used to track the progress of the service during a lengthy operation
			oldCheckPoint = scStatus.dwCheckPoint;
		}

		while (scStatus.dwCurrentState == pendingState)
		{
			// Do not wait longer than the wait hint. A good interval is 
			// one-tenth the wait hint, but no less than 1 second and no 
			// more than 10 seconds. 
			timeWait = scStatus.dwWaitHint / 10;
			timeWait = timeWait < 1000 ? 1000 : timeWait > 10000 ? 10000 : timeWait;
			Sleep(timeWait);

			// Service status
			opResult = ServiceQueryStatus(scHandlePtr, scStatus);
			if (opResult != INSTALLER_OP_SUCCESS)
				return opResult;

			// If a service already in desired state, just return
			if (scStatus.dwCurrentState == finalState)
				return INSTALLER_OP_SUCCESS;

			// Handle service's wrong state
			if (scStatus.dwCurrentState != pendingState)
			{
				if (isHandleWrongState)
				{
					// Only one attempt a service to move to pending state
					isHandleWrongState = false;

					startTickCount = GetTickCount();
					oldCheckPoint = scStatus.dwCheckPoint;

					continue;
				}
				else
					return INSTALLER_ERROR_INVALID_STATE;
			}

			// If service shows some progress ...
			if (scStatus.dwCheckPoint > oldCheckPoint)
			{
				// ... continue to wait and check
				startTickCount = GetTickCount();
				oldCheckPoint = scStatus.dwCheckPoint;
			}
			else
			{
				// No progress made within the wait hint
				timeWithoutProgress = GetTickCount() - startTickCount;
				if (timeWithoutProgress > scStatus.dwWaitHint)
					break;
			}
		}

		if (scStatus.dwCurrentState != finalState)
			return INSTALLER_ERROR_TRANSITION_FAILURE;
		else
			return INSTALLER_OP_SUCCESS;
	}
	
	LIBRARY_API bool ServiceExists(const STRING_ENCODING* srvName)
	{
		return ServiceTryOpen(srvName) == INSTALLER_OP_SUCCESS;
	}

	DLL_IMPORT int ServiceTryOpen(const STRING_ENCODING* srvName)
	{
		DWORD opResult;

		unique_sch scManager(OpenSCManager(0, 0, SC_MANAGER_CONNECT));

		if (scManager.get())
		{
#ifdef UNICODE
			unique_sch scHandle(::OpenServiceW(scManager.get(), srvName, SERVICE_ALL_ACCESS));
#else
			unique_sch scHandle(::OpenServiceA(scManager.get(), srvName, SERVICE_ALL_ACCESS));
#endif
			if (scHandle.get())
				return INSTALLER_OP_SUCCESS;
			else
			{
				opResult = GetLastError();
				if (opResult == ERROR_ACCESS_DENIED)
					return INSTALLER_ERROR_ACCESS_DENIED;
				else if (opResult == ERROR_INVALID_HANDLE)
					return INSTALLER_ERROR_INVALID_HANDLE;
				else if (opResult == ERROR_INVALID_NAME)
					return INSTALLER_ERROR_INVALID_NAME;
				else if (opResult == ERROR_SERVICE_DOES_NOT_EXIST)
					return INSTALLER_ERROR_SERVICE_DOES_NOT_EXIST;
				else
					return INSTALLER_ERROR_OPEN_SERVICE;
			}
		}
		else
		{
			if (GetLastError() == ERROR_ACCESS_DENIED)
				return INSTALLER_ERROR_ACCESS_DENIED;
			else
				return INSTALLER_ERROR_OPEN_SCMANAGER;
		}
	}

	LIBRARY_API int ServiceInstall(const STRING_ENCODING* srvName, const STRING_ENCODING* srvFullName, const STRING_ENCODING* srvPath)
	{
		if (ServiceTryOpen(srvName) == INSTALLER_OP_SUCCESS)
			return INSTALLER_OP_SUCCESS;

		unique_sch scManager(OpenSCManager(0, 0, SC_MANAGER_CREATE_SERVICE));

		if (scManager.get())
		{
#ifdef UNICODE
			unique_sch scHandle(CreateServiceW(
				scManager.get(),				// A handle to the service control manager database
				srvName,						// The name of the service to install
				srvFullName,					// The display name to identify the service
				SERVICE_ALL_ACCESS,				// The access to the service
				SERVICE_WIN32_OWN_PROCESS,		// The service type: service that runs in its own process
				SERVICE_AUTO_START,				// The service start options
				SERVICE_ERROR_NORMAL,			// The severity of the error: logs the error and continues the startup
				srvPath,						// The fully qualified path to the service binary file
				0,								// No load ordering group
				0,								// No tag value changing
				0,								// No service dependencies supported
				L"NT AUTHORITY\\LocalService", 	// Give a service a broader permissions
				0								// No password is used
				));
#else
			unique_sch scHandle(CreateServiceA(
				scManager.get(),				// A handle to the service control manager database
				srvName,						// The name of the service to install
				srvFullName,					// The display name to identify the service
				SERVICE_ALL_ACCESS,				// The access to the service
				SERVICE_WIN32_OWN_PROCESS,		// The service type: service that runs in its own process
				SERVICE_AUTO_START,				// The service start options
				SERVICE_ERROR_NORMAL,			// The severity of the error: logs the error and continues the startup
				srvPath,						// The fully qualified path to the service binary file
				0,								// No load ordering group
				0,								// No tag value changing
				0,								// No service dependencies supported
				"NT AUTHORITY\\LocalService", 	// Give a service a broader permissions
				0								// No password is used
				));
#endif
			if (!scHandle.get())
				return INSTALLER_ERROR_CREATE_SERVICE;

			return INSTALLER_OP_SUCCESS;
		}
		else
		{
			if (GetLastError() == ERROR_ACCESS_DENIED)
				return INSTALLER_ERROR_ACCESS_DENIED;
			else
				return INSTALLER_ERROR_OPEN_SCMANAGER;
		}
	}

	LIBRARY_API int ServiceUninstall(const STRING_ENCODING* srvName)
	{
		BOOL opResult;

		unique_sch scManager(OpenSCManager(0, 0, SC_MANAGER_CONNECT));

		if (scManager.get())
		{
#ifdef UNICODE
			unique_sch scHandle(OpenServiceW(scManager.get(), srvName, SERVICE_ALL_ACCESS));
#else
			unique_sch scHandle(OpenServiceA(scManager.get(), srvName, SERVICE_ALL_ACCESS));
#endif
			if (scHandle.get())
			{
				opResult = DeleteService(scHandle.get());

				if (opResult == ERROR_SERVICE_MARKED_FOR_DELETE)
					return INSTALLER_OP_SUCCESS;
				else if (opResult == ERROR_INVALID_HANDLE)
					return INSTALLER_ERROR_INVALID_HANDLE;
				else if (opResult == ERROR_ACCESS_DENIED)
					return INSTALLER_ERROR_ACCESS_DENIED;
				else
					return INSTALLER_ERROR_DELETE_SERVICE;
			}
			else
				return INSTALLER_ERROR_OPEN_SERVICE;
		}
		else
		{
			if (GetLastError() == ERROR_ACCESS_DENIED)
				return INSTALLER_ERROR_ACCESS_DENIED;
			else
				return INSTALLER_ERROR_OPEN_SCMANAGER;
		}
	}

	LIBRARY_API int ServiceStart(const STRING_ENCODING* srvName)
	{
		DWORD opResult, scState;

		unique_sch scManager(OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS));

		if (scManager.get())
		{
#ifdef UNICODE
			unique_sch scHandle(OpenServiceW(scManager.get(), srvName, SERVICE_ALL_ACCESS));
#else
			unique_sch scHandle(OpenServiceA(scManager.get(), srvName, SERVICE_ALL_ACCESS));
#endif
			if (!scHandle.get())
				return INSTALLER_ERROR_OPEN_SERVICE;

			// Service status
			opResult = ServiceQueryState(scHandle, scState);
			if (opResult != INSTALLER_OP_SUCCESS)
				return opResult;

			// If service is running ...
			if ((scState == SERVICE_START_PENDING) ||
				(scState == SERVICE_RUNNING))
				return INSTALLER_OP_SUCCESS;

			// Else, try to start it
			if (!StartService(scHandle.get(), 0, NULL))
				return INSTALLER_ERROR_START_SERVICE;

			// Try to wait for the desired state
			opResult = ServiceWaitForState(scHandle, SERVICE_START_PENDING, SERVICE_RUNNING);
			if (opResult != INSTALLER_OP_SUCCESS)
				return opResult;

			// Service status
			opResult = ServiceQueryState(scHandle, scState);
			if (opResult != INSTALLER_OP_SUCCESS)
				return opResult;

			if (scState != SERVICE_RUNNING)
				return INSTALLER_ERROR_START_SERVICE;

			return INSTALLER_OP_SUCCESS;
		}
		else
		{
			if (GetLastError() == ERROR_ACCESS_DENIED)
				return INSTALLER_ERROR_ACCESS_DENIED;
			else
				return INSTALLER_ERROR_OPEN_SCMANAGER;
		}
	}

	DLL_IMPORT int ServiceMoveToState(const STRING_ENCODING* srvName, DWORD actionForState, DWORD pendingState, DWORD finalState)
	{
		DWORD opResult, scState;
		DWORD errAlias;
		SERVICE_STATUS scStatus;

		unique_sch scManager(OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS));

		if (scManager.get())
		{
#ifdef UNICODE
			unique_sch scHandle(OpenServiceW(scManager.get(), srvName, SERVICE_ALL_ACCESS));
#else
			unique_sch scHandle(OpenServiceA(scManager.get(), srvName, SERVICE_ALL_ACCESS));
#endif
			if (!scHandle.get())
				return INSTALLER_ERROR_OPEN_SERVICE;

			// Service status
			opResult = ServiceQueryState(scHandle, scState);
			if (opResult != INSTALLER_OP_SUCCESS)
				return opResult;

			// If service is pending ... 
			if ((scState == pendingState) ||
				(scState == finalState))
				return INSTALLER_OP_SUCCESS;

			if (actionForState != SERVICE_CONTROL_STOP &&
				actionForState != SERVICE_CONTROL_PAUSE &&
				actionForState != SERVICE_CONTROL_CONTINUE)
				return INSTALLER_ERROR_WRONG_ACTION;

			errAlias =
				actionForState == SERVICE_CONTROL_STOP ? INSTALLER_ERROR_STOP_SERVICE :
				actionForState == SERVICE_CONTROL_PAUSE ? INSTALLER_ERROR_PAUSE_SERVICE : INSTALLER_ERROR_CONTINUE_SERVICE;

			// Else try to perform the action
			if (!ControlService(scHandle.get(), actionForState, (LPSERVICE_STATUS)&scStatus))
				return errAlias;

			// Wait for the final state
			opResult = ServiceWaitForState(scHandle, pendingState, finalState);
			if (opResult != INSTALLER_OP_SUCCESS)
				return opResult;

			// Service status
			opResult = ServiceQueryState(scHandle, scState);
			if (opResult != INSTALLER_OP_SUCCESS)
				return opResult;

			if (scState != finalState)
				return errAlias;

			return INSTALLER_OP_SUCCESS;
		}
		else
		{
			if (GetLastError() == ERROR_ACCESS_DENIED)
				return INSTALLER_ERROR_ACCESS_DENIED;
			else
				return INSTALLER_ERROR_OPEN_SCMANAGER;
		}
	}

	LIBRARY_API int ServiceStop(const STRING_ENCODING* srvName)
	{
		return ServiceMoveToState(srvName, SERVICE_CONTROL_STOP, SERVICE_STOP_PENDING, SERVICE_STOPPED);
	}

	LIBRARY_API int ServicePause(const STRING_ENCODING* srvName)
	{
		return ServiceMoveToState(srvName, SERVICE_CONTROL_PAUSE, SERVICE_PAUSE_PENDING, SERVICE_PAUSED);
	}

	LIBRARY_API int ServiceResume(const STRING_ENCODING* srvName)
	{
		return ServiceMoveToState(srvName, SERVICE_CONTROL_CONTINUE, SERVICE_CONTINUE_PENDING, SERVICE_RUNNING);
	}
}
