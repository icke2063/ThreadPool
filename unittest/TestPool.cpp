/*
 * TestPool.cpp
 *
 *  Created on: 24.06.2014
 *      Author: icke
 */

#include "TestPool.h"

namespace icke2063 {
namespace threadpool {

TestPool::TestPool(uint8_t worker_count, bool auto_start):
	ThreadPool(worker_count, auto_start){

}

TestPool::~TestPool() {
	// TODO Auto-generated destructor stub
}

} /* namespace ThreadPool */
} /* namespace icke2063 */
