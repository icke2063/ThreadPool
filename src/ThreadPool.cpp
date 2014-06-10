/**
 * @file   ThreadPool.cpp
 * @Author icke
 * @date   28.05.2013
 * @brief  ThreadPool implementation
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

//generic
#include <iostream>
#include <functional>
#include <vector>
#include <algorithm>
#include <string>
#include <stdexcept>

//common_cpp
#include "../include/ThreadPool.h"
#include "../include/WorkerThread.h"

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || (__cplusplus >= 201103L)
	using namespace std;
#else
	using namespace boost;
#endif

namespace icke2063 {
namespace threadpool {

ThreadPool::ThreadPool(uint8_t worker_count, bool auto_start):
#ifndef NO_DYNAMIC_TP_SUPPORT
		DynamicPoolInt(worker_count, worker_count>1?true:false),
#endif
		m_pool_running(true)
		,m_main_sleep_us(DEFAULT_MAIN_SLEEP_US)

{
	int add_worker_count = 0;

	ThreadPool_log_info("ThreadPool[%p]\n", (void*)this);

	/* init all shared objects with new objects */
	//m_functor_lock.reset(new mutex()); //init mutex for functor list
	//m_worker_lock.reset(new mutex()); //init mutex for worker list

	addWorker(); //add at least one worker thread failed -> threadpool not usable -> throw exception


	while (add_worker_count++ < WORKERTHREAD_MAX
			&& m_workerThreads.size() < worker_count) {
		if(!addWorker())break;	// break on failure
	}

	if(m_workerThreads.size() < 1){
		throw std::runtime_error("icke2063::ThreadPool: Cannot create Worker\n");
	}

	// auto start pool loop
	if (auto_start) {
		ThreadPool_log_debug("auto start Pool Loop\n");
		startPoolLoop();
	}
}

ThreadPool::~ThreadPool() {
	ThreadPool_log_info("~ThreadPool[%p]\n", (void*)this);
	m_pool_running = false; //disable ThreadPool

	if (m_main_thread.get() && m_main_thread->joinable()) {
			m_main_thread->join();
	}
	m_main_thread.reset();

#ifndef NO_DELAYED_TP_SUPPORT
	///DelayedPoolInt
	clearDelayedList();
#endif

	///BasePoolInt
	clearQueue();
	clearWorker();

	ThreadPool_log_info("~~ThreadPool[%p]", (void*)this);
}


bool ThreadPool::startPoolLoop() {
	if(m_pool_running){
		m_loop_running = true;
		if (m_main_thread.get() == NULL) {
			try {
				ThreadPool_log_debug("new main thread[%p]\n", (void*)this);
				m_main_thread.reset(new thread(&ThreadPool::main_thread_func, this)); // create new main thread_function
			} catch (std::exception& e) {
				ThreadPool_log_error("init main_thread failure: %s", e.what());
				m_loop_running = false;
				return false;
			}
		}
	}
	return true;
}

void ThreadPool::stopPoolLoop() {
	//set running flag for main_loop function
	m_loop_running = false;
}

void ThreadPool::main_thread_func(void){
    main_pre();
    while (m_pool_running) {
	this_thread::yield();
      if(m_pool_running){
    	  ThreadPool_log_trace("main_loop\n");
    	  main_loop();
      }
      this_thread::sleep_for(chrono::microseconds(m_main_sleep_us));
    }
    main_past();
}

void ThreadPool::main_pre(void){ 
}

void ThreadPool::main_loop(void) {
#ifndef NO_DYNAMIC_TP_SUPPORT
	if (isDynEnabled()) {
		/* dynamic worker handling enabled -> handle current worker count */
		handleWorkerCount();
	}
#endif
#ifndef NO_DELAYED_TP_SUPPORT
	checkDelayedQueue();
#endif
}

void ThreadPool::main_past(void){
}
#ifndef NO_PRIORITY_TP_SUPPORT
///advanced implemenatation of default function
bool ThreadPool::addFunctor(FunctorInt *work, uint8_t add_mode) {

	if (m_pool_running && (m_functor_queue.size() < FUNCTOR_MAX)) {
		ThreadPool_log_debug("add Functor #%i\n", (int)m_functor_queue.size() + 1);
		Functor *tmp_functor = dynamic_cast<Functor*>(work);
		if (!tmp_functor)
			return false;

		switch (add_mode) {
		case TPI_ADD_LiFo: {
			ThreadPool_log_debug("TPI_ADD_LiFo\n");
			tmp_functor->setPriority(100); //set highest priority to hold list in order
			lock_guard<mutex> lock(m_functor_lock);
			m_functor_queue.push_front(work);
			return true;
		}
			break;
		case TPI_ADD_FiFo: {
			ThreadPool_log_debug("TPI_ADD_FiFo\n");
			tmp_functor->setPriority(0); //set lowest priority to hold list in order
			lock_guard<mutex> lock(m_functor_lock);
			m_functor_queue.push_back(work);
			return true;
		}
		case TPI_ADD_Prio:
			ThreadPool_log_debug("TPI_ADD_Prio\n");
		default:
			return addPrioFunctor(tmp_functor);
		}
	}


	ThreadPool_log_error("failure add Functor #%i\n", (int)m_functor_queue.size() + 1);

	return false;
}
#else
bool ThreadPool::addFunctor(FunctorInt *work) {
	lock_guard<mutex> lock(m_functor_lock);
	m_functor_queue.push_back(work);
	return true;
}
#endif

