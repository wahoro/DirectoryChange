#ifndef _SAFE_DOG_FILE_MON_H__
#define _SAFE_DOG_FILE_MON_H__

#ifndef __cplusplus
extern "C" {
#endif

#define MONITOR_TYPE_DIRECTORY      1	// ����Ŀ��ΪĿ¼
#define MONITOR_TYPE_FILES			2	// ����Ŀ��Ϊ�ļ�

typedef BOOL (WINAPI * LPFILE_CHANGE_NOTIFY)(IN LPCWSTR lpcwszFileName);


//
// ���ܣ�
//     ��������ʵ��
//
// ������
//     dwMonitorType ���Ӷ�������
//
// ����ֵ:
//     �ɹ�����ʵ������ʧ�ܷ��� NULL.
//
PVOID WINAPI CreateFileMonInstance(DWORD dwMonitorType);


//
// ���ܣ�
//     ���ټ���ʵ��
//
// ������
//     pInstance ��Ҫ�����ٵ�ʵ������
//
// ����ֵ:
//     �ɹ�����ʵ������ʧ�ܷ��� NULL.
//
VOID WINAPI DestroyFileMonInstance(IN PVOID pInstance);


//
// ���ܣ�
//     �����ļ��仯֪ͨ�ص�����
//
// ������
//     pInstance ʵ������
//     lpcwszFileName ��Ҫ���ӵ��ļ���
//     lpfnFileChangeNotify �ļ��б仯ʱ�����õĻص�����
//     bRemove �����û����Ƴ�����
//
// ����ֵ:
//     �ɹ����� TRUE��ʧ�ܷ��� FALSE.
//
BOOL WINAPI SetFileMonNotify(
	IN PVOID pInstance,
	IN LPCWSTR lpcwszFileName,
	IN LPFILE_CHANGE_NOTIFY lpfnFileChangeNotify,
	IN BOOL bRemove
);


//
// ���ܣ���ʼ��ָ��Ŀ¼�е��ļ��仯���м���
//     
// ������
//     pInstance ʵ������
//     lpwszDirectory ��Ҫ���ӵ��ļ�·��
//     dwNotifyFilter ��Ҫ���ӵ��ļ��仯��Ϊ
//     bWatchSubDirs �Ƿ������·��
//     lpcwszIncludeFilter Ҫ���ӵ��ļ���׺ *.xxx;*.?x?, NULL ��ʾ����ȫ���ļ�����
//     lpcwszExcludeFilter �ų����ļ���׺
//
// ����ֵ:
//     �ɹ����� TRUE��ʧ�ܷ��� FALSE.
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
// ���ܣ�ֹͣ��ָ��Ŀ¼�е��ļ��仯����
//
// ����:
//		pInstance ʵ������
//		lpcwszDirectory ��Ҫֹͣ���ӵ��ļ�·��
//
BOOL StopWatchDirectory(
	IN PVOID pInstance,
	IN LPCWSTR lpwszDirectory
);

#ifndef __cplusplus
}
#endif

#endif	// _SAFE_DOG_FILE_MON_H__