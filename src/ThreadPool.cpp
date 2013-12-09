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

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || (__cplusplus >= 201103L)
  #include <memory>
  #include <chrono>
  #include <thread>
  using namespace std;
#endif


#include <boost/concept_check.hpp>

//common_cpp
#include "../include/ThreadPool.h"
#include "../include/WorkerThread.h"

namespace icke2063 {
namespace threadpool {

ThreadPool::ThreadPool(uint8_t worker_count):
	BasePoolInt(worker_count){
	/**
	 * Init Logging
	 * - set category name
	 * - connect with console
	 * - set loglevel
	 * @todo do this by configfile
	 */
	ThreadPool_log_info("ThreadPool");
}

ThreadPool::~ThreadPool() {
	ThreadPool_log_info("~ThreadPool");
	stopBasePool();
	ThreadPool_log_info("~ThreadPool");

}

void ThreadPool::main_pre(void){ 
}

void ThreadPool::main_loop(void){
	if(m_main_running){
		handleWorkerCount();
		checkDelayedQueue();
	}
}

void ThreadPool::main_past(void){
}

bool ThreadPool::addFunctor(shared_ptr<FunctorInt> work, uint8_t add_mode){
	ThreadPool_log_debug("add Functor #%i", m_functor_queue->size() + 1);
	
	shared_ptr<Functor> tmp_functor = dynamic_pointer_cast<Functor>(work);
	if(!tmp_functor.get())return false;
	
	switch(add_mode){
	  case TPI_ADD_LiFo:
	  {
	    tmp_functor->setPriority(100);	//set highest priority to hold list in order	
	    BasePoolInt::addFunctor(work);
	    return true;
	  }
	    break;
	  case TPI_ADD_FiFo:
	  {
	   tmp_functor->setPriority(0);	//set lowest priority to hold list in order 
	   BasePoolInt::addFunctor(work);
	   return true;
	  }
	  case TPI_ADD_Prio:
	  default:
	    return addPrioFunctor(tmp_functor);
	}
	return false;
}

bool ThreadPool::addPrioFunctor(shared_ptr<PrioFunctorInt> work){
  ThreadPool_log_debug("add priority Functor #%i", m_functor_queue->size() + 1);
 
  lock_guard<mutex> lock(*m_functor_lock.get());	//lock functor list
  
//   dynamic cast Functor object reference to determine correct type */
  shared_ptr<FunctorInt> addable = dynamic_pointer_cast<FunctorInt>(work);
  if(!addable.get())return false;
  
  std::deque<shared_ptr<FunctorInt> >::iterator queue_it = m_functor_queue.begin();
  while(queue_it != m_functor_queue.end()){
    
    {
      shared_ptr<PrioFunctorInt> queue_item = dynamic_pointer_cast<PrioFunctorInt>(*queue_it);
      shared_ptr<PrioFunctorInt> param_item = dynamic_pointer_cast<PrioFunctorInt>(work);
      
      if(queue_item.get() && param_item.get()){
	if(queue_item->getPriority() < param_item->getPriority()){
	  m_functor_queue.insert(queue_it,addable);	//insert before
	  return true;
	}
      }
    }
    ++queue_it;
  }
  // not inserted yet -> push it at the end
  m_functor_queue.push_back(addable);
  
  return true;
  }


void ThreadPool::handleWorkerCount(void){
 
  // check functor list
  if (m_functor_lock.get() != NULL && (m_functor_lock.get() != NULL)) { 
    lock_guard<mutex> lock(*m_functor_lock.get()); // lock before queue access

    ThreadPool_log_debug("m_functor_queue.size(): %d", m_functor_queue->size());
    ThreadPool_log_debug("m_workerThreads.size(): %d", m_workerThreads.size());
    ThreadPool_log_debug("lowWatermark(): %d", getLowWatermark());
    ThreadPool_log_debug("HighWatermark(): %d", getHighWatermark());
    ThreadPool_log_debug("max_queue_size: %i", max_queue_size);

    // add needed worker threads
    while (getWorkerCount() < getLowWatermark()) {
      //add new worker thread
      ThreadPool_log_debug("try add worker\n");
      if(!addWorker())break;
      ThreadPool_log_debug("new worker (under low): %i of %i", m_workerThreads.size(), getHighWatermark());
    }

//     add ondemand worker threads
    if (m_functor_queue.size() > max_queue_size && getWorkerCount() < getHighWatermark()) {
      //added new worker thread
      if(addWorker()){
	ThreadPool_log_debug("new worker (ondemand): %i of %i", m_workerThreads.size(), getHighWatermark());
	}
      }

//  remove worker threads
  if (m_functor_queue.size() == 0 && getWorkerCount() > getLowWatermark()) {
	  delWorker();
  }

} else {
	ThreadPool_log_error("m_functor_lock.get() == NULL");
}  
  
  max_queue_size = (1 << getWorkerCount());			//calc new maximum waiting functor count
}

DelayedPoolInt::DelayedPoolInt(){
    ///list of delayed functors

    //lock functor queue
    m_delayed_lock.reset(new mutex());
}


void ThreadPool::checkDelayedQueue(void){
	  //std::mutex *mut = (Mutex *)m_delayed_lock.get()
	  
	  lock_guard<mutex> lock( *m_delayed_lock.get());		//lock 
	  
	  ThreadPool_log_debug("m_delayed_queue.size():%d\n",m_delayed_queue->size());
	  
	  struct timeval timediff, tnow;
	  if( gettimeofday(&tnow, 0) != 0){
	     return;
	  }
	    long msec;
	  
	  std::deque<shared_ptr<DelayedFunctorInt> >::iterator delayed_it = m_delayed_queue.begin();
	  while(delayed_it != m_delayed_queue.end()){
	      
	    msec=(tnow.tv_sec-(*delayed_it)->getDeadline().tv_sec)*1000;
	    msec+=(tnow.tv_usec-(*delayed_it)->getDeadline().tv_usec)/1000;	    
	    
	    if(msec >= 0){
	      // add current functor to queue & remove from delayed queue
	      addFunctor((*delayed_it)->getFunctor());
	      delayed_it = m_delayed_queue.erase(delayed_it);
	      continue;
	    }
	    ++delayed_it;
	  }
	  
	}


shared_ptr<DelayedFunctorInt> ThreadPool::addDelayedFunctor(shared_ptr<FunctorInt> work, struct timeval deadline){
	ThreadPool_log_info("add DelayedFunctor #%i", m_delayed_queue->size() + 1);
	lock_guard<mutex> lock(*m_delayed_lock.get());
	shared_ptr<DelayedFunctorInt> tmp_functor = shared_ptr<DelayedFunctorInt>(new DelayedFunctorInt(work, deadline));
	
	m_delayed_queue.push_back(tmp_functor);
	return tmp_functor;
}

} /* namespace common_cpp */
} /* namespace icke2063 */