int ThreadPool::getQueuePos(FunctorInt *searchedFunctor) {
	int pos = -1;
	lock_guard<mutex> lock(m_functor_lock); //lock functor list

	functor_queue_type::iterator queue_it = m_functor_queue.begin();
	while (queue_it != m_functor_queue.end()) {
		pos++;	//raise
		if (*queue_it == searchedFunctor) {
			//found matching reference -> exit function
			return pos;
		}
		++queue_it;
	}

	return -1;
}


#ifndef NO_PRIORITY_TP_SUPPORT
bool ThreadPool::addPrioFunctor(PrioFunctorInt *work){
  ThreadPool_log_debug("add priority Functor #%i\n", (int)m_functor_queue.size() + 1);
 
  lock_guard<mutex> lock(m_functor_lock);	//lock functor list
  
//   dynamic cast Functor object reference to determine correct type */
  FunctorInt *addable = dynamic_cast<FunctorInt*>(work);
  if(!addable)return false;
  
  functor_queue_type::iterator queue_it = m_functor_queue.begin();
	while (queue_it != m_functor_queue.end()) {

		{
			PrioFunctorInt *queue_item = dynamic_cast<PrioFunctorInt*>(*queue_it);
			PrioFunctorInt *param_item = dynamic_cast<PrioFunctorInt*>(work);

			if (queue_item && param_item) {
				if (queue_item->getPriority() < param_item->getPriority()) {
					m_functor_queue.insert(queue_it, addable); //insert before
					return true;
				}
			}
		}
		++queue_it;
	}

  ThreadPool_log_debug("push it at the end\n");
  // not inserted yet -> push it at the end
  m_functor_queue.push_back(addable);
  
  return true;
  }
#endif

bool ThreadPool::addWorker(void) {
	if (m_pool_running && m_workerThreads.size() < WORKERTHREAD_MAX) {
		ThreadPool_log_debug("addworker[%p] #%d\n", (void*)this, (int)m_workerThreads.size() +1);
		lock_guard<mutex> lock(m_worker_lock); // lock before worker list access
		try {
			WorkerThreadInt *newWorker = new WorkerThread(sp_reference);
			m_workerThreads.push_back(newWorker);
		} catch (std::exception& e) {;
			ThreadPool_log_error("addworker: failure: %s\n",e.what());
			return false;
		}
		return true;
	}
	return false;
}

bool ThreadPool::delWorker(void) {
	WorkerThreadInt *deleteWorker = NULL;

	if (m_pool_running) {
		lock_guard<mutex> lock(m_worker_lock); // lock before worker access
		worker_list_type::iterator workerThreads_it = m_workerThreads.begin();
		while (workerThreads_it != m_workerThreads.end()) {
			WorkerThread *tmpWorker = dynamic_cast<WorkerThread*>(*workerThreads_it);

			if (tmpWorker && tmpWorker->getStatus() == WorkerThread::worker_idle) {
				m_workerThreads.erase(workerThreads_it);
				deleteWorker = *workerThreads_it;
				break;
			}
			++workerThreads_it;
		}
	}

	if(deleteWorker){
		delete deleteWorker;
		return true;
	}

	return false;
}

void ThreadPool::clearQueue(void){
	lock_guard<mutex> g(m_functor_lock);

	functor_queue_type::iterator queue_it = m_functor_queue.begin();
	while(queue_it != m_functor_queue.end()){
		delete *queue_it;
		queue_it = m_functor_queue.erase(queue_it);
	}
}

