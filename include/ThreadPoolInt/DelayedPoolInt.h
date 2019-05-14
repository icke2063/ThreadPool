/**
 * @file   DelayedThreadPool.h
 * @Author icke2063
 * @date   23.11.2013
 * @brief  	Interface for the delayed Threadpool extension
 * 		The idea is to have a extra list with Functor objects and a deadline timestamp.
 * 		If this deadline is over the Functor has to be added to normal Threadpool queue.
 * The function to check the timestamps has to be called continuously.
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

#ifndef _DELAYED_THREADPOOL_H_
#define _DELAYED_THREADPOOL_H_

#include <icke2063_TP_config.h>

#ifndef NO_DELAYED_TP_SUPPORT

//C++11
#include <memory>
#include <mutex>
#include <chrono>

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
   DelayedFunctorInt(FunctorInt *functor,
		   std::chrono::steady_clock::time_point &deadline):
    m_functor(functor),m_deadline(deadline){}
   
   /**
    * -delete Functor
    */
   virtual ~DelayedFunctorInt(){}
   
   /**
    * get functor deadline
    * - after this timestamp functor should be activated
    */
   std::chrono::steady_clock::time_point getDeadline(){return m_deadline;}
   
   /**
    * set Deadline to current time
    * - Functor should be activated immediately after this function call
    */
   void resetDeadline(){m_deadline =
		   std::chrono::steady_clock::time_point::max();}

   /**
   * set Deadline to given deadline
   */
  void renewDeadline(std::chrono::steady_clock::time_point &deadline)
  	  	  {m_deadline = deadline;}

   /**
    * get stored FunctorInt and release reference
    * - no deletion of functor
    */
   virtual FunctorInt *releaseFunctor() = 0;

   /**
    * reset stored FunctorInt
    * - deletion of functor
    */
   virtual void resetFunctor(FunctorInt *functor) = 0;

   /**
    * delete stored FunctorInt
    */
   void deleteFunctor( void ){
	   resetFunctor(NULL);
   }

 protected:   
   // storage for functor reference
   std::auto_ptr<FunctorInt> m_functor;

   /**
    * absolute timestamp after this deadline the functor should be added to threadpool
    */
   std::chrono::steady_clock::time_point m_deadline;
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
	 * @param[IN]	dfunctor	shared pointer to delayed functor object
	 *
	 * @return	[success] empty shared pointer, [failure] same pointer as input
	 */
	virtual std::shared_ptr<DelayedFunctorInt>
	delegateDelayedFunctor(std::shared_ptr<DelayedFunctorInt> dfunctor) = 0;

protected:

	/**
	 *	clear delayed list
	 */
	virtual void clearDelayedList( void ) = 0;

	///list of delayed functors
	typedef std::deque<std::shared_ptr<DelayedFunctorInt> > delayed_list_type;
	delayed_list_type m_delayed_queue;

	///lock functor queue
	std::mutex					m_delayed_lock;

};
} /* namespace threadpool */
} /* namespace icke2063 */

#endif /* _DELAYED_TP_SUPPORT */
#endif /* _DELAYED_THREADPOOL_H_ */
