/*
 * config.h
 *
 *  Created on: 16.06.2014
 *      Author: icke
 */

#ifndef ICKE2063_TP_CONFIG_H_
#define ICKE2063_TP_CONFIG_H_

/**
 * Disable C++11 support
 *
 * When this definition is uncommented the whole threadpool
 * project will not use C++11 classes, templates,... anymore. To get the pool
 * running anyway, boost will be used. Then you have to link boost library
 * additional to the application.
 */
//#define ICKE2063_THREADPOOL_NO_CPP11	1

/**
 * Define count of addable workerthreads
 */
#ifndef WORKERTHREAD_MAX
	#define WORKERTHREAD_MAX	60
#endif


/**
 * define count of addable functor
 */
#ifndef FUNCTOR_MAX
	#define FUNCTOR_MAX	1024
#endif

/*
 * Uncomment this to remove delayed function support from threadpool
 */
//#define NO_DELAYED_TP_SUPPORT	1

/*
 * Uncomment this to remove dynamic function support from threadpool
 */
//#define NO_DYNAMIC_TP_SUPPORT	1

/*
 * Uncomment this to remove priority function support from threadpool
 */
//#define NO_PRIORITY_TP_SUPPORT
#endif /* ICKE2063_TP_CONFIG_H_ */
