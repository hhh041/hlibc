/**
 *	buf.h -- thread safety producer and consumer model	
 *
 *	@author tony	<tunghai.huo@gmail.com>
 */ 

#ifndef BUF_H
#define BUF_H

#ifdef WINDOWS
	#include "pthreadGC2/pthread.h"
	#include "pthreadGC2/semaphore.h"
#else
	#include <pthread.h>
	#include <semaphore.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * the handle of buf including the all data used in producer-and-consumer model
 * only maintain the pointer of data
 *
 */
typedef struct _buf_handle
{
	int magic;	/**< sizeof _buf_handle */

	void ** buffer;	/**< the data buffer of pointers */
	int buf_size;	/**< the buffer size */
	int count;	/**< the current size of the buffer */

	sem_t empty;	/**< empty semaphore */
	sem_t full;		/**< full semaphore */
	sem_t mutex;	/**< mutex for sync */
	int in;		/**< index for push */
	int out;	/**< index for pop */

	int entry_size;	/**< size of every entry */

} buf_handle;


/**
 * the initialization function of the buffer
 *
 * @param[in] buf_size	-	the size of buffer
 * @param[in] entry_size	-	the size of every entry
 *
 * @return 
 *	NULL	-	if failed
 *	buffer handle	-	if succeed
 */ 
buf_handle * buf_init(int buf_size, int entry_size);


/**
 * put entry in the buffer
 *
 * @param[in] hand	-	buffer handle
 * @param[in] entry	-	the entry be put
 *
 * @return 
 * <0	-	if failed
 * 0	-	if succeed
 */ 
int buf_put(buf_handle * hand,void * entry);

/**
 * get entry from the buffer
 *
 * @param[in] hand	-	buffer handle
 *
 * @return
 * NULL	-	if failed
 * pointer to entry	-	if succeed
 */ 
void * buf_get(buf_handle * hand);

/**
 * get all entries from the buffer
 *
 * @param[in] hand	-	buffer handle
 *
 * @return
 * NULL	-	if failed
 * pointer to entries	-	if succeed
 *     void ** is a array malloc in buf_getall, should
 *     be free() outside.
 */ 
int buf_getall(buf_handle * hand, void *** entries);

/**
 * the uninitialization function of the buffer
 *
 * @param[in] hand	-	buffer handle
 *
 * @return
 * <0	-	if failed
 * 0	-	if succeed
 */ 
int buf_uninit(buf_handle * hand);

#endif
