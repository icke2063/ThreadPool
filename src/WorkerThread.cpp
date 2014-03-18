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

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || (__cplusplus >= 201103L)
  #include <memory>
  #include <chrono>
  using namespace std;
#else
  #include <boost/shared_ptr.hpp>
  using namespace boost;
#endif

#include "WorkerThread.h"

namespace icke2063 {
namespace threadpool {

WorkerThread::WorkerThread(shared_ptr<Ext_Ref<Ext_Ref_Int> > sp_reference):
	m_worker_running(true),
	m_status(worker_idle),
	sp_basepool(sp_reference) {

	WorkerThread_log_info("WorkerThread[%p] \n", this);

	// create new worker thread
	try {
		m_worker_thread.reset(new thread(&WorkerThread::worker_function, this));
	} catch (std::exception e) {
		WorkerThread_log_error("WorkerThread: init failure: %s\n",e.what());
		throw e;
	}
}

WorkerThread::~WorkerThread() {
	WorkerThread_log_info("~WorkerThread[%p]\n", this);

	m_worker_running = false; //disable worker thread

	/**
	 * wait for ending worker thread function
	 * @todo how to kill blocked thread function?
	 */
	while (m_status != worker_finished) {
		WorkerThread_log_trace("~wait for finish worker[%p]: %d\n", this);
		usleep(1000);
	}

//at this point the worker thread should have ended -> join it
	if (m_worker_thread.get()
			&& m_worker_thread->joinable()) {
		WorkerThread_log_trace("wait for finish worker[%p]: %d\n", this, w_count);
		m_worker_thread->join();
		WorkerThread_log_trace("~wait for finish worker[%p]: %d\n", this, w_count);
	} else {
		// kill thread function !!! but how ?
	}
	WorkerThread_log_debug("~~WorkerThread[%p]", this);

}

void WorkerThread::worker_function(void) {
	FunctorInt *curFunctor = NULL;
	while (m_worker_running) {
		{
			curFunctor = NULL;
			this_thread::yield();
			//this_thread::sleep_for(chrono::microseconds(1000));
			if (!m_worker_running){
				break;
			}

			if (sp_basepool.get()) {
				lock_guard<mutex> g(sp_basepool->getLock());
				ThreadPool *p_base = dynamic_cast<ThreadPool*>(sp_basepool->getRef());

				if (p_base) { //parent object valid
					lock_guard<mutex> lock(p_base->m_functor_lock); // lock before queue access
					if (!m_worker_running){
						break;
					}

					if (p_base->m_functor_queue.size() > 0) {
						curFunctor = p_base->m_functor_queue.front(); // get next functor from queue
						p_base->m_functor_queue.pop_front(); // remove functor from queue
					}

				} else {
					break;
				}
			}

			if (curFunctor != NULL) {
				//logger->debug("get next functor");
				m_status = worker_running; //
				WorkerThread_log_trace("curFunctor[%p]->functor_function();\n", curFunctor);
				curFunctor->functor_function(); // call handling function
				delete curFunctor;	//delete object
				m_status = worker_idle; //
			} else {
				usleep(100);
				m_status = worker_idle; //
			}
		}
	}

finish:
	WorkerThread_log_debug("exit worker_function[%p]\n", this);
	m_status = worker_finished;
	return; //running mode changed -> exit thread

}


} /* namespace common_cpp */
} /* namespace icke2063 */
