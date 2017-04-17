#ifndef FILE_MON_H__
#define FILE_MON_H__

#ifndef __cplusplus
extern "C" {
#endif

#define MONITOR_TYPE_DIRECTORY      1	// 监视目标为目录
#define MONITOR_TYPE_FILES			2	// 监视目标为文件

typedef BOOL (WINAPI * LPFILE_CHANGE_NOTIFY)(IN LPCWSTR lpcwszFileName);


//
// 功能：
//     创建监视实例
//
// 参数：
//     dwMonitorType 监视对象类型
//
// 返回值:
//     成功返回实例对象，失败返回 NULL.
//
PVOID WINAPI CreateFileMonInstance(DWORD dwMonitorType);


//
// 功能：
//     销毁监视实例
//
// 参数：
//     pInstance 需要被销毁的实例对象
//
// 返回值:
//     成功返回实例对象，失败返回 NULL.
//
VOID WINAPI DestroyFileMonInstance(IN PVOID pInstance);


//
// 功能：
//     设置文件变化通知回调函数
//
// 参数：
//     pInstance 实例对象
//     lpcwszFileName 需要监视的文件名
//     lpfnFileChangeNotify 文件有变化时，调用的回调函数
//     bRemove 是设置还是移除开关
//
// 返回值:
//     成功返回 TRUE，失败返回 FALSE.
//
BOOL WINAPI SetFileMonNotify(
	IN PVOID pInstance,
	IN LPCWSTR lpcwszFileName,
	IN LPFILE_CHANGE_NOTIFY lpfnFileChangeNotify,
	IN BOOL bRemove
);


//
// 功能：开始对指定目录中的文件变化进行监视
//     
// 参数：
//     pInstance 实例对象
//     lpwszDirectory 需要监视的文件路径
//     dwNotifyFilter 需要监视的文件变化行为
//     bWatchSubDirs 是否监视子路径
//     lpcwszIncludeFilter 要监视的文件后缀 *.xxx;*.?x?, NULL 表示监视全部文件类型
//     lpcwszExcludeFilter 排除的文件后缀
//
// 返回值:
//     成功返回 TRUE，失败返回 FALSE.
//
BOOL StartWatchDirectory(
	IN PVOID pInstance,
	IN LPCWSTR lpwszDirectory,
	IN DWORD dwNotifyFilter, 
	IN BOOL bWatchSubDirs,
	IN LPCWSTR lpcwszIncludeFilter,
	IN LPCWSTR lpcwszExcludeFilter
);

//
// 功能：停止对指定目录中的文件变化监视
//
// 参数:
//		pInstance 实例对象
//		lpcwszDirectory 需要停止监视的文件路径
//
BOOL StopWatchDirectory(
	IN PVOID pInstance,
	IN LPCWSTR lpwszDirectory
);

#ifndef __cplusplus
}
#endif

#endif	// _SAFE_DOG_FILE_MON_H__
