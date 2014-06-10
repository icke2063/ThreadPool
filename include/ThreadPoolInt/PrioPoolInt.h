/**
 * @file   PrioPoolInt.h
 * @Author icke2063
 * @date   23.11.2013
 * @brief  PriorityTreadPool extension for ThreadPoolInt
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

#ifndef _PRIOPOOLINT__
#define _PRIOPOOLINT__
#ifndef NO_PRIORITY_TP_SUPPORT

#include <stdint.h>
#include <stdio.h>

namespace icke2063 {
namespace threadpool {

  
  
///Priority Functor for ThreadPool
/**
 * Inherit from this class then it can be added by ThreadPool::addFunctor with an implementation
 * of the ThreadPool class. This class has to implement the functor_function. All member functions and variables
 * are accessable by the functor_function. This function will be executed by the WorkerThreadInts
 * and after execution this object will be deleted. Make sure that this object is dynamically created and is
 * not referenced after adding to ThreadPool.
 *
 * @todo make sure that it cannot declared as static variable
 */
class PrioFunctorInt {
    #define TPI_ADD_Prio	10 
public:
	PrioFunctorInt():m_priority(0){};
	virtual ~PrioFunctorInt(){};
	
	/**
	 * set Functor priority
	 * range: 0(lowest)...100(highest)
	 * 
	 */
	void setPriority(uint8_t prio){m_priority = (prio<=100)?prio:100;}
	uint8_t getPriority(){return m_priority;}
	
private:
  
  /**
   * Functor priority
   * low priority: 0
   * high priority: 100
   * 
   */
  uint8_t m_priority;
};

///Abstract ThreadPool interface
class PrioThreadPoolInt {

public:
	PrioThreadPoolInt(){}
	virtual ~PrioThreadPoolInt(){}
	
	/**
	 * Add functor object at the correct position of base functor queue.
	 * MUST be implemented in inherit class (correct usage of locks, threads, ...)
	 */
	virtual FunctorInt *delegatePrioFunctor(FunctorInt *work) = 0;
	
};


} /* namespace common_cpp */
} /* namespace icke2063 */

#endif
#endif /* _PRIOPOOLINT__ */
