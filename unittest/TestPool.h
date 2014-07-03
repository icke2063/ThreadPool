/*
 * TestPool.h
 *
 *  Created on: 24.06.2014
 *      Author: icke
 */

#ifndef TESTPOOL_H_
#define TESTPOOL_H_

#include <ThreadPool.h>
#include <memory>
#include "DummyFunctor.h"
#include <stdint.h>

#define TEST_FUNC_CONSTRUCT	5



namespace icke2063 {
namespace threadpool {


class TestPool: public icke2063::threadpool::ThreadPool {
public:

	TestPool(uint8_t worker_count = 1, bool auto_start = true);
	virtual ~TestPool();
};


class Test_Functor: public Dummy_Functor {
public:
	enum test_func_state{
		init = 0x01,
		construct = 0x02,
		start = 0x03,
		stop = 0x04
	};

public:
	Test_Functor(TP_NS::shared_ptr<uint32_t> functor_finished_flag, uint32_t time_to_wait_ms = 0, bool silent = false):
		Dummy_Functor(time_to_wait_ms, silent),
		sp_functor_finished_flag(functor_finished_flag){
		if(sp_functor_finished_flag.get()){
			*sp_functor_finished_flag.get() = icke2063::threadpool::Test_Functor::construct;
		}
	};
	virtual ~Test_Functor(){};
	virtual void functor_function(void) {

		if (sp_functor_finished_flag.get()) {
			*sp_functor_finished_flag.get() = icke2063::threadpool::Test_Functor::start;
		}
		Dummy_Functor::functor_function();
		if (sp_functor_finished_flag.get()) {
			*sp_functor_finished_flag.get() = icke2063::threadpool::Test_Functor::stop;
		}
	}

private:
	TP_NS::shared_ptr<uint32_t> sp_functor_finished_flag;

};


class Endless_Functor: public Functor {
public:
	Endless_Functor(TP_NS::shared_ptr<bool> running_flag):
		sp_running_flag(running_flag){
	};
	virtual ~Endless_Functor(){};
	virtual void functor_function(void) {

		if (sp_running_flag.get()) {
			while(*sp_running_flag.get()){
				usleep(100);
			}
		}
	}

private:
	TP_NS::shared_ptr<bool> sp_running_flag;

};



} /* namespace ThreadPool */
} /* namespace icke2063 */
#endif /* TESTPOOL_H_ */
