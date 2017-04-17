#include "FileChangeMonitor.h"

CFileChangeMonitor::CFileChangeMonitor(DWORD dwMonitorType, bool bAppHasGUI, DWORD dwFilterFlags)
: m_DirctoryChangeWatcher(bAppHasGUI, dwFilterFlags)
, m_hEventExit(NULL)
, m_pThreadChangeNotify(NULL)
{
	m_dwMonitorType = dwMonitorType;
	InitializeCriticalSection(&m_csFileChange);
}

CFileChangeMonitor::~CFileChangeMonitor(void)
{
	StopFileChangeMonitor();
	DeleteCriticalSection(&m_csFileChange);
}

BOOL CFileChangeMonitor::SetFileChangeNotify(LPCWSTR lpcwszFileName, LPFILE_CHANGE_NOTIFY lpfnFileChangeNotify, BOOL bRemove)
{
	BOOL bRet = FALSE;
	map <wstring, CHANGED_FILE_INFORMATION>::iterator it = m_mapChangedFiles.find(lpcwszFileName);

	do 
	{
		if (NULL == lpcwszFileName || NULL == lpfnFileChangeNotify)
		{
			break;
		}

		EnterCriticalSection(&m_csFileChange);

		if (bRemove)
		{
			//
			// �ļ���Ӧ��¼�Ѵ���
			//
			if (m_mapChangedFiles.end() != it)
			{
				//
				// �Ӽ�¼��ɾ���ص�����ָ��
				//
				it->second.RegisteredCallbackRoutines.erase(lpfnFileChangeNotify);

				//
				// ���û�лص�������ɾ����Ӧ�ļ��ڵ�
				//
				if (0 == it->second.RegisteredCallbackRoutines.size())
				{
					m_mapChangedFiles.erase(lpcwszFileName);
				}

				bRet = TRUE;
			}
		}
		else
		{
			if (m_mapChangedFiles.end() != it)
			{
				//
				// �ļ���Ӧ��¼�Ѵ���
				//
				it->second.RegisteredCallbackRoutines.insert(lpfnFileChangeNotify);
			}
			else
			{
				//
				// �ļ���Ӧ��¼�����ڣ������Ҫ���ӵ��ļ���¼
				//
				CHANGED_FILE_INFORMATION ChangedFileInformation;

				ChangedFileInformation.dwLastWriteTime = 0;
				ChangedFileInformation.RegisteredCallbackRoutines.insert(lpfnFileChangeNotify);
				ChangedFileInformation.wstrFileName = lpcwszFileName;

				m_mapChangedFiles[lpcwszFileName] = ChangedFileInformation;
			}

			bRet = TRUE;
		}

		LeaveCriticalSection(&m_csFileChange);
	} while (FALSE);

	return bRet;
}

BOOL CFileChangeMonitor::StartFileChangeNotify()
{
	BOOL bRet = FALSE;

	do 
	{
		if (NULL != m_hEventExit && NULL != m_pThreadChangeNotify)
		{
			bRet = TRUE;
			break;
		}

		m_hEventExit = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (NULL == m_hEventExit)
		{
			break;
		}

		m_pThreadChangeNotify = AfxBeginThread(CFileChangeMonitor::ThreadNotify, this, 0, 0, CREATE_SUSPENDED, NULL);
		if (NULL != m_pThreadChangeNotify)
		{
			m_pThreadChangeNotify->m_bAutoDelete = FALSE;
			m_pThreadChangeNotify->ResumeThread();
			bRet = TRUE;
		}

	} while (FALSE);

	return bRet;
}

void CFileChangeMonitor::StopFileChangeMonitor()
{
	//
	// ֪ͨ�߳��˳�
	//
	if (NULL != m_hEventExit)
	{
		SetEvent(m_hEventExit);
		CloseHandle(m_hEventExit);
		m_hEventExit = NULL;
	}

	//
	// �ȴ��߳��˳�����ɾ���̶߳���
	//
	if (NULL != m_pThreadChangeNotify)
	{
		if (NULL != m_pThreadChangeNotify->m_hThread)
		{
			WaitForSingleObject(m_pThreadChangeNotify->m_hThread, INFINITE);
		}

		delete m_pThreadChangeNotify;
		m_pThreadChangeNotify = NULL;
	}
}

