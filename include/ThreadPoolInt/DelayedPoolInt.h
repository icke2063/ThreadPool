/**
 * @file   DelayedThreadPool.h
 * @Author icke2063
 * @date   23.11.2013
 * @brief  	Interface for the delayed Threadpool extension
 * 		The idea is to have a extra list with Functor objects and a deadline timestamp.
 * 		If this deadline is over the Functor has to be added to normal Threadpool queue.
 * The function to check the timestamps has to be called continuously.
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

#ifndef _DELAYED_THREADPOOL_H_
#define _DELAYED_THREADPOOL_H_

#include <config.h>

#ifndef NO_DELAYED_TP_SUPPORT

#include <sys/time.h>

#ifdef TPD_NS
#error "namespace constant 'TPD_NS' already defined"
#endif

#ifndef ICKE2063_THREADPOOL_NO_CPP11
	#include <memory>
	#include <mutex>
	#define TPD_NS std
#else
	#include <boost/thread/mutex.hpp>
	#include <boost/shared_ptr.hpp>
	#define TPD_NS boost
#endif

#include "BasePoolInt.h"

#ifndef DELAYED_FUNCTOR_MAX
	#define DELAYED_FUNCTOR_MAX	1024
#endif


namespace icke2063 {
namespace threadpool {

  
class DelayedFunctorInt{
 public:
   /**
    * default constructor
    * - store FunctorInt object reference
    * - set deadline
    */
   DelayedFunctorInt(FunctorInt *functor, struct timeval *deadline):
    m_functor(functor),m_deadline(*deadline){}
   
   /**
    * -delete Functor
    */
   virtual ~DelayedFunctorInt(){}
   
   /**
    * get functor deadline
    * - after this timestamp functor should be activated
    */
   struct timeval getDeadline(){return m_deadline;}
   
   /**
    * set Deadline to current time
    * - Functor should be activated immediately after this function call
    */
   void resetDeadline(){gettimeofday(&m_deadline,0);}

   /**
   * set Deadline to given deadline
   */
  void renewDeadline(struct timeval deadline){m_deadline = deadline;}

   /**
    * get stored FunctorInt
    * - no deletion of functor
    */
   virtual FunctorInt *releaseFunctor() = 0;

   /**
    * delete stored FunctorInt
    */
   virtual void deleteFunctor( void ) = 0;

 protected:   
   // storage for functor reference
   FunctorInt *m_functor;

   /**
    * absolute timestamp after this deadline the functor should be added to threadpool
    */
   struct timeval m_deadline;
};

class DelayedPoolInt{ 
public:  
	DelayedPoolInt(){};

	/**
	 * - clear delayed list
	 */
	virtual ~DelayedPoolInt(){}

	/**
	 * get current count of queued/delayed functor objects
	 */
	size_t getDQueueCount(){ return m_delayed_queue.size(); }

	/**
	 * 
	 * check queue with stored DelayedFunctorInt for their deadline
	 * expired deadlines should be added to ThreadPool
	 */
	virtual void checkDelayedQueue(void) = 0;
	
	/**
	 * Add new functor object with given deadline
	 * @param work: pointer to functor object
	 * @param deadline: pointer to functor object
	 */
	virtual TPD_NS::shared_ptr<DelayedFunctorInt> addDelayedFunctor(FunctorInt *work, struct timeval *deadline) = 0;

protected:

	/**
	 *	clear delayed list
	 */
	virtual void clearDelayedList( void ) = 0;

	///list of delayed functors
	typedef std::deque<TPD_NS::shared_ptr<DelayedFunctorInt> > delayed_list_type;
	delayed_list_type m_delayed_queue;

	///lock functor queue
	TPD_NS::mutex					m_delayed_lock;

};
} /* namespace threadpool */
} /* namespace icke2063 */

#endif /* _DELAYED_TP_SUPPORT */
#endif /* _DELAYED_THREADPOOL_H_ */
