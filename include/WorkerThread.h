/**
 * @file   WorkerThread.h
 * @Author icke2063
 * @date   31.05.2013
 * @brief  WorkerThreadInt implementation with c++11 threads
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

#ifndef WORKERTHREAD_H_
#define WORKERTHREAD_H_

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || (__cplusplus >= 201103L)
  #include <memory>
  #include <thread>
  using namespace std;
#else
  #include <boost/shared_ptr.hpp>
  using namespace boost;
#endif


//common_cpp
#include <ThreadPool.h>

  //logging macros
  #ifndef WorkerThread_log_debug
  	#define WorkerThread_log_debug(...)
  #endif

  #ifndef WorkerThread_log_info
  	#define WorkerThread_log_info(...)
  #endif

  #ifndef WorkerThread_log_error
  	#define WorkerThread_log_error(...)
  #endif


namespace icke2063 {
namespace threadpool {

class WorkerThread: public WorkerThreadInt {
public:
	WorkerThread(shared_ptr<std::deque<shared_ptr<FunctorInt> > > functor_queue, shared_ptr<mutex> functor_lock);

	virtual ~WorkerThread();

	void startThread(void);
	void stopThread(void);
};

} /* namespace common_cpp */
} /* namespace icke2063 */
#endif /* WORKERTHREAD_H_ */