UINT CFileChangeMonitor::ThreadNotify()
{
	DWORD dwWait = WAIT_TIMEOUT;
	DWORD dwCurrentTickCount = 0;
	map <wstring, CHANGED_FILE_INFORMATION>::iterator it;
	set <LPFILE_CHANGE_NOTIFY>::iterator itCallback;

	do 
	{
		if (NULL == m_hEventExit)
		{
			break;
		}

		dwWait = WaitForSingleObjectEx(m_hEventExit, 500, FALSE);
		if (WAIT_TIMEOUT != dwWait)
		{
			break;
		}

		EnterCriticalSection(&m_csFileChange);

		dwCurrentTickCount = GetTickCount();
		for(it = m_mapChangedFiles.begin(); it != m_mapChangedFiles.end(); ++it)
		{
			//
			// �ļ�û�з����仯
			//
			if (0 == it->second.dwLastWriteTime)
			{
				continue;
			}

			//
			// ���һ���ļ�д����û�г��� 500 ����, ����ʱ��֪ͨ�ñ仯
			//
			if (it->second.dwLastWriteTime+1000 > dwCurrentTickCount)
			{
				continue;
			}

			itCallback = it->second.RegisteredCallbackRoutines.begin();
			for (; it->second.RegisteredCallbackRoutines.end() != itCallback; ++itCallback)
			{
				QueueUserWorkItem((LPTHREAD_START_ROUTINE)(*itCallback), (PVOID)it->second.wstrFileName.c_str(), WT_EXECUTELONGFUNCTION);
			}

			it->second.dwLastWriteTime = 0;
		}

		LeaveCriticalSection(&m_csFileChange);

	} while (TRUE);

	return 0;
}

UINT CFileChangeMonitor::ThreadNotify(LPVOID lpParam)
{
	CFileChangeMonitor * pFileChangeMonitor = (CFileChangeMonitor *)lpParam;
	
	if (NULL != pFileChangeMonitor)
	{
		pFileChangeMonitor->ThreadNotify();
	}

	return 0;
}

void CFileChangeMonitor::On_FileAdded(const CString & strFileName,TARGET_TYPE eTargetType)
{

}

void CFileChangeMonitor::On_FileRemoved(const CString & strFileName,TARGET_TYPE eTargetType)
{

}

void CFileChangeMonitor::On_FileNameChanged(const CString & strOldFileName, const CString & strNewFileName, TARGET_TYPE eTargetType)
{

}

void CFileChangeMonitor::On_FileModified(const CString & strFileName,TARGET_TYPE eTargetType)
{
	map <wstring, CHANGED_FILE_INFORMATION>::iterator it = m_mapChangedFiles.find((LPCWSTR)strFileName);
	if (it != m_mapChangedFiles.end() && eIsFile == eTargetType)
	{
		EnterCriticalSection(&m_csFileChange);
		it->second.dwLastWriteTime = GetTickCount();
		LeaveCriticalSection(&m_csFileChange);
	}
}

bool CFileChangeMonitor::On_FilterNotification(DWORD dwNotifyAction, LPCTSTR szFileName, LPCTSTR szNewFileName, TARGET_TYPE eTargetType)
{
	return true;
}

void CFileChangeMonitor::On_ReadDirectoryChangesError(DWORD dwError, const CString & strDirectoryName)
{

}

void CFileChangeMonitor::On_WatchStarted(DWORD dwError, const CString & strDirectoryName)
{

}

void CFileChangeMonitor::On_WatchStopped(const CString & strDirectoryName)
{
	StopFileChangeMonitor();
}
