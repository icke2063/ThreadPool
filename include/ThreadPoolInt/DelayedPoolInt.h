/**
 * @file   DelayedThreadPool.h
 * @Author icke2063
 * @date   23.11.2013
 * @brief  	Interface for the delayed Threadpool extension
 * 		The idea ist to have a extra list with Functor objects and a deadline timestamp.
 * 		If this deadline is over the Functor has to be added to normal Threadpool queue.
 * The function to check the timestamps has to be called continously.
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

#include <sys/time.h>

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || (__cplusplus >= 201103L)
  #include <memory>
  #include <mutex>
  using namespace std;
#endif


#include "BasePoolInt.h"

namespace icke2063 {
namespace threadpool {

  
class DelayedFunctorInt{
 public:
   /**
    * default constructor
    * - set FunctorInt object
    * - set deadline
    */
   DelayedFunctorInt(shared_ptr<FunctorInt> functor, struct timeval deadline):
    m_functor(functor),m_deadline(deadline){}
   
   
   /**
    * get functor deadline
    * - after this timestamp functor should be activated
    */
   struct timeval getDeadline(){return m_deadline;}
   
   /**
    * set Deadline to current time
    */
   void resetDeadline(){gettimeofday(&m_deadline,0);}


   /**
    * get stored FunctorInt
    */
   shared_ptr<FunctorInt> getFunctor(){return m_functor;}
 
 protected:   
   shared_ptr<FunctorInt> m_functor;
   /**
    * abslute timestamp after this deadline the functor should be added to threadpool
    * 
    */
   struct timeval m_deadline;
};

class DelayedPoolInt{ 
public:  
	DelayedPoolInt();
	virtual ~DelayedPoolInt(){}

	/**
	 * 
	 * check queue with stored DelayedFunctorInt for their deadline
	 * expired deadlines should be added to ThreadPool
	 */
	virtual void checkDelayedQueue(void) = 0;
	
	/**
	 * Add new functor object with given deadline
	 * @param work pointer to functor object
	 */
	virtual void addDelayedFunctor(shared_ptr<FunctorInt> work, struct timeval deadline) = 0;
	virtual size_t getDQueueCount(){return m_delayed_queue.get()?m_delayed_queue->size():0;}

protected:
  ///list of delayed functors
  shared_ptr<std::deque<shared_ptr<DelayedFunctorInt> > > 	m_delayed_queue;

  ///lock functor queue
  shared_ptr<mutex>					m_delayed_lock;
  
};
} /* namespace common_cpp */
} /* namespace icke2063 */
#endif /* _DELAYED_THREADPOOL_H_ */