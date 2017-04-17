#include "DirectoryChangeMonitor.h"

#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#define  ISOLATE_PATH_A   "A:\\SafeDogRecycle\\"
#define  ISOLATE_PATH_W   L"A:\\SafeDogRecycle\\"

CDirectoryChangeMonitor::CDirectoryChangeMonitor(DWORD dwMonitorType, bool bAppHasGUI, DWORD dwFilterFlags)
: m_DirctoryChangeWatcher(bAppHasGUI, dwFilterFlags)
, m_pfnFileChangeNotify(NULL)
{
	m_dwMonitorType = dwMonitorType;

	InitializeCriticalSection(&m_csFileChange);
}

CDirectoryChangeMonitor::~CDirectoryChangeMonitor(void)
{
	StopFileChangeMonitor();
	DeleteCriticalSection(&m_csFileChange);
}

BOOL CDirectoryChangeMonitor::SetFileChangeNotify(LPCWSTR lpcwszFileName, LPFILE_CHANGE_NOTIFY lpfnFileChangeNotify, BOOL bRemove)
{
	BOOL bRet = FALSE;

	do 
	{
		if (NULL == lpfnFileChangeNotify)
		{
			break;
		}

		EnterCriticalSection(&m_csFileChange);
		if (bRemove)
		{
			if (lpfnFileChangeNotify == m_pfnFileChangeNotify)
			{
				m_pfnFileChangeNotify = NULL;
				bRet = TRUE;
			}
		}
		else
		{
			m_pfnFileChangeNotify = lpfnFileChangeNotify;
			bRet = TRUE;
		}
		LeaveCriticalSection(&m_csFileChange);

	} while (FALSE);

	return bRet;
}

BOOL CDirectoryChangeMonitor::StartFileChangeNotify()
{
	BOOL bRet = TRUE;

	return bRet;
}

void CDirectoryChangeMonitor::StopFileChangeMonitor()
{

}

void CDirectoryChangeMonitor::On_FileAdded(const CString & strFileName, TARGET_TYPE eTargetType)
{
	if (eIsFile == eTargetType)
	{
		EnterCriticalSection(&m_csFileChange);
		if (NULL != m_pfnFileChangeNotify)
		{
			m_pfnFileChangeNotify(strFileName);
		}
		LeaveCriticalSection(&m_csFileChange);
	}
}

void CDirectoryChangeMonitor::On_FileRemoved(const CString & strFileName, TARGET_TYPE eTargetType)
{

}

void CDirectoryChangeMonitor::On_FileNameChanged(const CString & strOldFileName, const CString & strNewFileName, TARGET_TYPE eTargetType)
{
	if (eIsFile == eTargetType)
	{
		LPCTSTR lpcszNewFileName = (LPCTSTR)strNewFileName;

		if (lpcszNewFileName+1 != StrStrI(lpcszNewFileName, ISOLATE_PATH_W+1))
		{
			EnterCriticalSection(&m_csFileChange);
			if (NULL != m_pfnFileChangeNotify)
			{
				m_pfnFileChangeNotify(strOldFileName);
				m_pfnFileChangeNotify(strNewFileName);
			}
			LeaveCriticalSection(&m_csFileChange);
		}
	}
}

void CDirectoryChangeMonitor::On_FileModified(const CString & strFileName, TARGET_TYPE eTargetType)
{
	if (eIsFile == eTargetType)
	{
		EnterCriticalSection(&m_csFileChange);
		if (NULL != m_pfnFileChangeNotify)
		{
			m_pfnFileChangeNotify(strFileName);
		}
		LeaveCriticalSection(&m_csFileChange);
	}
}

bool CDirectoryChangeMonitor::On_FilterNotification(DWORD dwNotifyAction, LPCTSTR szFileName, LPCTSTR szNewFileName, TARGET_TYPE eTargetType)
{
	return true;
}

void CDirectoryChangeMonitor::On_ReadDirectoryChangesError(DWORD dwError, const CString & strDirectoryName)
{

}

void CDirectoryChangeMonitor::On_WatchStarted(DWORD dwError, const CString & strDirectoryName)
{

}

void CDirectoryChangeMonitor::On_WatchStopped(const CString & strDirectoryName)
{
	StopFileChangeMonitor();
}
