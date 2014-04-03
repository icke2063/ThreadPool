//============================================================================
// Name        : TestPool.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <time.h>
#include <signal.h>
#include <stdlib.h>
using namespace std;

#include "../include/ThreadPool.h"
#include "../include/DummyFunctor.h"

using namespace icke2063::threadpool;

bool	running = true;
void signal_handler(int sig);

int main() {
	struct timeval timestamp, deadline;
	Dummy_Functor *tmpFunctor;
#if defined(__GXX_EXPERIMENTAL_CXX0X__) || (__cplusplus >= 201103L)
	std::unique_ptr<ThreadPool> pool;
#else
	boost::scoped_ptr<ThreadPool> pool;
#endif
	int r_wait, r_prio, r_type, r_loop_wait, r_delay, r_add_count;

	pool.reset(new ThreadPool());

	signal(SIGINT, signal_handler); /* always clean up if an error occurs */
	signal(SIGTERM, signal_handler); /* always clean up if an error occurs */
	signal(SIGPIPE, SIG_IGN); /* catch the signal on broken pipes e.g socket
	 else the current threat will be finished */

	//pool.addFunctor(new Dummy_Functor());
#ifndef NO_DYNAMIC_TP_SUPPORT
	pool->setDynEnable(true);
	pool->setHighWatermark(10);
#endif

	srand(time(NULL));


	while(running){

		// loop waiting time after adding 0..4 sec
		r_loop_wait = rand() % 5;


		// how much functor objects to add within this step 0..50
		r_add_count = rand() % 49;

		for (int i = 0; i < r_add_count; i++) {

			// how long should the functor wait 0..99 ms
			r_wait = rand() % 100;

			// priority of the functor 0..99
			r_prio = rand() % 100;


			/* type of adding
			 * - 0..49 direct
			 * - 50..99 delayed
			 */
			r_type = rand() % 100;

			printf(
					"New Functor:\nr_type: %d r_wait: %d r_prio: %d r_loop_wait: %d\n\n",
					r_type, r_wait, r_prio, r_loop_wait);

			gettimeofday(&timestamp, NULL);

			tmpFunctor = new Dummy_Functor(r_wait, true);
#ifndef NO_PRIORITY_TP_SUPPORT
			tmpFunctor->setPriority(r_prio);
#endif
			if (r_type <= 49) {

				tmpFunctor->printTimestamp(&timestamp);
				if(!pool->addFunctor(tmpFunctor))printf("\n!!! failure on adding functor\n");
			} else {
#ifndef NO_DELAYED_TP_SUPPORT
				// loop waiting time after adding 0..99 ms
				r_delay = rand() % 100;

				deadline = timestamp;
				deadline.tv_usec += r_delay;

				tmpFunctor->printTimestamp(&timestamp);
				pool->addDelayedFunctor(tmpFunctor, &deadline);
#endif
			}
		}

		printf("Pool Info:\n");
		printf("Queue count: %d\n",pool->getQueueCount());
#ifndef NO_DELAYED_TP_SUPPORT
		printf("DQueue count: %d\n",pool->getDQueueCount());
#endif
		printf("Worker Count: %d\n",pool->getWorkerCount());


		sleep(r_loop_wait);
	}

	pool.reset(NULL);


	pool.reset(new ThreadPool());

	tmpFunctor = new Dummy_Functor(10000, true);
	pool->addFunctor(tmpFunctor);

	usleep(100);

	pool.reset(NULL);


	return 0;
}


void signal_handler(int sig) {
	printf("\ninterrupted...Signal: %d\n", sig);
	running = false;
}
