/**
 * @file   WorkerThread.h
 * @Author icke2063
 * @date   31.05.2013
 * @brief  WorkerThreadInt implementation with c++11 threads
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

#ifndef WORKERTHREAD_H_
#define WORKERTHREAD_H_

#include <icke2063_TP_config.h>

#include <pthread.h>

//C++11
#include <memory>

#ifndef TP_OVERRIDE
	#define TP_OVERRIDE override
#endif

//common_cpp
#include <ThreadPool.h>

//logging macros
#ifndef WorkerThread_log_trace
	#define WorkerThread_log_trace(...)
#endif

#ifndef WorkerThread_log_debug
	#define WorkerThread_log_debug(...)
#endif

#ifndef WorkerThread_log_info
	#define WorkerThread_log_info(...)
#endif

#ifndef WorkerThread_log_error
	#define WorkerThread_log_error(...)
#endif


#ifndef DEFAULT_WORKER_IDLE_US
	#define DEFAULT_WORKER_IDLE_US 1000
#endif

namespace icke2063 {
namespace threadpool {

class WorkerThread: public WorkerThreadInt {
	friend class ThreadPool;
public:
	WorkerThread(BasePoolInt *ref_pool
			, uint32_t worker_idle_us = DEFAULT_WORKER_IDLE_US);

	virtual ~WorkerThread();

	/**
	 * reset reference to Threadpool base object
	 */
	void resetBaseRef( void );

	/**
	 * enumeration of WorkerThread states
	 */
	enum worker_status{
		worker_idle=0x00,   	//!< worker_idle
		worker_running=0x01,	//!< worker_running
		worker_finished=0x02	//!< worker_finished
	};

	/**
	 * return status of current Worker object
	 * @return status value
	 */
	enum worker_status getStatus(){return m_status;}

	/**
	 * signal idle worker to wakeup
	 */
	bool wakeupWorker( void );

private:

	/**
	 * Status of current WorkerThread
	 * This Threadpool has the ability to create/destroy WorkerThread objects.
	 * The current solution is to set a status value at each worker to let
	 * the scheduler decide which worker can be destroyed.
	 */
	worker_status m_status;					//status of current thread

	virtual void worker_function( void ) TP_OVERRIDE;

	static void* pthread_func(void * ptr);

  	/**
	 * worker thread object
	 */
	pthread_t id_worker_thread = 0;

	/**
	 * condition to wait/signal
	 */
	pthread_cond_t m_worker_cond = PTHREAD_COND_INITIALIZER;

	/**
	 * running flag for worker thread
	 *
	 */
	bool m_worker_running;

	/**
	 * flag for fast shutdown of this WorkerThread
	 */
	bool m_fast_shutdown;

	/**
	 * reference to basepool object
	 */
	BasePoolInt *p_basepool;
	std::mutex pool_lock;

};

} /* namespace common_cpp */
} /* namespace icke2063 */
#endif /* WORKERTHREAD_H_ */
