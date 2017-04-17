#include "stdafx.h"
#include "DirectoryChanges.h"
#include "DelayedDirectoryChangeHandler.h"
#include "DelLock.h"
/*
File Added   On_FileAdded
A file was added to a watched directory (newly created, or copied into that directory).
FILE_NOTIFY_CHANGE_FILE_NAME and/or FILE_NOTIFY_CHANGE_DIR_NAME (for directories)


File Removed   On_FileRemoved
A file was deleted, or removed from the watched directory(ie: sent to the recycle bin, moved to another directory, or deleted)
FILE_NOTIFY_CHANGE_FILE_NAME  FILE_NOTIFY_CHANGE_DIR_NAME


File Name Changed   On_FileNameChanged
A file's name has been changed in the watched directory.  The parameters to this notification are the old file name, and the new file name.
FILE_NOTIFY_CHANGE_FILE_NAME  FILE_NOTIFY_CHANGE_DIR_NAME

File Modified   On_FileModified
A file was modified in some manner in a watched directory.
Things that can cause you to receive this notification include changes to a file's last accessed, last modified, or created timestamps.
Other changes, such as a change to a file's attributes, size, or security descriptor can also trigger this notification.
FILE_NOTIFY_CHANGE_ATTRIBUTES
FILE_NOTIFY_CHANGE_SIZE 
FILE_NOTIFY_CHANGE_LAST_WRITE 
FILE_NOTIFY_CHANGE_LAST_ACCESS
FILE_NOTIFY_CHANGE_CREATION
FILE_NOTIFY_CHANGE_SECURITY
*/

typedef enum _FILTER_MATCH_TYPE FILTER_MATCH_TYPE;

typedef VOID (__stdcall *ON_FILE_RENAMED)(PWCHAR pwszOldFileName,ULONG ulOldFileNameLenInWchar,PWCHAR pwszNewFileName,ULONG ulNewFileNameLenInWchar,CDirectoryChangeHandler::TARGET_TYPE eTargetType);

typedef VOID (__stdcall *ON_FILE_ADDED)(PWCHAR pwszFileAdded,ULONG ulFileNameLenInWchar,CDirectoryChangeHandler::TARGET_TYPE eTargetType);

typedef VOID (__stdcall *ON_FILE_REMOVED)(PWCHAR pwszFileRemoved,ULONG ulFileNameLenInWchar,CDirectoryChangeHandler::TARGET_TYPE eTargetType);

typedef VOID (__stdcall *ON_FILE_MODIFIED)(PWCHAR pwszFileModified,ULONG ulFileNameLenInWchar,CDirectoryChangeHandler::TARGET_TYPE eTargetType);

typedef VOID (__stdcall *ON_ERROR)(DWORD dwError,PWCHAR pwszMonitorDirName,ULONG ulFileNameLenInWchar);

#pragma pack(push,1)

typedef struct _FIEL_CHANGE_CALLBACKS
{
	ON_FILE_RENAMED pOnFileRenamed;

	ON_FILE_ADDED pOnFileAdded;

	ON_FILE_REMOVED pOnFileRemoved;

	ON_FILE_MODIFIED pOnFileModified;

	ON_ERROR pOnError;

}FIEL_CHANGE_CALLBACKS,*PFIEL_CHANGE_CALLBACKS;

#pragma pack(pop)

class DirChangeHandler:public CDirectoryChangeHandler
{

public:
	DirChangeHandler(PFIEL_CHANGE_CALLBACKS pCallbacks)
	{
		InitDelLock(&m_DelLock);

		m_Callbacks=*pCallbacks;
	}

	~DirChangeHandler()
	{
		WaitForAllDelLockRelease(&m_DelLock);

		RtlZeroMemory(&m_Callbacks,sizeof(m_Callbacks));
	}

private:

	DEL_LOCK m_DelLock;

	FIEL_CHANGE_CALLBACKS m_Callbacks;

	BOOL Ref()
	{
		return AcquireDelLock(&m_DelLock);
	}

	VOID Def()
	{
		ReleaseDelLock(&m_DelLock);
	}

private:

	virtual void On_FileNameChanged(const CString & strOldFileName, const CString & strNewFileName,TARGET_TYPE eTargetType)
	{
		BOOL bIsNeedRelease=Ref();

		if(bIsNeedRelease &&
		   m_Callbacks.pOnFileRenamed)
		{
			m_Callbacks.pOnFileRenamed((PWCHAR)(const wchar_t *)strOldFileName,
				                       (ULONG)strOldFileName.GetLength(),
									   (PWCHAR)(const wchar_t *)strNewFileName,
									   (ULONG)strNewFileName.GetLength(),
									   eTargetType);
		}

		if (bIsNeedRelease)
		{
			bIsNeedRelease=FALSE;

			Def();
		}

		return;
	}

