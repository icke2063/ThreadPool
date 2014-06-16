/**
 * @file   BasePoolInt.h
 * @Author icke2063
 * @date   26.05.2013
 *
 * @brief	Base Interface for a "Threadpool". Therefore within this file the BasePool, WorkerThread and Functor
 * 		classes are defined. The Threadpool shall be used to store Functor
 * 		objects. These functors are handled by WorkerThreads.
 * 		The inherit class has to define the abstract functions with own or external thread and mutex
 * 		implementations (e.g. c++11, boost...).
 *
 * Copyright © 2013 icke2063 <icke2063@gmail.com>
 *
 * This framework is free software; you can redistribute it and/or
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

#ifndef BASEPOOLINT_H_
#define BASEPOOLINT_H_

//std libs
#include <stdint.h>
#include <deque>
#include <list>
#include <unistd.h>

#include <icke2063_TP_config.h>

#ifndef WORKERTHREAD_MAX
	#define WORKERTHREAD_MAX	60
#endif

#ifndef FUNCTOR_MAX
	#define FUNCTOR_MAX	1024
#endif

namespace icke2063 {
namespace threadpool {

///Functor for ThreadPool
/**
 * Inherit from this class then it can be added by ThreadPool::addFunctor with an implementation
 * of the ThreadPool class. This class has to implement the functor_function. All member functions and variables
 * are accessable by the functor_function. This function will be executed by the WorkerThreads
 *
 * @todo make sure that it cannot declared as static variable
 */
class FunctorInt {  
public:
	FunctorInt(){}
	virtual ~FunctorInt(){}
	/**
	 * Function called by WorkerThread
	 * @brief This function will be called by WorkerThreadInt of ThreadPool
	 */
	virtual void functor_function(void) = 0;
};

///WorkerThread of ThreadPool
/**
 * The inherit objects of this class are used to handle new functors from functor_queue
 * and call their functor function.
 * The implementation has to create for each object of this class an own thread.
 */

class WorkerThreadInt {
	friend class BasePoolInt;
public:
	/**
	 * Standard Constructor
	 * @param sp_reference:	reference object to BasePool
	 */
	WorkerThreadInt(){}

	virtual ~WorkerThreadInt(){}

protected:

	/**
	 * Worker thread function
	 * This is the "magic" function of worker thread object.
	 * This functions loops over the functor queue and if there is sth.
	 * to do DO IT ;-)
	 *
	 * Every functor has to be removed from list before handling it.
	 */
	virtual	void worker_function(void) = 0;
};


///Abstract ThreadPool interface
/**
 * This class list all useful/needed functions for a simple Threadpool implementation.
 */
class BasePoolInt {
	friend class WorkerThreadInt;	//let worker threads access this class
public:
	/**
	 * Base constructor for threadpool interface
	 * - depending classes should initiate worker threads
	 */
	BasePoolInt(){}

	/**
	 * Base destructor for threadpool interface
	 * - depending classes shall
	 * 		* reset worker threads
	 * 		* clear functor list
	 */
	virtual ~BasePoolInt(){}

	/**
	  * Delegate new functor to queue
	  * - lock list before access list items
	  * - add functor object to list
	  *
	  * @param work:	pointer to FunctorInt Object (will be deleted after use)
	  * @return		[success]: NULL
	  * 			[failure]: FunctorInt* of given object. The Functor was not added to list -> threadpool will not delete object
	  */
	virtual FunctorInt *delegateFunctor(FunctorInt *work) = 0;

	/**
	 * get current worker count within worker list
	 */
	size_t getWorkerCount(void){ return m_workerThreads.size(); }

	/**
	 * get current functor size of functor queue
	 */
	uint16_t getQueueCount(){ return m_functor_queue.size(); }

protected:

	/**
	 * add new worker to internal worker list
	 * @return true on success , else false
	 */
	virtual bool addWorker( void ) = 0;

	/**
	 * remove internal worker object
	 * - try to remove idle worker
	 * @return true on success , else false
	 */
	virtual bool delWorker( void ) = 0;

	/**
	 * remove all queued functor objects
	 * - lock functor queue
	 * - remove/delete each element of list
	 */
	virtual void clearQueue(void) = 0;

	/**
	 * remove all worker objects
	 * - remove/delete each element of list
	 */
	virtual void clearWorker(void) = 0;

protected:
	///list of waiting functors
	typedef std::deque<FunctorInt *> functor_queue_type;
	functor_queue_type  	m_functor_queue;

	///list of used WorkerThreadInts
	typedef std::list<WorkerThreadInt*> worker_list_type;
	worker_list_type	m_workerThreads;

};

} /* namespace threadpool */
} /* namespace icke2063 */
#endif /* BASEPOOLINT_H_ */
