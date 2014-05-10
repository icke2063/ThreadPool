/**
 * @file   DummyFunctor.h
 * @Author icke
 * @date   24.07.2013
 * @brief  Brief description of file.
 *
 * Detailed description of file.
 */

#ifndef DUMMYFUNCTOR_H_
#define DUMMYFUNCTOR_H_

#include <ThreadPool.h>
#include <sys/time.h>
#include <stdint.h>

using namespace icke2063::threadpool;

namespace icke2063 {
namespace threadpool {

//logging macros
#ifndef Dummy_Functor_log_debug
	#define Dummy_Functor_log_debug(...)
#endif

#ifndef Dummy_Functor_log_info
	#define Dummy_Functor_log_info(...)
#endif

#ifndef Dummy_Functor_log_error
	#define Dummy_Functor_log_error(...)
#endif


class Dummy_Functor: public Functor {
public:
	Dummy_Functor(uint32_t time_to_wait_ms = 0, bool silent = false):
		m_time_to_wait_ms(time_to_wait_ms), m_silent(silent){
	  gettimeofday(&creation_time,NULL);
	  Dummy_Functor_log_info("Dummy_Functor[%p]\n", this);
	  printTimestamp(&creation_time);
	};
	virtual ~Dummy_Functor(){
		  Dummy_Functor_log_info("~Dummy_Functor[%p]\n", this);
	};
	virtual void functor_function(void);

	void printTimestamp (struct timeval *timestamp);

private:
  struct timeval creation_time;
  uint32_t m_time_to_wait_ms;
  bool m_silent;
};


} /* namespace MB_Gateway */
} /* namespace icke2063 */
#endif /* DUMMYFUNCTOR_H_ */