	virtual void On_FileAdded(const CString & strFileName,TARGET_TYPE eTargetType)
	{
		BOOL bIsNeedRelease=Ref();

		if(bIsNeedRelease &&
			m_Callbacks.pOnFileAdded)
		{
			m_Callbacks.pOnFileAdded((PWCHAR)(const wchar_t *)strFileName,
				(ULONG)strFileName.GetLength(),
				eTargetType);
		}

		if (bIsNeedRelease)
		{
			bIsNeedRelease=FALSE;

			Def();
		}

		return;
	}

	virtual void On_FileRemoved(const CString & strFileName,TARGET_TYPE eTargetType)
	{
		BOOL bIsNeedRelease=Ref();

		if(bIsNeedRelease &&
			m_Callbacks.pOnFileRemoved)
		{
			m_Callbacks.pOnFileRemoved((PWCHAR)(const wchar_t *)strFileName,
				(ULONG)strFileName.GetLength(),
				eTargetType);
		}

		if (bIsNeedRelease)
		{
			bIsNeedRelease=FALSE;

			Def();
		}

		return;
	}

	virtual void On_FileModified(const CString & strFileName,TARGET_TYPE eTargetType)
	{
		BOOL bIsNeedRelease=Ref();

		if(bIsNeedRelease &&
			m_Callbacks.pOnFileModified)
		{
			m_Callbacks.pOnFileModified((PWCHAR)(const wchar_t *)strFileName,
				(ULONG)strFileName.GetLength(),
				eTargetType);
		}

		if (bIsNeedRelease)
		{
			bIsNeedRelease=FALSE;

			Def();
		}

		return;
	}

	virtual void On_ReadDirectoryChangesError(DWORD dwError, const CString & strDirectoryName)//不管用不用关心这个函数里的信息，这个函数必须被重载
	{
		BOOL bIsNeedRelease=Ref();

		if(bIsNeedRelease &&
			m_Callbacks.pOnError)
		{
			m_Callbacks.pOnError(dwError,
				(PWCHAR)(const wchar_t *)strDirectoryName,
				(ULONG)strDirectoryName.GetLength());
		}

		if (bIsNeedRelease)
		{
			bIsNeedRelease=FALSE;

			Def();
		}
	}
};

typedef struct _FILE_WATCH_OBJ
{
	CDirectoryChangeWatcher* pFileWatch;

	DirChangeHandler* pHandler;

}FILE_WATCH_OBJ,*PFILE_WATCH_OBJ;


