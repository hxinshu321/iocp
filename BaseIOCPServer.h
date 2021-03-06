#pragma once

typedef enum _IO_OPERATION {
    ClientIoAccept,
    ClientIoRead,
    ClientIoWrite
} IO_OPERATION, *PIO_OPERATION;

typedef enum _IO_POST_RESULT {
	PostIoSuccess,
	PostIoInvalicSocket,
	PostIoFailed
}IO_POST_RESULT,*PIO_POST_RESULT;

typedef struct _PER_IO_CONTEXT {
    WSAOVERLAPPED               Overlapped;
    WSABUF                      wsabuf;
    IO_OPERATION                IOOperation;
    SOCKET                      SocketAccept; 
	CIOCPBuffer*				IOCPBuffer;

	_PER_IO_CONTEXT();
} PER_IO_CONTEXT, *PPER_IO_CONTEXT;

class _PER_SOCKET_CONTEXT {
public:
    SOCKET                      m_Socket;
	IOCPMutex					m_Lock;
	LONGLONG					m_guid;
	ULONG						m_NumberOfPendingIO;
	PER_IO_CONTEXT				m_SendContext;
	PER_IO_CONTEXT				m_RecvContext;
	IOCPQueue<CIOCPBuffer*>		m_SendBufferList;

	std::list<_PER_SOCKET_CONTEXT*>::iterator pos;
	_PER_SOCKET_CONTEXT();
	virtual ~_PER_SOCKET_CONTEXT();
};

typedef _PER_SOCKET_CONTEXT PER_SOCKET_CONTEXT;
typedef PER_SOCKET_CONTEXT* PPER_SOCKET_CONTEXT;


class _PER_SOCKET_CONTEXT_LIST
{
public:
	IOCPMutex m_ContextLock;
	std::map<LONGLONG,PPER_SOCKET_CONTEXT> m_vContextMap;
	std::list<PPER_SOCKET_CONTEXT> m_vContextList;

	void ClearAll();
	void AddContext(PPER_SOCKET_CONTEXT lpPerSocketContext);
	void DeleteContext(PPER_SOCKET_CONTEXT lpPerSocketContext);
	PPER_SOCKET_CONTEXT GetContext(LONGLONG guid);		//使用这个函数需要手动锁定 ***且不会增加引用计数器***
};

typedef _PER_SOCKET_CONTEXT_LIST PER_SOCKET_CONTEXT_LIST;
typedef PER_SOCKET_CONTEXT_LIST* PPER_SOCKET_CONTEXT_LIST;
class CBaseIOCPServer
{
private:
	WSADATA m_WSAData;
	LPFN_ACCEPTEX m_AcceptEx;
	SOCKET m_Listen;
	HANDLE m_IOCP;
	HANDLE* m_ThreadHandles;
	DWORD m_ThreadHandleCount;
	
	ULONG m_CurrentConnectCount;
	ULONG m_LimitConnectCount;

	PER_SOCKET_CONTEXT_LIST m_ContextList;

	std::vector <PPER_IO_CONTEXT> m_AcceptIOContext;
private:
	SOCKET CreateSocket(void);
	BOOL CreateListenSocket(USHORT nPort);
	BOOL CreateAcceptSocket(PPER_IO_CONTEXT lpPerIOContext = NULL);
	FARPROC GetExtensionProcAddress(GUID& Guid);
	VOID UpdateCompletionPort(SOCKET hSocket, DWORD_PTR lpCompletionKey = NULL);
	VOID UpdateSocket(SOCKET sd);
	

	PPER_SOCKET_CONTEXT AllocateSocketContext();
	VOID ReleaseSocketContext(PPER_SOCKET_CONTEXT lpPerSocketContext);

	ULONG EnterIOLoop(PPER_SOCKET_CONTEXT lpPerSocketContext);
	ULONG ExitIOLoop(PPER_SOCKET_CONTEXT lpPerSocketContext);

	VOID PostClientIoRead(PPER_SOCKET_CONTEXT lpPerSocketContext,PPER_IO_CONTEXT lpPerIOContext,IO_POST_RESULT& PostResult);
	VOID PostClientIoWrite(PPER_SOCKET_CONTEXT lpPerSocketContext,PPER_IO_CONTEXT lpPerIOContext,IO_POST_RESULT& PostResult);

	VOID OnClientIoAccept(PPER_SOCKET_CONTEXT lpPerSocketContext,PPER_IO_CONTEXT lpPerIOContext,DWORD dwIoSize);
	VOID OnClientIoRead(PPER_SOCKET_CONTEXT lpPerSocketContext,PPER_IO_CONTEXT lpPerIOContext,DWORD dwIoSize);
	VOID OnClientIoWrite(PPER_SOCKET_CONTEXT lpPerSocketContext,PPER_IO_CONTEXT lpPerIOContext,DWORD dwIoSize);

	static DWORD WINAPI WorkerThread(LPVOID Param);

protected:
	VOID CloseClient (PPER_SOCKET_CONTEXT lpPerSocketContext);

public:
	BOOL Startup(USHORT nPort,DWORD dwWorkerThreadCount,DWORD dwMaxConnection);
	BOOL Shutdown();
	BOOL Send(PPER_SOCKET_CONTEXT lpPerSocketContext,CIOCPBuffer* lpIOCPBuffer);
	BOOL Send(LONGLONG guid,CIOCPBuffer* lpIOCPBuffer);
	CBaseIOCPServer(void);
	~CBaseIOCPServer(void);

protected:
	virtual VOID NotifyNewConnection(PPER_SOCKET_CONTEXT lpPerSocketContext);
	virtual VOID NotifyDisconnectedClient(PPER_SOCKET_CONTEXT lpPerSocketContext);
	virtual VOID NotifyWriteCompleted(PPER_SOCKET_CONTEXT lpPerSocketContext, CIOCPBuffer* lpIOCPBuffer);
	virtual VOID NotifyWritePackage(PPER_SOCKET_CONTEXT lpPerSocketContext, CIOCPBuffer* lpIOCPBuffer);
	virtual VOID NotifyReceivedPackage(PPER_SOCKET_CONTEXT lpPerSocketContext, CIOCPBuffer* pBuffer);
};

int myprintf(const char *lpFormat, ...);