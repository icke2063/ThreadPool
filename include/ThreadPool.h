/**
 * @file   ThreadPool.h
 * @Author icke2063
 * @date   28.05.2013
 * @brief  ThreadPoolInt implementation with usage of c++11 threads, mutex,...
 *
 * Namespace switching: see README.md
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

#ifdef TP_NS
#error "namespace constant 'TP_NS' already defined"
#endif

#ifndef ICKE2063_THREADPOOL_NO_CPP11
	#include <memory>
	#include <mutex>
	#include <thread>

	#define TP_NS std
	#define OVERRIDE override
#else
	//C++03
	#include <auto_ptr.h>

	#include <boost/shared_ptr.hpp>
	#include <boost/thread/mutex.hpp>
	#include <boost/thread/thread.hpp>


	#define TP_NS boost
	#define OVERRIDE
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
#include "ThreadPoolInt/Ext_Ref.h"

#ifndef DEFAULT_MAIN_SLEEP_US
	#define DEFAULT_MAIN_SLEEP_US 1000
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

	DelayedFunctor(FunctorInt *functor, struct timeval *deadline) :
			DelayedFunctorInt(functor, deadline) {}

	virtual ~DelayedFunctor(){
		deleteFunctor();
	}
	/**
	 * get stored FunctorInt
	 * - lock list
	 * - release smartpointer
	 */
	virtual FunctorInt *releaseFunctor() OVERRIDE;

	/**
	 * delete stored FunctorInt
	 * - lock
	 * - reset smartpointer
	 */
	virtual void resetFunctor(FunctorInt *functor) OVERRIDE;

private:
	// lock for reference access
	TP_NS::mutex m_lock_functor;
};
#endif

class ThreadPool:
	public Ext_Ref_Int
	,public BasePoolInt
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

	 ///Implementations for BasePoolInt
	/**
	 * Add new functor object
	 * @param work pointer to functor object
	 */
#ifndef NO_PRIORITY_TP_SUPPORT
	FunctorInt *delegateFunctor(FunctorInt *work, uint8_t add_mode);

	virtual FunctorInt *delegateFunctor(FunctorInt *work) OVERRIDE {
		return delegateFunctor(work, TPI_ADD_Default);
	}
#else
	virtual FunctorInt *delegateFunctor(FunctorInt *work) OVERRIDE;
#endif

	/**
	 * get queue position of given Functor reference
	 */
	virtual int getQueuePos(FunctorInt *searchedFunctor);

#ifndef NO_DELAYED_TP_SUPPORT
	///Implementations for DelayedPoolInt
	virtual TP_NS::shared_ptr<DelayedFunctorInt> delegateDelayedFunctor(TPD_NS::shared_ptr<DelayedFunctorInt> dfunctor) OVERRIDE;
#endif
#ifndef NO_PRIORITY_TP_SUPPORT
	///Implementations for PrioPoolInt
	virtual FunctorInt *delegatePrioFunctor(FunctorInt *work) OVERRIDE;
#endif
protected:

	 ///Implementations for BasePoolInt
	virtual bool addWorker(void) OVERRIDE;
	virtual bool delWorker(void) OVERRIDE;
	virtual void clearQueue(void) OVERRIDE;
	virtual void clearWorker(void) OVERRIDE;

	///lock functor queue
	TP_NS::mutex	m_functor_lock;

	///lock worker queue
	TP_NS::mutex	m_worker_lock;
#ifndef NO_DELAYED_TP_SUPPORT

	///Implementations for DelayedPoolInt
	virtual void checkDelayedQueue(void) OVERRIDE;

	/**
	 *	clear delayed list
	 */
	virtual void clearDelayedList( void ) OVERRIDE;
#endif
#ifndef NO_DYNAMIC_TP_SUPPORT
	///Implementations for DynamicPoolInt
	/**
	 * Scheduler is used for creating and scheduling the WorkerThreads.
	 * - on high usage (many unhandled functors in queue) create new threads until HighWatermark limit
	 * - on low usage and many created threads -> delete some to save resources
	 */
	virtual void handleWorkerCount(void) OVERRIDE;
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

	std::auto_ptr<TP_NS::thread> m_main_thread;

	///running flag
	bool m_loop_running;

	/// sleep time between every loop of main thread
	uint32_t m_main_sleep_us;
};

} /* namespace threadpool */
} /* namespace icke2063 */
#endif /* THREADPOOL_H_ */
