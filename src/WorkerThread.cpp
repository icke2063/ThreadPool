/**
 * @file   WorkerThread.cpp
 * @Author icke2063
 * @date   31.05.2013
 * @brief  WorkerThreadInt implementation
 *
 * Copyright © 2013,2014 icke2063 <icke2063@gmail.com>
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

#include "WorkerThread.h"

namespace icke2063 {
namespace threadpool {

WorkerThread::WorkerThread(BasePoolInt *ref_pool, uint32_t worker_idle_us):
	m_status(worker_idle),
	m_worker_running(true),
	m_fast_shutdown(false),
	p_basepool(ref_pool)
{

	int result = 0;

	WorkerThread_log_info("WorkerThread[%p] \n", (void*)this);

	// create new worker thread
	if ( 0 != ( result = pthread_create(&id_worker_thread, NULL, pthread_func, this)) )
	{
		ThreadPool_log_error("create worker thread failure: %i \n", result);
		throw std::runtime_error("create worker thread failure");
	}

	// create new thread condition
	if ( 0 != ( result = pthread_cond_init(&m_worker_cond, NULL)) )
	{
		WorkerThread_log_error("WorkerThread: create condition failure: %i\n", result);
		throw std::runtime_error("cannot create condition");
	}
}

WorkerThread::~WorkerThread()
{
	int wait_count=0;
	WorkerThread_log_info("~WorkerThread[%p]\n", (void*)this);

	resetBaseRef();

	m_worker_running = false; //disable worker thread
	pthread_cond_signal(&m_worker_cond);

	/**
	 * wait for ending worker thread function
	 * @todo how to kill blocked thread function?
	 */
	while (m_status != worker_finished
			&& !(m_fast_shutdown
					&& wait_count++ > 1000))
	{
		WorkerThread_log_trace("~wait for finish worker[%p]\n", this);
		usleep(100);
	}

	if (m_status == worker_finished)
	{
		//at this point the worker thread should have ended -> join it
		if (id_worker_thread > 0 )
		{
			pthread_join(id_worker_thread, NULL);
			pthread_cond_destroy(&m_worker_cond);
		}
	}
	else
	{
		WorkerThread_log_error("Reset unfinished thread[%p]\n", (void*)this);
		/// @todo[icke2063] how to kill a thread by thread id?
	}
	WorkerThread_log_debug("~~WorkerThread[%p]\n", (void*)this);

}

void WorkerThread::worker_function(void)
{
	FunctorInt *curFunctor = NULL;
	pthread_mutex_t fakeMutex = PTHREAD_MUTEX_INITIALIZER;

	while (m_worker_running)
	{
		{
			curFunctor = NULL;
			pthread_yield();
			if (!m_worker_running)
			{
				break;
			}

			{
				std::lock_guard<std::mutex> g(pool_lock);
				ThreadPool *p_base = dynamic_cast<ThreadPool*>(p_basepool);

				if (p_base)
				{ //parent object valid
					std::lock_guard<std::mutex> lock(p_base->m_functor_lock); // lock before queue access
					if (m_worker_running)
					{
						if (p_base->m_functor_queue.size() > 0)
						{
							curFunctor = p_base->m_functor_queue.front(); // get next functor from queue
							p_base->m_functor_queue.pop_front(); // remove functor from queue
						}
					}
				}
				else
				{
					m_worker_running = false;
				}
			}

			//exit loop
			if (!m_worker_running)
			{
				break;
			}


			if (curFunctor != NULL)
			{
				//logger->debug("get next functor");
				m_status = worker_running; //
				WorkerThread_log_trace("curFunctor[%p]->functor_function();\n", curFunctor);
				try
				{
					curFunctor->functor_function(); // call handling function
				}
				catch (...)
				{
					WorkerThread_log_error("Exception in functor_function();\n");
				}
				delete curFunctor;	//delete object
			}
			else
			{
				//nothing to do -> wait for work (reduce cpu load)
				m_status = worker_idle;
			    pthread_mutex_lock(&fakeMutex);
				pthread_cond_wait(&m_worker_cond, &fakeMutex);
				pthread_mutex_unlock(&fakeMutex);
			}
		}
	}

	WorkerThread_log_debug("exit worker_function[%p]\n", (void*)this);
	m_status = worker_finished;
	return; //running mode changed -> exit thread
}

bool WorkerThread::wakeupWorker( void )
{
	if ( m_status == worker_idle )
	{
		if ( 0 == pthread_cond_signal(&m_worker_cond))
		{
			return true;
		}
	}
	return false;
}

void* WorkerThread::pthread_func(void * ptr)
{
	WorkerThread* p_self = dynamic_cast<WorkerThread*>((WorkerThread*)ptr);

	if( p_self == NULL )
	{
		return NULL;
	}

	p_self->worker_function();

	return NULL;
}

void WorkerThread::resetBaseRef( void )
{
	std::lock_guard<std::mutex> g(pool_lock);
	p_basepool = NULL;

}

} /* namespace common_cpp */
} /* namespace icke2063 */
