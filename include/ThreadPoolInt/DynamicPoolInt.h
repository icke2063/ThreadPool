/**
 * @file   DelayedThreadPool.h
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

#ifndef _DYNAMIC_THREADPOOL_H_
#define _DYNAMIC_THREADPOOL_H_

#include <sys/time.h>

namespace icke2063 {
namespace threadpool {

  
class DynamicPoolInt{ 
public:  
	DynamicPoolInt(uint8_t worker_count = 1, bool dyn_enable = false):
		max_queue_size(1),
		HighWatermark(1),
		LowWatermark(1),
		dynamic_enabled(dyn_enable)
	{
		setHighWatermark(worker_count);
		setLowWatermark(worker_count);
	}

	virtual ~DynamicPoolInt(){}
	
	/**
	 * set low watermark
	 * @param low: low count of WorkerThreads
	 */
	void setLowWatermark(uint16_t low) {
		if(dynamic_enabled){
			LowWatermark = ((low < HighWatermark)) ? low : HighWatermark;
		}
	}
	/**
	 * get low count of WorkerThreads
	 * @return lowWatermark
	 */
	uint16_t getLowWatermark(void){
		return LowWatermark;
	}

	/**
	 * set high watermark
	 * @param high: high count of WorkerThreadInts
	 */
	void setHighWatermark(uint16_t high){
		if(dynamic_enabled){
			HighWatermark = ((high > LowWatermark) && (high < WORKERTHREAD_MAX)) ? high : WORKERTHREAD_MAX;
		}
	}

	/**
	 * Get high count of WorkerThreads
	 * @return highWatermark
	 */
	uint16_t getHighWatermark(){
		return HighWatermark;
	}

	void setDynEnable(bool enable){dynamic_enabled = enable;}
	bool isDynEnabled( void ){return dynamic_enabled;}

protected:
  	/**
	 * 	This function is used to create needed WorkerThread objects
	 * 	and to destroy (really) not needed WorkerThread objects.
	 * 	MUST be implemented by inherit class
	 * 	- this function shall be called continuously
	 */
	virtual void handleWorkerCount(void) = 0;
	long max_queue_size;
protected:
  	uint16_t LowWatermark;		//low count of worker threads
	uint16_t HighWatermark;		//high count of worker threads
	bool	dynamic_enabled;	//enable flag
};
} /* namespace common_cpp */
} /* namespace icke2063 */
#endif /* _DYNAMIC_THREADPOOL_H_ */
