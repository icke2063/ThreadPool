/**
 * @file   DummyFunctor.cpp
 * @Author icke
 * @date   24.07.2013
 * @brief  Brief description of file.
 *
 * Detailed description of file.
 */

#include "DummyFunctor.h"
#include "unistd.h"

namespace icke2063 {
namespace threadpool {

void Dummy_Functor::printTimestamp(struct timeval *timestamp) {
	time_t nowtime;
	struct tm *nowtm;
	char tmbuf[64], buf[64];

	nowtime = timestamp->tv_sec;
	nowtm = localtime(&nowtime);
	strftime(tmbuf, sizeof tmbuf, "%Y-%m-%d %H:%M:%S", nowtm);
	snprintf(buf, sizeof buf, "%s.%06d", tmbuf, (int)(timestamp->tv_usec));
	printf("%s \n", buf);
}

void Dummy_Functor::functor_function(void) {
	struct timeval current;

	gettimeofday(&current, NULL);
	if (!m_silent) {
		printf("Dummy Functor[%p]: start\n", (void*)this);
#ifndef NO_PRIORITY_TP_SUPPORT
		printf("priority:%d\n", getPriority());
#endif
		printf("creation time:");
		printTimestamp(&creation_time);
		printf("current time:");
		printTimestamp(&current);

		printf("wait for %d ms \n", m_time_to_wait_ms);
	}
	usleep(1000 * m_time_to_wait_ms); //wait a litte bit

	if (!m_silent) {
		printf("Dummy Functor[%p]: finished\n", (void*)this);
	}
}

} /* namespace MB_Gateway */
} /* namespace icke2063 */
