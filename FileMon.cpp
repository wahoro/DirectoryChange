#include "FileChangeMonitor.h"
#include "DirectoryChangeMonitor.h"
#include "SafeDogFileMon.h"

PVOID WINAPI CreateFileMonInstance(DWORD dwMonitorType)
{
	PVOID pMonitor = NULL;

	if (MONITOR_TYPE_FILES == dwMonitorType)
	{
		pMonitor = new CFileChangeMonitor(dwMonitorType, FALSE);
	}
	else if (MONITOR_TYPE_DIRECTORY == dwMonitorType)
	{
		pMonitor = new CDirectoryChangeMonitor(dwMonitorType, FALSE);
	}
	else
	{
		pMonitor = new CDirectoryChangeHandler();
	}

	return pMonitor;
}

VOID WINAPI DestroyFileMonInstance(IN PVOID pInstance)
{
	if (NULL != pInstance)
	{
		CDirectoryChangeHandler * pDirectoryChangeHandler = (CDirectoryChangeHandler *)pInstance;
		if (MONITOR_TYPE_FILES == pDirectoryChangeHandler->m_dwMonitorType)
		{
			CFileChangeMonitor * pFileChangeMonitor = (CFileChangeMonitor *)pInstance;
			delete pFileChangeMonitor;
		}
		else if (MONITOR_TYPE_DIRECTORY == pDirectoryChangeHandler->m_dwMonitorType)
		{
			CDirectoryChangeMonitor * pDirectoryChangeMonitor = (CDirectoryChangeMonitor *)pInstance;
			delete pDirectoryChangeMonitor;
		}
		else
		{
			delete pDirectoryChangeHandler;
		}
	}
}

BOOL WINAPI SetFileMonNotify(
	IN PVOID pInstance,
	IN LPCWSTR lpcwszFileName,
	IN LPFILE_CHANGE_NOTIFY lpfnFileChangeNotify,
	IN BOOL bRemove
)
{
	BOOL bRet = FALSE;
	
	if (NULL != pInstance)
	{
		CDirectoryChangeHandler * pDirectoryChangeHandler = (CDirectoryChangeHandler *)pInstance;
		if (MONITOR_TYPE_FILES == pDirectoryChangeHandler->m_dwMonitorType)
		{
			CFileChangeMonitor * pFileChangeMonitor = (CFileChangeMonitor *)pInstance;		
			bRet = pFileChangeMonitor->SetFileChangeNotify(lpcwszFileName, lpfnFileChangeNotify, bRemove);
		}
		else if (MONITOR_TYPE_DIRECTORY == pDirectoryChangeHandler->m_dwMonitorType)
		{
			CDirectoryChangeMonitor * pDirectoryChangeMonitor = (CDirectoryChangeMonitor *)pInstance;
			bRet = pDirectoryChangeMonitor->SetFileChangeNotify(lpcwszFileName, lpfnFileChangeNotify, bRemove);
		}
		else
		{
			
		}
	}

	return bRet;
}

BOOL StartWatchDirectory(
	IN PVOID pInstance,
	IN LPCWSTR lpwszDirectory, 
	IN DWORD dwNotifyFilter, 
	IN BOOL bWatchSubDirs,
	IN LPCWSTR lpcwszIncludeFilter,
	IN LPCWSTR lpcwszExcludeFilter
)
{
	BOOL bRet = FALSE;
	CFileChangeMonitor * pFileChangeMonitor = NULL;
	CDirectoryChangeMonitor * pDirectoryChangeMonitor = NULL;
	CDirectoryChangeHandler * pDirectoryChangeHandler = NULL;

	do 
	{
		if (NULL == pInstance || NULL == lpwszDirectory || 0 == dwNotifyFilter)
		{
			SetLastError(ERROR_INVALID_PARAMETER);
			break;
		}

		pDirectoryChangeHandler = (CDirectoryChangeHandler * )pInstance;
		if (MONITOR_TYPE_DIRECTORY == pDirectoryChangeHandler->m_dwMonitorType)
		{
			pDirectoryChangeMonitor = (CDirectoryChangeMonitor *)pInstance;

			bRet = (ERROR_SUCCESS == pDirectoryChangeMonitor->m_DirctoryChangeWatcher.WatchDirectory(
				lpwszDirectory,
				dwNotifyFilter,
				pDirectoryChangeMonitor,
				bWatchSubDirs,
				lpcwszIncludeFilter,
				lpcwszExcludeFilter
				));
		}
		else if (MONITOR_TYPE_FILES == pDirectoryChangeHandler->m_dwMonitorType)
		{
			pFileChangeMonitor = (CFileChangeMonitor *)pInstance;

			if (!pFileChangeMonitor->StartFileChangeNotify())
			{
				break;
			}

			bRet = (ERROR_SUCCESS == pFileChangeMonitor->m_DirctoryChangeWatcher.WatchDirectory(
				lpwszDirectory,
				dwNotifyFilter,
				pFileChangeMonitor,
				bWatchSubDirs,
				lpcwszIncludeFilter,
				lpcwszExcludeFilter
				));
		}
		else
		{

		}

	} while (FALSE);

	return bRet;
}

BOOL StopWatchDirectory(
	IN PVOID pInstance,
	IN LPCWSTR lpwszDirectory
)
{
	BOOL bRet = FALSE;
	CFileChangeMonitor * pFileChangeMonitor = NULL;
	CDirectoryChangeMonitor * pDirectoryChangeMonitor = NULL;
	CDirectoryChangeHandler * pDirectoryChangeHandler = NULL;

	do 
	{
		if (NULL == pInstance || NULL == lpwszDirectory)
		{
			SetLastError(ERROR_INVALID_PARAMETER);
			break;
		}

		pDirectoryChangeHandler = (CDirectoryChangeHandler * )pInstance;
		if (MONITOR_TYPE_DIRECTORY == pDirectoryChangeHandler->m_dwMonitorType)
		{
			pDirectoryChangeMonitor = (CDirectoryChangeMonitor *)pInstance;

			bRet = pDirectoryChangeMonitor->m_DirctoryChangeWatcher.UnwatchDirectory(lpwszDirectory);
		}
		else if (MONITOR_TYPE_FILES == pDirectoryChangeHandler->m_dwMonitorType)
		{
			pFileChangeMonitor = (CFileChangeMonitor *)pInstance;

			bRet = pFileChangeMonitor->m_DirctoryChangeWatcher.UnwatchDirectory(lpwszDirectory);
		}
		else
		{

		}

	} while (FALSE);

	return bRet;
}