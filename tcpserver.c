#include "tcpserver.h"


tcpserver_handle *tcpserver_create(int port)
{
	int ret = 0;
	tcpserver_handle *hand =
		(tcpserver_handle *) malloc(sizeof(tcpserver_handle));
	memset(hand, 0, sizeof(tcpserver_handle));

	hand->port = port;

	/* socket init */
	struct sockaddr_in servaddr_in;
	memset(&servaddr_in, 0, sizeof(struct sockaddr_in));
	servaddr_in.sin_family = AF_INET;
	servaddr_in.sin_port = htons(hand->port);
	servaddr_in.sin_addr.s_addr = INADDR_ANY;

	/* socket */
	hand->listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (hand->listenfd < 0)
	{
		free(hand);
		return NULL;
	}

	/* bind */
	ret =
		bind(hand->listenfd, (struct sockaddr *)&servaddr_in,
			 sizeof(struct sockaddr_in));
	if (ret < 0)
	{
		free(hand);
		return NULL;
	}

	/* listen */
	ret = listen(hand->listenfd, 1024);
	if (ret < 0)
	{
		free(hand);
		return NULL;
	}

	return hand;
}

int set_accept_callback(tcpserver_handle * hand, Accept_callback cb)
{
	hand->accept_cb = cb;
	return 0;
}

int set_message_calllback(tcpserver_handle * hand, Message_callback cb)
{
	hand->msg_cb = cb;
	return 0;
}


void *selecting(void *parg)
{
	if (parg == NULL)
		return (void *)-1;

	tcpserver_handle *hand = (tcpserver_handle *)parg;

	/* get ready for select() function */
	int ret = 0;
	fd_set rset;
	struct timeval time_out;

	int nfds = 0;
	int i = 0;

	struct buf_entry **entries;

	for (;;)
	{
		int buf_size = buf_getall(hand->cliefd_buf, &entries);
		for (i = 0; i < buf_size; i++)
		{
			nfds = nfds > entries[i]->fd ? nfds : entries[i]->fd;
		}
		nfds += 1;

		FD_ZERO(&rset);
		for (i = 0; i < buf_size; i++)
		{
			FD_SET(entries[i]->fd, &rset);
		}
		time_out.tv_sec = hand->timeout;
		time_out.tv_usec = 0;

		if ((ret = select(nfds, &rset, NULL, NULL, &time_out)) < 0)
		{
			return (void *)-1;
		} else if (ret == 0)
		{						/* time out */
			return (void *)-2;
		}

		for (i = 0; i < buf_size; i++)
		{
			if (FD_ISSET(entries[i]->fd, &rset))
			{
				buf_put(hand->readablefd_buf,entries[i]);
			} else 
			{
				buf_put(hand->cliefd_buf,entries[i]->fd);
			}
		}
	}

	return (void *)0;
}

/**
 * the working function in every thread
 */
void *working(void *parg)
{
	if (parg == NULL)
		return (void *)-1;

	tcpserver_handle *hand = (tcpserver_handle *)parg;
	
	struct buf_entry *entry;

    for(;;)
    {
        entry = (struct buf_entry *)buf_get(hand->readablefd_buf);

    	int ret = hand->msg_cb(entry->fd);
    	if (ret < 0 )
    	{
    		close(entry->fd);
    	}
        free(entry);
    }

	return (void *)0;
}

int accepting(tcpserver_handle * hand)
{
	/* main loop for accept */
	struct sockaddr_in clientaddr_in;
	socklen_t clientlen = sizeof(clientaddr_in);
	memset(&clientaddr_in, 0, sizeof(struct sockaddr_in));
	int connfd;
	struct buf_entry * entry;
	
	for (;;)
	{
		/* accept */
		connfd = accept(hand->listenfd, (struct sockaddr *)&clientaddr_in,
						&clientlen);

		if (connfd < 0)
			continue;

		/* buffer put and create pthread with argument passing */
		entry = (struct buf_entry *)malloc(sizeof(struct buf_entry));
		entry->fd = connfd;
		buf_put(hand->cliefd_buf, entry);
	}
	return 0;
}

int start(tcpserver_handle * hand)
{
	int ret = 0;
	int i = 0;
	/* pthread init */
	pthread_t pid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	/* working threads */
	for (i= 0 ; i< hand->cnt_workings;i++)
	{
		if (pthread_create
				(&pid, &attr, (void *(*)(void *))working, (void *)hand) < 0)
		{
			return -1;	
		}
	}

	/* selecting thread */
	if (pthread_create(&pid, &attr, (void * (*)(void *))selecting, (void *)hand) < 0)
	{
		return -1;
	}

	accepting(hand);

	return 0;
}

int tcpserver_destory(tcpserver_handle * hand)
{
	if(hand == NULL)
	  return -1;

	close(hand->listenfd);
   
    if(hand != NULL)
    	free(hand);

    return 0;
}
