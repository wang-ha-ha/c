#include <iostream>
#include <map>
#include <pthread.h>
#include <queue>
#include <signal.h>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <vector>

#define XMBool bool
#define XMTrue true
#define XMFalse false
#define XMString std::string
#define XMByteBuffer RCF::ByteBuffer
#define XMVector std::vector
#define XMQueue std::queue
#define XMMap std::map

class CProcUnitBase
{
public:
	CProcUnitBase(std::string &unitName);
	virtual ~CProcUnitBase();

	virtual void Run(pthread_t threadID);
	virtual void Exit();
	virtual bool GetStopState();
	virtual XMString GetName();

private:
	bool m_bExit;
	std::string m_strUnitName;
};

CProcUnitBase::CProcUnitBase(std::string &unitName)
{
	m_strUnitName = unitName;
}

CProcUnitBase::~CProcUnitBase()
{
}

bool CProcUnitBase::GetStopState()
{
	return false;
}

XMString CProcUnitBase::GetName()
{
	return m_strUnitName;
}

void CProcUnitBase::Run(pthread_t threadID)
{
}

void CProcUnitBase::Exit()
{
}

#define DEFAULT_TIME 10
#define MIN_WAIT_TASK_NUM 3
#define DEFAULT_THREAD_VARY 10

class CXMThreadPool
{
public:
	CXMThreadPool();
	~CXMThreadPool();

	bool Init(int min_thr_num, int max_thr_num);
	bool UnInit();

	bool ThreadPoolAddTask(CProcUnitBase *procUnit);
	int ThreadPoolAllThreadNum();
	int ThreadPoolBusyThreadNum();
	bool IsThreadAlive(pthread_t tid);

public:
	pthread_mutex_t m_mutexLock;
	pthread_mutex_t m_mutexThreadCounter;

	pthread_cond_t m_condQueueNotFull;
	pthread_cond_t m_condQueueNotEmpty;

	XMVector<pthread_t> m_vectorThreads;
	pthread_t m_ManageThread;
	XMQueue<CProcUnitBase *> m_queueTask;

	int m_iMinNum;
	int m_iMaxNum;
	int m_iLiveNum;
	int m_iBusyNum;
	int m_iWaitExitNum;
	int m_iQueueSize;
	int m_iShutDown;
};

void *ThreadPoolProc(void *arg)
{
	CXMThreadPool *objThreadPool = (CXMThreadPool *)arg;

	while (true)
	{
		pthread_mutex_lock(&(objThreadPool->m_mutexLock));

		while ((objThreadPool->m_iQueueSize == 0) && (!objThreadPool->m_iShutDown))
		{
			std::cout << "thread " << pthread_self() << " is waiting" << std::endl;
			pthread_cond_wait(&(objThreadPool->m_condQueueNotEmpty),
							  &(objThreadPool->m_mutexLock));

			if (objThreadPool->m_iWaitExitNum > 0)
			{
				objThreadPool->m_iWaitExitNum--;

				if (objThreadPool->m_iLiveNum > objThreadPool->m_iMinNum)
				{
					std::cout << "thread " << pthread_self() << " is exiting" << std::endl;
					objThreadPool->m_iLiveNum--;
					pthread_mutex_unlock(&(objThreadPool->m_mutexLock));

					pthread_exit(NULL);
				}
			}
		}

		if (objThreadPool->m_iShutDown)
		{
			pthread_mutex_unlock(&(objThreadPool->m_mutexLock));
			std::cout << "thread " << pthread_self() << "is exiting" << std::endl;
			pthread_detach(pthread_self());
			pthread_exit(NULL);
		}

		CProcUnitBase *taskProc = objThreadPool->m_queueTask.front();
		objThreadPool->m_queueTask.pop();
		objThreadPool->m_iQueueSize--;

		pthread_cond_broadcast(&(objThreadPool->m_condQueueNotFull));

		pthread_mutex_unlock(&(objThreadPool->m_mutexLock));

		std::cout << "thread " << pthread_self() << "start working\n"
				  << std::endl;
		pthread_mutex_lock(&(objThreadPool->m_mutexThreadCounter));
		objThreadPool->m_iBusyNum++;
		pthread_mutex_unlock(&(objThreadPool->m_mutexThreadCounter));

		std::cout << "taskProc:" << static_cast<const void *>(taskProc) << "  " << static_cast<const void *>(&taskProc) <<std::endl;
		if (taskProc != NULL)
		{
			if(taskProc->GetStopState() != true)
			{
				std::cout << taskProc->GetName() + " start:"<< pthread_self() << std::endl;
				taskProc->Run(pthread_self());
				std::cout << taskProc->GetName() + " end:"<< pthread_self() << std::endl;
			}

			delete taskProc;
		}

		std::cout << "thread " << pthread_self() << "end working\n"
				  << std::endl;
		pthread_mutex_lock(&(objThreadPool->m_mutexThreadCounter));
		objThreadPool->m_iBusyNum--;
		pthread_mutex_unlock(&(objThreadPool->m_mutexThreadCounter));
	}

	pthread_exit(NULL);
}

