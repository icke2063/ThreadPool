/*
 * pool_test.cpp
 *
 *  Created on: 24.06.2014
 *      Author: icke
 */

//#include <auto_ptr.h>
#include <memory>

#include "ThreadPool.h"
#include "TestPool.h"
//#include <icke2063_TP_config.h>
using namespace icke2063::threadpool;

#include "DummyFunctor.h"


int main(int argc, char **argv){
int subtest = 0;

std::auto_ptr<icke2063::threadpool::ThreadPool> testpool;
std::auto_ptr<icke2063::threadpool::FunctorInt> dummy;


if(argc >= 2){
		switch (argv[1][0]) {
		case 'A':
			/**
			 * ThreadPool creation
			 * - test all parameter combinations
			 * @param count	set different Worker counts
			 * @param autostart
			 *
			 * - indirect testet functions:
			 * 		* getWorkerCount
			 * 		* addWorker
			 */

			//create default ThreadPool
			testpool.reset(new icke2063::threadpool::ThreadPool());
			if (testpool.get() == NULL && !testpool->isPoolLoopRunning()) {
				printf("Test[A;%d]: failed\n", subtest);
				exit(1);
			} else {
				printf("Test[A;%d]: passed\n", subtest++);
			}

			//create default ThreadPool
			testpool.reset(new icke2063::threadpool::ThreadPool(1, false));
			if (testpool.get() == NULL && testpool->isPoolLoopRunning()) {
				printf("Test[A;%d]: failed\n", subtest);
				exit(1);
			} else {
				printf("Test[A;%d]: passed\n", subtest++);
			}

			//create 10 worker threadpool
			testpool.reset(new icke2063::threadpool::ThreadPool(10));
			if (testpool.get() == NULL || testpool->getWorkerCount() != 10) {
				printf("Test[A;%d]: failed\n", subtest++);
				exit(1);
			} else {
				printf("Test[A;%d]: passed\n", subtest++);
			}

			//create maximum worker threadpool
			testpool.reset(
					new icke2063::threadpool::ThreadPool(WORKERTHREAD_MAX));
			if (testpool.get() == NULL
					|| testpool->getWorkerCount() != WORKERTHREAD_MAX) {
				printf("Test[A;%d]: failed\n", subtest++);
				exit(1);
			} else {
				printf("Test[A;%d]: passed\n", subtest++);
			}

			//create 0 worker threadpool
			testpool.reset(new icke2063::threadpool::ThreadPool(0));
			if (testpool.get() == NULL || testpool->getWorkerCount() != 1) {
				printf("Test[A;%d]: failed\n", subtest++);
				exit(1);
			} else {
				printf("Test[A;%d]: passed\n", subtest++);
			}

			//create maximum worker threadpool
			testpool.reset(
					new icke2063::threadpool::ThreadPool(WORKERTHREAD_MAX));
			if (testpool.get() == NULL
					|| testpool->getWorkerCount() != WORKERTHREAD_MAX) {
				printf("Test[A;%d]: failed\n", subtest++);
				exit(1);
			} else {
				printf("Test[A;%d]: passed\n", subtest++);
			}

			break;
		case 'B':
			/**
			 * create Functor
			 */
			dummy.reset(new icke2063::threadpool::Dummy_Functor(100, true));
			if (dummy.get() == NULL) {
				printf("Test[B;%d]: failed", subtest++);
				exit(1);
			} else {
				printf("Test[B;%d]: passed", subtest++);
			}
			break;

		case 'C': {
			/**
			 * delegateFunctor -> use ThreadPool to handle functor
			 */
			int counter;
			std::shared_ptr<uint32_t> flag(new uint32_t);
			(*flag.get()) = icke2063::threadpool::Test_Functor::init;

			testpool.reset(new icke2063::threadpool::ThreadPool());

			dummy.reset(new icke2063::threadpool::Test_Functor(flag, 1000, true));
			//check constructor call from functor
			if ((*flag.get()) != icke2063::threadpool::Test_Functor::construct) {
				printf("Test[C;%d]: failed\n", subtest++);
				exit(1);
			} else {
				printf("Test[C;%d]: passed\n", subtest++);
			}

			if (dummy.get() != NULL || testpool.get() != NULL || flag.get() != NULL) {

				icke2063::threadpool::Test_Functor *result = (icke2063::threadpool::Test_Functor *) testpool->delegateFunctor(dummy.release());

				//check result of adding Functor
				if (result) {
					printf("Test[C;%d]: failed\n", subtest++);
					exit(1);
				} else {
					printf("Test[C;%d]: passed\n", subtest++);
				}

				counter = 0;

				//wait for start handling of functor
				while ((*flag.get())!= icke2063::threadpool::Test_Functor::start && (counter++ < 100)) {
					usleep(1);
				}

				if (counter >= 100) {
					printf("Test[C;%d]: failed\n", subtest++);
					exit(1);
				} else {
					printf("Test[C;%d]: passed\n", subtest++);
				}

				counter = 0;
				//wait for finish handling functor
				while ((*flag.get()) != icke2063::threadpool::Test_Functor::stop && (counter++ < 1000)) {
					usleep(1000);
				}

				if (counter >= 1000) {
					printf("Test[C;%d]: failed\n", subtest++);
					exit(1);
				} else {
					printf("Test[C;%d]: passed\n", subtest++);
				}

				if(testpool->getQueueCount() != 0){
					printf("Test[C;%d]: failed\n", subtest++);
					exit(1);
				} else {
					printf("Test[C;%d]: passed\n", subtest++);
				}
			} else {
				printf("Test[C;%d]: failed\n", subtest++);
				exit(1);
			}
			printf("Test[C]: passed\n");
		}
			break;

		case 'D': {
				/**
				 * Test getQueueCount
				 */

			int workercount = 5;
			int functorcount = 1030;
			std::shared_ptr<bool> flag(new bool);

			for (int worker = 1; worker <= workercount; worker++) {

				(*(flag.get())) = true;

				testpool.reset(new icke2063::threadpool::ThreadPool(worker));
				if (testpool.get() != NULL) {
					int functor_num;
					for (int i = 0; i < functorcount; i++) {
						dummy.reset(new icke2063::threadpool::Endless_Functor(flag));
						icke2063::threadpool::Test_Functor *result =
								(icke2063::threadpool::Test_Functor *) testpool->delegateFunctor(dummy.get());

						functor_num = i;

						//check result of adding Functor
						if (result) {
							if(testpool->getQueueCount() == FUNCTOR_MAX && (i >= (FUNCTOR_MAX + worker)) ){
								dummy.reset();
								break;
							} else {
								printf("Test[D;1;{%d;%d}]: failed\n", worker, i);

								printf("QC: %d",testpool->getQueueCount());

								exit(1);
							}

						} else {
							dummy.release();
							//printf("Test[D;1;{%d;%d}]: passed\n", worker, i);
						}
						usleep(100);

					}

					sleep(1); //wait a little until all functor added to worker

					if (testpool->getQueueCount() != (functor_num - worker < 0 ?	0 : functor_num - worker)) {
						printf("Test[D;2;{%d}] failed\n", worker);
						(*flag.get()) = false;
						sleep(1);
						exit(1);
					} else {
						printf("Test[D;2;{%d;%d}]: passed\n", worker, functor_num);
					}

					(*flag.get()) = false;
					sleep(1);

					if (testpool->getQueueCount() != 0) {
						printf("Test[D;3]: failed\n");
						exit(1);
					} else {
						printf("Test[D;3]: passed\n");
					}
				}
			}
		}
		break;
#ifndef NO_DELAYED_TP_SUPPORT
		case 'E':
		{
			/**
			 * Test delegateDelayedFunctor
			 */
			int counter;
			std::shared_ptr<uint32_t> flag(new uint32_t);
			struct timeval t_deadline, t_now;

			long int msec;

			gettimeofday(&t_now,0);
			t_deadline = t_now;

			t_deadline.tv_sec += 1;


			(*flag.get()) = icke2063::threadpool::Test_Functor::init;

			testpool.reset(new icke2063::threadpool::ThreadPool());

			dummy.reset(new icke2063::threadpool::Test_Functor(flag, 1000, true));
			//check constructor call from functor
			if ((*flag.get()) != icke2063::threadpool::Test_Functor::construct) {
				printf("Test[E;1]: failed\n");
				exit(1);
			} else {
				printf("Test[E;1]: passed\n");
			}

			std::shared_ptr<DelayedFunctorInt> sp_dfunc(new DelayedFunctor(dummy.release(), &t_deadline));


			if (sp_dfunc.get() != NULL || testpool.get() != NULL || flag.get() != NULL) {

				std::shared_ptr<DelayedFunctorInt> result = testpool->delegateDelayedFunctor(sp_dfunc);

				//check result of adding Functor
				if (result.get() != NULL) {
					printf("Test[E;2]: failed\n");
					exit(1);
				} else {
					printf("Test[E;2]: passed\n");
				}


				do{
					gettimeofday(&t_now, 0);

					msec = (t_now.tv_sec - t_deadline.tv_sec)*1000;
					msec += (t_now.tv_usec - t_deadline.tv_usec)/1000;


					if((*flag.get()) == icke2063::threadpool::Test_Functor::start){
						printf("Test[E;3;1]: failed\n");
						exit(1);
					}

					if(testpool->getQueueCount() != 0){
						printf("Test[E;3;2]: failed\n");
						exit(1);
					}


					if(testpool->getDQueueCount() != 1){
						printf("Test[E;3]: failed\n");
						exit(1);
					}


					usleep(10);
				}while(msec < -10);

				printf("Test[E;3]: passed\n");

				counter = 0;
				//wait for start handling functor
				while ((*flag.get()) != icke2063::threadpool::Test_Functor::start && (counter++ < 1000)) {
					usleep(10);
				}

				if (counter >= 1000) {
					printf("Test[E;4]: failed\n");
					exit(1);
				} else {
					printf("Test[E;4]{%d}: passed\n",counter);
				}

				counter = 0;
				//wait for finish handling functor
				while ((*flag.get()) != icke2063::threadpool::Test_Functor::stop
						&& (counter++ < 1000)) {
					usleep(1000);
				}

				if (counter >= 1000) {
					printf("Test[E;5]: failed\n");
					exit(1);
				} else {
					printf("Test[E;5]: passed\n");
				}
			} else {
				printf("Test[E;0]: failed\n");
				exit(1);
			}
			printf("Test[C]: passed\n");
		}
		break;
#endif

		default:
			break;
	}
}

}
