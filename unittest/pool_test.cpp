/*
 * pool_test.cpp
 *
 *  Created on: 24.06.2014
 *      Author: icke
 */

//#include <auto_ptr.h>
#include <memory>
#include <stdlib.h>

#include "ThreadPool.h"
#include "TestPool.h"
//#include <icke2063_TP_config.h>
using namespace icke2063::threadpool;

#include "DummyFunctor.h"

#define TP_TEST_E_WAITTOSTART_MAX_COUNT	100000

int main(int argc, char **argv){
int subtest = 0;

std::unique_ptr<icke2063::threadpool::ThreadPool> testpool;
std::unique_ptr<icke2063::threadpool::FunctorInt> dummy;


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

			printf("Test A:\n");
			printf("Create pool test -> test parameter and threadpool object creation\n");

			//create default ThreadPool
			printf("[]:\t\t ");
			testpool.reset(new icke2063::threadpool::ThreadPool());
			if (testpool.get() == NULL && !testpool->isPoolLoopRunning()) {
				printf("failed\n");
				exit(1);
			} else {
				printf("passed\n");
			}

			//create default ThreadPool
			printf("[1, 0]:\t\t ");
			testpool.reset(new icke2063::threadpool::ThreadPool(1, false));
			if (testpool.get() == NULL && testpool->isPoolLoopRunning()) {
				printf("failed\n");
				exit(1);
			} else {
				printf("passed\n");
			}

			//create 10 worker threadpool
			printf("[10]:\t\t ");
			testpool.reset(new icke2063::threadpool::ThreadPool(10));
			if (testpool.get() == NULL || testpool->getWorkerCount() != 10) {
				printf("failed\n");
				exit(1);
			} else {
				printf("passed\n");
			}

			//create maximum worker threadpool
			printf("[MAX]:\t\t ");
			testpool.reset(	new icke2063::threadpool::ThreadPool(WORKERTHREAD_MAX));
			if (testpool.get() == NULL || testpool->getWorkerCount() != WORKERTHREAD_MAX) {
				printf("failed\n");
				exit(1);
			} else {
				printf("passed\n");
			}

			//create 0 worker threadpool
			printf("[0]:\t\t ");
			testpool.reset(new icke2063::threadpool::ThreadPool(0));
			if (testpool.get() == NULL || testpool->getWorkerCount() != 1) {
				printf("failed\n");
				exit(1);
			} else {
				printf("passed\n");
			}

			printf("Test[A]: \t passed\n");

			break;
		case 'B':
			/**
			 * create Functor
			 */

			printf("Test B:\n");
			printf("Create functor test -> simple DummyFunctor object creation\n");

			printf("[100,0]:\t\t ");
			dummy.reset(new icke2063::threadpool::Dummy_Functor(100, true));
			if (dummy.get() == NULL) {
				printf("failed\n");
				exit(1);
			} else {
				printf("passed\n");
			}
			printf("Test[B]: \t passed\n");
			break;

		case 'C': {
			/**
			 * delegateFunctor -> use ThreadPool to handle functor
			 */
			printf("Test C:\n");
			printf("Test_Functor test -> use threadpool to handle special functor\n");

			printf("Test_Functor[p,1000,1]:\n");

			int counter;
			std::shared_ptr<uint32_t> flag(new uint32_t);
			(*flag.get()) = icke2063::threadpool::Test_Functor::init;
			testpool.reset(new icke2063::threadpool::ThreadPool());
			dummy.reset(new icke2063::threadpool::Test_Functor(flag, 1000, true));
			//check constructor call from functor
			printf("construct: \t\t ");
			if ((*flag.get()) != icke2063::threadpool::Test_Functor::construct) {
				printf("failed\n");
				exit(1);
			} else {
				printf("passed\n");
			}

			if (dummy.get() != NULL || testpool.get() != NULL || flag.get() != NULL) {

				icke2063::threadpool::Test_Functor *result = (icke2063::threadpool::Test_Functor *) testpool->delegateFunctor(dummy.release());

				//check result of adding Functor
				printf("delegate: \t\t ");
				if (result) {
					printf("failed\n");
					exit(1);
				} else {
					printf("passed\n");
				}

				counter = 0;

				//wait for start handling of functor
				while ((*flag.get())!= icke2063::threadpool::Test_Functor::start && (counter++ < 100)) {
					usleep(1);
				}

				printf("start: \t\t ");

				if (counter >= 100) {
					printf("failed\n");
					exit(1);
				} else {
					printf("passed\n");
				}

				counter = 0;
				//wait for finish handling functor
				while ((*flag.get()) != icke2063::threadpool::Test_Functor::stop && (counter++ < 1000)) {
					usleep(1000);
				}

				printf("stop: \t\t ");
				if (counter >= 1000) {
					printf("failed\n");
					exit(1);
				} else {
					printf("passed\n");
				}

				printf("finished: \t\t ");
				if(testpool->getQueueCount() != 0){
					printf("failed\n");
					exit(1);
				} else {
					printf("passed\n");
				}
			} else {
				printf("Test C failed\n");
				exit(1);
			}
			printf("Test[C]: passed\n");
		}
			break;

		case 'D': {
				/**
				 * Test getQueueCount
				 */

			printf("Test D:\n");
			printf("Queue test\n");


			int workercount = 5;
			int functorcount = 1030;

			printf("test input[max worker:%d;max functor:%d]\n",workercount, functorcount);
			printf("test output{used worker,added functors}\n");

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
								printf("Test[D;Add;{%d;%d}]: failed\n", worker, i);

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
						printf("Test[D;Check;{%d}] failed\n", worker);
						(*flag.get()) = false;
						sleep(1);
						exit(1);
					} else {
						printf("Test[D;Add/Check;{%d;%d}]: passed\n", worker, functor_num);
					}

					(*flag.get()) = false;
					sleep(1);

					printf("handle: \t");

					if (testpool->getQueueCount() != 0) {
						printf(" failed\n");
						exit(1);
					} else {
						printf(" passed\n");
					}
				}
			}
		}
		printf("Test[D]: passed\n");
		break;