void *ThreadPoolManager(void *arg)
{
	CXMThreadPool *objThreadPool = (CXMThreadPool *)arg;

	int i = 0;
	while (!objThreadPool->m_iShutDown)
	{
		pthread_mutex_lock(&(objThreadPool->m_mutexLock));
		int queueSize = objThreadPool->m_iQueueSize;
		int liveThrNum = objThreadPool->m_iLiveNum;
		pthread_mutex_unlock(&(objThreadPool->m_mutexLock));

		pthread_mutex_lock(&(objThreadPool->m_mutexThreadCounter));
		int busyThrNum = objThreadPool->m_iBusyNum;
		pthread_mutex_unlock(&(objThreadPool->m_mutexThreadCounter));

		if (queueSize >= MIN_WAIT_TASK_NUM && liveThrNum < objThreadPool->m_iMaxNum)
		{
			std::cout << "11" << std::endl;
			pthread_mutex_lock(&(objThreadPool->m_mutexLock));
			int add = 0;

			for (i = 0; i < objThreadPool->m_iMaxNum && add < DEFAULT_THREAD_VARY && objThreadPool->m_iLiveNum < objThreadPool->m_iMaxNum; i++)
			{
				if (objThreadPool->m_vectorThreads[i] == NULL ||
					!objThreadPool->IsThreadAlive(objThreadPool->m_vectorThreads[i]))
				{
					pthread_create(&(objThreadPool->m_vectorThreads[i]), NULL,
								   ThreadPoolProc, (void *)objThreadPool);
					add++;
					objThreadPool->m_iLiveNum++;
				}
			}

			pthread_mutex_unlock(&(objThreadPool->m_mutexLock));
		}

		if ((busyThrNum * 2) < liveThrNum && liveThrNum > objThreadPool->m_iMinNum)
		{
			pthread_mutex_lock(&(objThreadPool->m_mutexLock));
			objThreadPool->m_iWaitExitNum = DEFAULT_THREAD_VARY;
			pthread_mutex_unlock(&(objThreadPool->m_mutexLock));

			for (i = 0; i < DEFAULT_THREAD_VARY; i++)
			{
				pthread_cond_signal(&(objThreadPool->m_condQueueNotFull));
			}
		}
	}

	return NULL;
}

CXMThreadPool::CXMThreadPool()
{
}

CXMThreadPool::~CXMThreadPool()
{
}

bool CXMThreadPool::Init(int min_thr_num, int max_thr_num)
{
	int i;
	do
	{
		m_iMinNum = min_thr_num;
		m_iMaxNum = max_thr_num;
		m_iBusyNum = 0;
		m_iLiveNum = min_thr_num;
		m_iWaitExitNum = 0;
		m_iQueueSize = 0;
		m_iShutDown = false;

		pthread_t threadid;

		for (i = 0; i < m_iMaxNum; i++)
		{
			threadid = 0;
			m_vectorThreads.push_back(threadid);
		}

		if (pthread_mutex_init(&m_mutexLock, NULL) != 0 || pthread_mutex_init(&m_mutexThreadCounter, NULL) != 0 || pthread_cond_init(&m_condQueueNotFull, NULL) != 0 || pthread_cond_init(&m_condQueueNotEmpty, NULL) != 0)
		{
			printf("init the lock or cond fail");
			break;
		}

		for (i = 0; i < m_iMinNum; i++)
		{
			pthread_create(&(m_vectorThreads[i]), NULL, ThreadPoolProc, (void *)this);
			std::cout << "start thread " << pthread_self() << "..." << std::endl;
		}

		pthread_create(&m_ManageThread, NULL, ThreadPoolManager, (void *)this);

		return true;

	} while (0);

	return false;
}

bool CXMThreadPool::UnInit()
{
	int i;
	m_iShutDown = true;
	pthread_join(m_ManageThread, NULL);

	for (i = 0; i < m_iLiveNum; i++)
	{
		pthread_cond_broadcast(&m_condQueueNotEmpty);
	}
	for (i = 0; i < m_iLiveNum; i++)
	{
		pthread_join(m_vectorThreads[i], NULL);
	}

	while (!m_queueTask.empty())
	{
		m_queueTask.pop();
	}

	XMVector<pthread_t>().swap(m_vectorThreads);

	pthread_mutex_lock(&m_mutexLock);
	pthread_mutex_destroy(&m_mutexLock);
	pthread_mutex_lock(&m_mutexThreadCounter);
	pthread_mutex_destroy(&m_mutexThreadCounter);
	pthread_cond_destroy(&m_condQueueNotFull);
	pthread_cond_destroy(&m_condQueueNotEmpty);

	return true;
}

bool CXMThreadPool::ThreadPoolAddTask(CProcUnitBase *procUnit)
{
	pthread_mutex_lock(&(m_mutexLock));

	if (m_iShutDown)
	{
		pthread_cond_broadcast(&(m_condQueueNotEmpty));
		pthread_mutex_unlock(&(m_mutexLock));
		return true;
	}

	m_queueTask.push(procUnit);
	m_iQueueSize++;

	pthread_cond_signal(&(m_condQueueNotEmpty));
	pthread_mutex_unlock(&(m_mutexLock));

	return true;
}

