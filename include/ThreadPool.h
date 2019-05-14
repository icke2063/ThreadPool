/**
 * @file   ThreadPool.h
 * @Author icke2063
 * @date   28.05.2013
 * @brief  ThreadPoolInt implementation with usage of c++11 threads, mutex,...
 *
 *
 * Copyright © 2013 icke2063 <icke2063@gmail.com>
 *
 * This software is free; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This framework is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <icke2063_TP_config.h>

#include <sys/time.h>
#include <pthread.h>


//C++11
#include <memory>
#include <mutex>

#ifndef TP_OVERRIDE
	#define TP_OVERRIDE override
#endif

//common_cpp
#include "ThreadPoolInt/BasePoolInt.h"
#ifndef NO_DELAYED_TP_SUPPORT
	#include "ThreadPoolInt/DelayedPoolInt.h"
#endif
#ifndef NO_DYNAMIC_TP_SUPPORT
	#include "ThreadPoolInt/DynamicPoolInt.h"
#endif
#include "ThreadPoolInt/PrioPoolInt.h"

#ifndef DEFAULT_TP_MAINLOOP_IDLE_US
	#define DEFAULT_TP_MAINLOOP_IDLE_US 1000
#endif

//logging macros
#ifndef ThreadPool_log_trace
	#define ThreadPool_log_trace(...)
#endif

#ifndef ThreadPool_log_debug
	#define ThreadPool_log_debug(...)
#endif

#ifndef ThreadPool_log_info
	#define ThreadPool_log_info(...)
#endif

#ifndef ThreadPool_log_error
	#define ThreadPool_log_error(...)
#endif

namespace icke2063 {
namespace threadpool {

class Functor:
	public FunctorInt
#ifndef NO_PRIORITY_TP_SUPPORT
	,public PrioFunctorInt
#endif
	{
public:
	Functor(){};
	virtual ~Functor(){};

};

#ifndef NO_DELAYED_TP_SUPPORT
///Implementations for DelayedFunctorInt
class DelayedFunctor: public DelayedFunctorInt {
public:

	DelayedFunctor(FunctorInt *functor, std::chrono::steady_clock::time_point &deadline) :
			DelayedFunctorInt(functor, deadline) {}

	virtual ~DelayedFunctor(){
		deleteFunctor();
	}
	/**
	 * get stored FunctorInt
	 * - lock list
	 * - release smart pointer
	 */
	virtual FunctorInt *releaseFunctor() TP_OVERRIDE;

	/**
	 * delete stored FunctorInt
	 * - lock
	 * - reset smart pointer
	 */
	virtual void resetFunctor(FunctorInt *functor) TP_OVERRIDE;

private:
	// lock for reference access
	std::mutex m_lock_functor;
};
#endif

class ThreadPool:
	public BasePoolInt
#ifndef NO_DELAYED_TP_SUPPORT
	,public DelayedPoolInt
#endif
#ifndef NO_DYNAMIC_TP_SUPPORT
	,public DynamicPoolInt
#endif
#ifndef NO_PRIORITY_TP_SUPPORT
	,public PrioThreadPoolInt
#endif
	{

	friend class WorkerThread;

/**
* addFunctor modes
*
*/
#define TPI_ADD_Default	0
#define TPI_ADD_FiFo	1
#define TPI_ADD_LiFo	2

public:
	ThreadPool(uint8_t worker_count = 1, bool auto_start = true);
	virtual ~ThreadPool();

	/**
	 *	Set idle time for main loop
	 *	- reduce cpu load
	 */
	void setTPMainLoopIdleTime(uint32_t main_idle_us);

	///Implementations for BasePoolInt
	/**
	 * Add new functor object
	 * @param work pointer to functor object
	 */
#ifndef NO_PRIORITY_TP_SUPPORT
	FunctorInt *delegateFunctor(FunctorInt *work, uint8_t add_mode);

	virtual FunctorInt *delegateFunctor(FunctorInt *work) TP_OVERRIDE
	{
		return delegateFunctor(work, TPI_ADD_Default);
	}
#else
	virtual FunctorInt *delegateFunctor(FunctorInt *work) TP_OVERRIDE;
#endif

	bool isPoolLoopRunning(){return m_loop_running;}
	/**
	 * get queue position of given Functor reference
	 */
	virtual int getQueuePos(FunctorInt *searchedFunctor);

#ifndef NO_DELAYED_TP_SUPPORT
	///Implementations for DelayedPoolInt
	virtual std::shared_ptr<DelayedFunctorInt> delegateDelayedFunctor(std::shared_ptr<DelayedFunctorInt> dfunctor) TP_OVERRIDE;
#endif
#ifndef NO_PRIORITY_TP_SUPPORT
	///Implementations for PrioPoolInt
	virtual FunctorInt *delegatePrioFunctor(FunctorInt *work) TP_OVERRIDE;
#endif
protected:

	 ///Implementations for BasePoolInt
	virtual bool addWorker(void) TP_OVERRIDE;
	virtual bool delWorker(void) TP_OVERRIDE;
	virtual void clearQueue(void) TP_OVERRIDE;
	virtual void clearWorker(void) TP_OVERRIDE;

	void wakeupWorker(void);

	///lock functor queue
	std::mutex	m_functor_lock;

	///lock worker queue
	std::mutex	m_worker_lock;

#ifndef NO_DELAYED_TP_SUPPORT

	///Implementations for DelayedPoolInt
	virtual void checkDelayedQueue(void) TP_OVERRIDE;

	/**
	 *	clear delayed list
	 */
	virtual void clearDelayedList( void ) TP_OVERRIDE;
#endif
#ifndef NO_DYNAMIC_TP_SUPPORT
	///Implementations for DynamicPoolInt
	/**
	 * Scheduler is used for creating and scheduling the WorkerThreads.
	 * - on high usage (many unhandled functors in queue) create new threads until HighWatermark limit
	 * - on low usage and many created threads -> delete some to save resources
	 */
	virtual void handleWorkerCount(void) TP_OVERRIDE;
#endif

	///own stuff to get the other stuff running

	///running flag for this threadpool
	bool m_pool_running;

  	/* function called before main thread loop */
	virtual void main_pre(void);
	/* function called within main thread loop */
	virtual void main_loop(void);
	/* function called after main thread loop */
	virtual void main_past(void);

	/**
	 * start internal thread
	 * - creates new thread on first call
	 * - start lop thread
	 * - set pool loop running flag
	 */
	bool startPoolLoop();

	/**
	 * stop internal thread
	 * - unset set pool loop running flag
	 * - no call of main_loop anymore (until startPoolLoop again)
	 */
	void stopPoolLoop();


	/**
	 * stop this threadpool
	 */
	void stopBasePool();

	/**
	 * main thread function
	 * - call main_pre and main_past once
	 * - continuously calling main_loop while running flag is true
	 */
	void main_thread_func(void);

private:
	static void* pthread_func(void * ptr);

protected:
	pthread_t id_main_thread = 0;

	///running flag
	bool m_loop_running;

	/// sleep time between every loop of main thread
	uint32_t m_main_idle_us;

	/// idle time for worker threads
	uint32_t m_worker_idle_us;

};

} /* namespace threadpool */
} /* namespace icke2063 */
#endif /* THREADPOOL_H_ */