void ThreadPool::clearWorker(void){
	lock_guard<mutex> g(m_worker_lock);

	worker_list_type::iterator worker_it = m_workerThreads.begin();
	while(worker_it != m_workerThreads.end()){

		WorkerThread *worker = dynamic_cast<WorkerThread*>(*worker_it);
		if(worker){
			worker->m_fast_shutdown = true;
		}
		delete *worker_it;
		worker_it = m_workerThreads.erase(worker_it);
	}
}
#ifndef NO_DYNAMIC_TP_SUPPORT
void ThreadPool::handleWorkerCount(void) {
	bool deleteWorker = false;

	{
		// check functor list
		lock_guard<mutex> lock(m_functor_lock); // lock before queue access

		ThreadPool_log_trace("m_functor_queue.size(): %d\n", m_functor_queue.size());
		ThreadPool_log_trace("m_workerThreads.size(): %d\n", m_workerThreads.size());
		ThreadPool_log_trace("lowWatermark(): %d\n", getLowWatermark());
		ThreadPool_log_trace("HighWatermark(): %d\n", getHighWatermark());
		ThreadPool_log_trace("max_queue_size: %i\n", max_queue_size);
	}
	// add needed worker threads
	while (getWorkerCount() < getLowWatermark()) {
		//add new worker thread
		ThreadPool_log_debug("try add worker\n");
		if (!addWorker()){
			//adding not successful -> exit loop
			break;
		}
		ThreadPool_log_debug("new worker (under low): %i of %i\n", (int)m_workerThreads.size(), (int)getHighWatermark());
	}

	{
		lock_guard<mutex> lock(m_functor_lock); // lock before queue access

		// add ondemand worker threads
		if (m_functor_queue.size() > max_queue_size
				&& getWorkerCount() < getHighWatermark()) {
			//added new worker thread
			if (addWorker()) {
				ThreadPool_log_debug("new worker (ondemand): %i of %i\n", (int)m_workerThreads.size(), (int)getHighWatermark());
			}
		}
	}

	{
		lock_guard<mutex> lock(m_functor_lock); // lock before queue access
		//  try to remove worker threads
		if (m_functor_queue.size() == 0
				&& getWorkerCount() > getLowWatermark()) {
			deleteWorker = true;
		}
	}


	if(deleteWorker){
		delWorker();
	}

	max_queue_size = (1 << getWorkerCount()); //calc new maximum waiting functor count
}
#endif

#ifndef NO_DELAYED_TP_SUPPORT
void ThreadPool::checkDelayedQueue(void){
	  //std::mutex *mut = (Mutex *)m_delayed_lock.get()
	  
	  lock_guard<mutex> lock(m_delayed_lock);		//lock
	  
	  ThreadPool_log_trace("m_delayed_queue.size():%d\n",m_delayed_queue.size());
	  
	  struct timeval tnow;
	  if( gettimeofday(&tnow, 0) != 0){
	     return;
	  }
	    long msec;
	  
	  delayed_list_type::iterator delayed_it = m_delayed_queue.begin();
	  while(delayed_it != m_delayed_queue.end()){
	      
	    msec=(tnow.tv_sec-(*delayed_it)->getDeadline().tv_sec)*1000;
	    msec+=(tnow.tv_usec-(*delayed_it)->getDeadline().tv_usec)/1000;	    
	    
	    if(msec >= 0){

	    	//get functor reference and remove from list
	    	FunctorInt* p_tmp_Functor = (*delayed_it)->releaseFunctor();
	    	delayed_it = m_delayed_queue.erase(delayed_it);

	      // add current functor to queue
	      if(!addFunctor(p_tmp_Functor)){
	    	  //adding not successful -> readd to front of delayed list
	    	  m_delayed_queue.push_front(shared_ptr<DelayedFunctorInt>(new DelayedFunctor(p_tmp_Functor, &tnow)));
	      }
	      continue;
	    }
	    ++delayed_it;
	  }
	  
	}

void ThreadPool::clearDelayedList( void ){
	lock_guard<mutex> g(m_delayed_lock);
	m_delayed_queue.clear();
}


shared_ptr<DelayedFunctorInt> ThreadPool::addDelayedFunctor(FunctorInt *work, struct timeval *deadline){

	shared_ptr<DelayedFunctorInt> tmp_functor;

	if(m_delayed_queue.size() < DELAYED_FUNCTOR_MAX){
		ThreadPool_log_trace("add DelayedFunctor #%i", m_delayed_queue.size() + 1);
		lock_guard<mutex> lock(m_delayed_lock);
		tmp_functor = shared_ptr<DelayedFunctorInt>(new DelayedFunctor(work, deadline));

		m_delayed_queue.push_back(tmp_functor);
		return tmp_functor;

	}

	ThreadPool_log_error("failure add DelayedFunctor #%d", (int)m_delayed_queue.size() + 1);
	return tmp_functor;
}
#endif

} /* namespace common_cpp */
} /* namespace icke2063 */