int CXMThreadPool::ThreadPoolAllThreadNum()
{
	int allThrNum = -1;
	pthread_mutex_lock(&(m_mutexLock));
	allThrNum = m_iLiveNum;
	pthread_mutex_unlock(&(m_mutexLock));
	return allThrNum;
}

int CXMThreadPool::ThreadPoolBusyThreadNum()
{
	int busyThrNum = -1;
	pthread_mutex_lock(&(m_mutexThreadCounter));
	busyThrNum = m_iBusyNum;
	pthread_mutex_unlock(&(m_mutexThreadCounter));
	return busyThrNum;
}

bool CXMThreadPool::IsThreadAlive(pthread_t tid)
{
	int killRc = pthread_kill(tid, 0);
	if (killRc == ESRCH)
	{
		return false;
	}
	return true;
}

class COutPutMsgProcUnit : public CProcUnitBase
{
public:
	COutPutMsgProcUnit(std::string &name);
	~COutPutMsgProcUnit();

	void Run(pthread_t threadID);
	void Exit();
	bool Chancel();
	bool GetStopState();

private:
	volatile pthread_t m_pThreadID;
	std::string m_name;
	volatile bool stop_flag;
};

COutPutMsgProcUnit::COutPutMsgProcUnit(std::string &name)
	: CProcUnitBase(name)
{
	m_name = name;
	stop_flag = false;
	m_pThreadID = 0;
}

void COutPutMsgProcUnit::Run(pthread_t threadID)
{
	m_pThreadID = threadID;
	while (1)
	{
		std::cout << m_name + " hahaha " << threadID << std::endl;

		sleep(1);
	}
}

bool COutPutMsgProcUnit::GetStopState()
{
	return stop_flag;
}

bool COutPutMsgProcUnit::Chancel()
{
	if (m_pThreadID != 0)
	{
		std::cout << m_name + " pthread_cancel " << m_pThreadID << std::endl;
		pthread_cancel(m_pThreadID);
		return true;
	}

	std::cout << m_name + "pthread_cancel wait" << std::endl;
	stop_flag = true;
	return false;	
}

void COutPutMsgProcUnit::Exit()
{
	if (m_pThreadID != 0)
	{
		std::cout << m_name + " pthread_cancel " << m_pThreadID << std::endl;
		pthread_cancel(m_pThreadID);
	}
	stop_flag = true;
}

COutPutMsgProcUnit::~COutPutMsgProcUnit()
{
}

XMMap<XMString, CProcUnitBase*> m_mapProcUnitObj;

bool DelProcUnit(XMString strName)
{
	int iCount = 0;
	iCount = m_mapProcUnitObj.count(strName);
	if (iCount != 0)
	{
		COutPutMsgProcUnit * objOutPutMsgProcUnit = (COutPutMsgProcUnit*)m_mapProcUnitObj[strName];

		if (objOutPutMsgProcUnit)
		{
			if(objOutPutMsgProcUnit->Chancel())
			{
				delete objOutPutMsgProcUnit;
			}						
		}

		m_mapProcUnitObj.erase(strName);
	}

	return true;
}
CXMThreadPool *m_objThreadPool;
bool AddProcUnit(XMString strName, CProcUnitBase *objProcUnit)
{
	if (!objProcUnit)
		return false;

	DelProcUnit(strName);
	m_mapProcUnitObj[strName] = objProcUnit;

	std::cout << "objProcUnit:" << static_cast<const void *>(objProcUnit) << "  " << static_cast<const void *>(&objProcUnit) <<std::endl;
	if (m_objThreadPool)
		m_objThreadPool->ThreadPoolAddTask(objProcUnit);

	return true;
}

int main()
{
	m_objThreadPool = new CXMThreadPool();
	m_objThreadPool->Init(5, 50);

	XMString name = "task";
	int i = 0;

	while (1)
	{
		char c = getchar();
		if(c == 'a')
		{
			XMString taskname = name + std::to_string(i++);
			COutPutMsgProcUnit *task = new COutPutMsgProcUnit(taskname);
			AddProcUnit(name , task);
		}
		else if(c == 'd')
		{
			DelProcUnit(name);
		}
		else if(c == 't')
		{
			XMString taskname = name + std::to_string(i++);
			COutPutMsgProcUnit *task = new COutPutMsgProcUnit(taskname);
			AddProcUnit(name , task);
			DelProcUnit(name);
		}
		else if(c == 'i')
		{
			std::cout << "ThreadPoolAllThreadNum:" << m_objThreadPool->ThreadPoolAllThreadNum() << std::endl;
			std::cout << "ThreadPoolBusyThreadNum:" << m_objThreadPool->ThreadPoolBusyThreadNum() << std::endl;
		}
	}

	return 0;
}