#ifndef NO_DELAYED_TP_SUPPORT
		case 'E':
		{
			/**
			 * Test delegateDelayedFunctor
			 */

			printf("Test E:\n");
			printf("Delayed test\n");

			int counter;
			std::shared_ptr<uint32_t> flag(new uint32_t);
			std::chrono::steady_clock::time_point t_deadline, t_now;

			long int msec;

			t_now = std::chrono::steady_clock::now();
			t_deadline = t_now;

			t_deadline += std::chrono::seconds(1);


			(*flag.get()) = icke2063::threadpool::Test_Functor::init;

			testpool.reset(new icke2063::threadpool::ThreadPool());

			dummy.reset(new icke2063::threadpool::Test_Functor(flag, 1000, true));
			//check constructor call from functor
			printf("construct:\t");
			if ((*flag.get()) != icke2063::threadpool::Test_Functor::construct) {
				printf("failed\n");
				exit(1);
			} else {
				printf("passed\n");
			}

			std::shared_ptr<DelayedFunctorInt> sp_dfunc(
					new DelayedFunctor(dummy.release(), t_deadline));


			if (sp_dfunc.get() != NULL || testpool.get() != NULL || flag.get() != NULL) {

				std::shared_ptr<DelayedFunctorInt> result = testpool->delegateDelayedFunctor(sp_dfunc);

				//check result of adding Functor
				printf("delegate:\t");
				if (result.get() != NULL) {
					printf("failed\n");
					exit(1);
				} else {
					printf("passed\n");
				}


				do{
					t_now = std::chrono::steady_clock::now();


					  msec = (std::chrono::duration<float> (
							  t_now - t_deadline)).count();

					if((*flag.get()) == icke2063::threadpool::Test_Functor::start){
						printf("wait[start]:\t");
						printf("failed\n");
						exit(1);
					}

					if(testpool->getQueueCount() != 0){
						printf("wait[Q!=0]:\t");
						printf("failed\n");
						exit(1);
					}


					if(testpool->getDQueueCount() != 1){
						printf("wait[DQ!=1]:\t");
						printf("failed\n");
						exit(1);
					}


					usleep(10);
				}while(msec < -10);

				printf("wait[%li msec]:\t passed\n",msec);

				counter = 0;
				//wait for start handling functor (max 100 miliseconds)
				while ((*flag.get()) != icke2063::threadpool::Test_Functor::start && (counter++ < TP_TEST_E_WAITTOSTART_MAX_COUNT)) {
					usleep(1);
				}

				printf("start:\t");
				if (counter >= TP_TEST_E_WAITTOSTART_MAX_COUNT) {
					printf("failed\n");
					exit(1);
				} else {
					printf("{(%d us)} passed\n",counter);
				}

				counter = 0;
				//wait for finish handling functor
				while ((*flag.get()) != icke2063::threadpool::Test_Functor::stop
						&& (counter++ < 1000)) {
					usleep(1000);
				}

				printf("stop:\t");
				if (counter >= 1000) {
					printf("failed\n");
					exit(1);
				} else {
					printf("passed\n");
				}
			} else {
				printf("Test[E;init]: failed\n");
				exit(1);
			}
			printf("Test[E]: passed\n");
		}
		break;
#endif

		default:
			break;
	}
}

}