extern "C"
{
	PVOID __stdcall CreateFileWatchObj(BOOL bIsRunInGUIApp,
		PFIEL_CHANGE_CALLBACKS Callbacks)
	{
		CDirectoryChangeWatcher* pWatch=NULL;

		DirChangeHandler* pHandler=NULL;

		PFILE_WATCH_OBJ pFileWatchObj=NULL;

		BOOL bIsOk=FALSE;

		do 
		{
			pWatch=new CDirectoryChangeWatcher(bIsRunInGUIApp);

			if (!pWatch)
			{
				break;
			}

			pHandler=new DirChangeHandler(Callbacks);

			if (!pWatch)
			{
				break;
			}

			pFileWatchObj=(PFILE_WATCH_OBJ)malloc(sizeof(FILE_WATCH_OBJ));

			if (!pFileWatchObj)
			{
				break;
			}

			pFileWatchObj->pFileWatch=pWatch;

			pFileWatchObj->pHandler=pHandler;

			bIsOk=TRUE;

		} while (FALSE);

		if (!bIsOk)
		{
			if (pWatch)
			{
				delete pWatch;

				pWatch=NULL;
			}

			if (pHandler)
			{
				delete pHandler;

				pHandler=NULL;
			}

			if (pFileWatchObj)
			{
				free(pFileWatchObj);

				pFileWatchObj=NULL;
			}
		}

		return (PVOID)pFileWatchObj;
	}

	BOOL __stdcall StartWatchDir(PVOID pWatchObj,
		PWCHAR pwszDirPathToWatch,
		ULONG ulWatchActionFlags,
		PWCHAR pwszIncludeWatchFilters,
		PWCHAR pwszExcludeWatchFilters,
		BOOL bIsNeedWatchSubDir)
	{
		BOOL bIsOk=FALSE;

		PFILE_WATCH_OBJ pFileWatchObj=(PFILE_WATCH_OBJ)pWatchObj;

		DWORD dwWatchRet=0;

		do 
		{
			if (!pFileWatchObj ||
				!pwszDirPathToWatch)
			{
				break;
			}

			dwWatchRet=pFileWatchObj->pFileWatch->WatchDirectory(pwszDirPathToWatch,
				ulWatchActionFlags,
				pFileWatchObj->pHandler,
				bIsNeedWatchSubDir,
				pwszIncludeWatchFilters,
				pwszExcludeWatchFilters);
			bIsOk=(dwWatchRet==ERROR_SUCCESS);

		} while (FALSE);

		return bIsOk;
	}

	BOOL __stdcall StopWatch(PVOID pWatchObj,PWCHAR pwszDirToStopWatch)
	{
		BOOL bIsOk=FALSE;

		PFILE_WATCH_OBJ pFileWatchObj=(PFILE_WATCH_OBJ)pWatchObj;

		do 
		{
			if (!pFileWatchObj)
			{
				break;
			}

			if (!pwszDirToStopWatch)
			{
				bIsOk=pFileWatchObj->pFileWatch->UnwatchAllDirectories();
			}else
			{
				bIsOk=pFileWatchObj->pFileWatch->UnwatchDirectory(pwszDirToStopWatch);
			}

		} while (FALSE);

		return bIsOk;
	}

	VOID __stdcall DeleteWatchObj(PVOID pWatchObj)
	{
		PFILE_WATCH_OBJ pFileWatchObj=(PFILE_WATCH_OBJ)pWatchObj;

		if (pFileWatchObj)
		{
			StopWatch(pWatchObj,NULL);

			if (pFileWatchObj->pFileWatch)
			{
				delete pFileWatchObj->pFileWatch;

				pFileWatchObj->pFileWatch=NULL;
			}

			if (pFileWatchObj->pHandler)
			{
				delete pFileWatchObj->pHandler;

				pFileWatchObj->pHandler=NULL;
			}

			free(pFileWatchObj);

			pFileWatchObj=NULL;
		}

		return ;
	}

	BOOL __stdcall IsDirWatching(PVOID pWatchObj,PWCHAR pwszDirToStopWatch)
	{
		BOOL bIsOk=FALSE;

		PFILE_WATCH_OBJ pFileWatchObj=(PFILE_WATCH_OBJ)pWatchObj;

		do 
		{
			if (!pFileWatchObj ||
				!pwszDirToStopWatch)
			{
				break;
			}

			bIsOk=pFileWatchObj->pFileWatch->IsWatchingDirectory(pwszDirToStopWatch);


		} while (FALSE);

		return bIsOk;
	}

	INT __stdcall GetWatchingDirCount(PVOID pWatchObj)
	{
		BOOL bIsOk=FALSE;

		PFILE_WATCH_OBJ pFileWatchObj=(PFILE_WATCH_OBJ)pWatchObj;

		INT iWatchingCount=-1;

		do 
		{
			if (!pFileWatchObj)
			{
				break;
			}

			iWatchingCount=pFileWatchObj->pFileWatch->NumWatchedDirectories();

		} while (FALSE);

		return iWatchingCount;
	}

	FILTER_MATCH_TYPE __stdcall SetFilterMatchType(PVOID pWatchObj,FILTER_MATCH_TYPE dwFlags)
	{
		FILTER_MATCH_TYPE eOldFlags=(FILTER_MATCH_TYPE)0;

		PFILE_WATCH_OBJ pFileWatchObj=(PFILE_WATCH_OBJ)pWatchObj;

		do 
		{
			if (!pFileWatchObj)
			{
				__asm int 3;
			}

			eOldFlags=(FILTER_MATCH_TYPE)pFileWatchObj->pFileWatch->SetFilterFlags(dwFlags);

		} while (FALSE);

		return eOldFlags;
	}

	FILTER_MATCH_TYPE __stdcall GetFilterMatchType(PVOID pWatchObj)
	{
		FILTER_MATCH_TYPE eOldFlags=(FILTER_MATCH_TYPE)0;

		PFILE_WATCH_OBJ pFileWatchObj=(PFILE_WATCH_OBJ)pWatchObj;

		do 
		{
			if (!pFileWatchObj)
			{
				__asm int 3;
			}

			eOldFlags=(FILTER_MATCH_TYPE)pFileWatchObj->pFileWatch->GetFilterFlags();

		} while (FALSE);

		return eOldFlags;
	}
}

VOID __stdcall OnFileModify(PWCHAR pwszFileModified,ULONG ulFileNameLenInWchar,CDirectoryChangeHandler::TARGET_TYPE eTargetType)
{
	printf("FileName Modify:%ws\n",pwszFileModified);
}

VOID __stdcall OnFileAdd(PWCHAR pwszFileModified,ULONG ulFileNameLenInWchar,CDirectoryChangeHandler::TARGET_TYPE eTargetType)
{
	printf("FileName Add:%ws\n",pwszFileModified);
}


VOID __stdcall OnFileDel(PWCHAR pwszFileModified,ULONG ulFileNameLenInWchar,CDirectoryChangeHandler::TARGET_TYPE eTargetType)
{
	printf("FileName Del:%ws\n",pwszFileModified);
}

#include <locale>

int main(int argc,char** argv)
{
	FIEL_CHANGE_CALLBACKS Callbacks={0};

	Callbacks.pOnFileModified=OnFileModify;

	Callbacks.pOnFileAdded=OnFileAdd;

	Callbacks.pOnFileRemoved=OnFileDel;

	PVOID pFileObj=CreateFileWatchObj(FALSE,&Callbacks);

	setlocale(LC_ALL, "chs");

	StartWatchDir(pFileObj,L"C:\\",FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_LAST_ACCESS | FILE_NOTIFY_CHANGE_CREATION,L"*.ini;*.txt",NULL,TRUE);

	while(1)
	{
		Sleep(0);
	}

	return 0;
}