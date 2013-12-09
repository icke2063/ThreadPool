/**
 * @file   WorkerThread.cpp
 * @Author icke2063
 * @date   31.05.2013
 * @brief  WorkerThreadInt implementation
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

WorkerThread::WorkerThread(std::deque<shared_ptr<FunctorInt> > *functor_queue, mutex *functor_lock,int *worker_count) :
		WorkerThreadInt(functor_queue, functor_lock, worker_count) {

	WorkerThread_log_info("WorkerThread");

}

WorkerThread::~WorkerThread() {
WorkerThread_log_info("WorkerThread");
}



} /* namespace common_cpp */
} /* namespace icke2063 */
