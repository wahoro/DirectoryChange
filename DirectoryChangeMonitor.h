#pragma once

#include <string>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
using namespace std;

#include "DirectoryChanges.h"
#include "DelayedDirectoryChangeHandler.h"

typedef BOOL (WINAPI * LPFILE_CHANGE_NOTIFY)(LPCWSTR lpcwszFileName);

class CDirectoryChangeMonitor:public CDirectoryChangeHandler
{
public:
	CDirectoryChangeMonitor(DWORD dwMonitorType, bool bAppHasGUI = true, DWORD dwFilterFlags = CDirectoryChangeWatcher::FILTERS_DEFAULT_BEHAVIOR);
	~CDirectoryChangeMonitor(void);

	BOOL StartFileChangeNotify();
	void StopFileChangeMonitor();
	BOOL SetFileChangeNotify(LPCWSTR lpcwszFileName, LPFILE_CHANGE_NOTIFY lpfnFileChangeNotify, BOOL bRemove);

	static UINT ThreadNotify(LPVOID lpParam);

public:
	CDirectoryChangeWatcher m_DirctoryChangeWatcher;

private:
	CRITICAL_SECTION m_csFileChange;
	LPFILE_CHANGE_NOTIFY m_pfnFileChangeNotify;

private:
	UINT ThreadNotify();

	void On_FileAdded(const CString & strFileName,TARGET_TYPE eTargetType);
	void On_FileRemoved(const CString & strFileName,TARGET_TYPE eTargetType);
	void On_FileNameChanged(const CString & strOldFileName, const CString & strNewFileName, TARGET_TYPE eTargetType);
	void On_FileModified(const CString & strFileName,TARGET_TYPE eTargetType);
	bool On_FilterNotification(DWORD dwNotifyAction, LPCTSTR szFileName, LPCTSTR szNewFileName, TARGET_TYPE eTargetType);
	void On_ReadDirectoryChangesError(DWORD dwError, const CString & strDirectoryName);
	void On_WatchStarted(DWORD dwError, const CString & strDirectoryName);
	void On_WatchStopped(const CString & strDirectoryName);

};
