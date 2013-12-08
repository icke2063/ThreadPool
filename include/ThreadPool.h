/**
 * @file   ThreadPool.h
 * @Author icke2063
 * @date   28.05.2013
 * @brief  ThreadPoolInt implementation with usage of c++11 threads, mutex,...
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

#include <sys/time.h>


#if defined(__GXX_EXPERIMENTAL_CXX0X__) || (__cplusplus >= 201103L)
  #include <memory>
  #include <chrono>
  #include <thread>
  #include <mutex>
  using namespace std;
#else
	#include <boost/shared_ptr.hpp>
  using namespace boost;
#endif

//common_cpp
#include "ThreadPoolInt/BasePoolInt.h"
#include "ThreadPoolInt/DelayedPoolInt.h"
#include "ThreadPoolInt/DynamicPoolInt.h"
#include "ThreadPoolInt/PrioPoolInt.h"

//logging macros
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

class Functor:public FunctorInt,
	      public PrioFunctorInt{  
public:
	Functor(){};
	virtual ~Functor(){};

};

class ThreadPool: public BasePoolInt,
		  public DelayedPoolInt,
		  public DynamicPoolInt{
public:
	ThreadPool();
	virtual ~ThreadPool();

	/* DynamicPoolInt */
	virtual void setLowWatermark(uint16_t low) {DynamicPoolInt::setLowWatermark(low);}
	virtual uint16_t getLowWatermark(void){return DynamicPoolInt::getLowWatermark();}
	virtual void setHighWatermark(uint16_t high){DynamicPoolInt::setHighWatermark(high);}
	virtual uint16_t getHighWatermark(){return DynamicPoolInt::getHighWatermark();}

	/* DelayedPoolInt */
	virtual size_t getDQueueCount(){return DelayedPoolInt::getDQueueCount();}
	/**
	 * Add new functor object
	 * @param work pointer to functor object
	 */
	virtual bool addFunctor(shared_ptr<FunctorInt> work, uint8_t add_mode = TPI_ADD_Default);
	virtual void addDelayedFunctor(shared_ptr<FunctorInt> work, struct timeval deadline);
	virtual bool addPrioFunctor(shared_ptr<PrioFunctorInt> work);
private:
  
  	/* function called before main thread loop */
	virtual void main_pre(void);
	/* function called within main thread loop */
	virtual void main_loop(void);
	/* function called after main thread loop */
	virtual void main_past(void);

	
	/* DynamicPoolInt */
	virtual void handleWorkerCount(void);
	
	/* DelayedPoolInt */
	virtual void checkDelayedQueue(void);
};

} /* namespace threadpool */
} /* namespace icke2063 */
#endif /* THREADPOOL_H_ */
