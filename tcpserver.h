#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * callback fuction when accept returned 
 * */
typedef int (* Accept_callback)(int connfd);

/**
 * callback fuction when message is ready to read
 * */
typedef int (* Message_callback)(int connfd);

/**
 * the handle of tcpserver
 *
 */
typedef struct _tcpserver_handle {
	int port;   /**< port for listening */
	int listenfd;  /**< file descripter for listening */

	struct buf_handle *cliefd_buf;   /**< buffer of client file descriptors */
	struct buf_handle *readablefd_buf;     /**< buffer of readable file descriptors */

	int cnt_workings;   /**< count of working threads for system monitor */
	int timeout;	/**< timeout of select  default INFINITE */

	Accept_callback accept_cb;    /**< callback fuction when accept returned */
	Message_callback msg_cb;      /**< callback fuction when message is ready to read */
} tcpserver_handle;

/**
 * the entry stored in buffer
 */
struct buf_entry {
	int fd;
};


/**
 * the initialization function of tcpserver
 *
 * @param[in] port	-	the port for listening
 *
 * @return 
 *	NULL	-	if failed
 *	tcpserver handle	-	if succeed
 */ 
tcpserver_handle *tcpserver_create(int port);

/**
 * set callback fuction when main loop return on system call "accept"
 *
 * @param[in] hand  -   tcpserver handle
 * @param[in] cb    -   callback function need to be set
 *
 * @return 
 *  0   - if succeed
 *  <0  -   if failed
 *  
 */
int set_accept_callback(tcpserver_handle * hand, Accept_callback cb);

/**
 * set callback fuction when message is ready to read
 *
 * 
 * @param[in] hand  -   tcpserver handle
 * @param[in] cb    -   callback function need to be set
 *
 * @return 
 *  0   - if succeed
 *  <0  -   if failed
 *  
 */
int set_message_calllback(tcpserver_handle * hand, Message_callback cb);

/**
 * start the tcpserver to run
 *
 * @param[in] hand  -   tcpserver handle
 *
 * @return 
 *  0   - if succeed
 *  <0  -   if failed
 *  
 */
int start(tcpserver_handle * hand);

/**
 * the uninitialization function of tcpserver
 *
 * @param[in] hand	-	tcpserver handle
 *
 * @return 
 *  0   - if succeed
 *  <0  -   if failed
 */ 
int tcpserver_destory(tcpserver_handle * hand);

#endif //TCPSERVER_H
