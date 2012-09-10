/**
 *	buf.h -- thread safety producer and consumer model	
 *
 *	@author tony	<tunghai.huo@gmail.com>
 *
 */ 
#include "buf.h"
	
buf_handle * buf_init(int buf_size, int entry_size)
{
	int ret;

	if(buf_size <= 0)
	  return NULL;
	
	if(entry_size <= 0)
	  return NULL;

	buf_handle * hand = (buf_handle*)malloc(sizeof(buf_handle));
	if(hand == NULL)
	  return NULL;

	hand->buffer = (void **)malloc(sizeof(void *)*buf_size);
	hand->buf_size = buf_size;
	hand->count = 0;
	if(hand->buffer == NULL)
	  return NULL;

	hand->magic = sizeof(buf_handle);
	hand->entry_size = entry_size;
	hand->in = 0;
	hand->out = 0;

	ret = sem_init(&hand->mutex,0, 1);
	if(ret != 0)
	  return NULL;

	ret = sem_init(&hand->empty,0, buf_size);// maybe bug
	if(ret != 0)
	  return NULL;

	ret = sem_init(&hand->full,0, 0);
	if(ret != 0)
	  return NULL;

	return hand;
}

int buf_put(buf_handle * hand,void * entry)
{
	if(hand == NULL)
	  return -1;

	if(entry == NULL)
	  return -1;

	if(hand->magic != sizeof(buf_handle))
	  return -1;

	if(sizeof(entry) != ((struct buf_handle *)hand)->entry_size)
	  return -1;

	sem_wait(&hand->empty);
	sem_wait(&hand->mutex);

	hand->buffer[hand->in++] = entry;	//store pointer
	hand->in %= hand->buf_size;
	hand->count++;

	sem_post(&hand->mutex);
	sem_post(&hand->full);

	return 0;
}

void * buf_get(buf_handle * hand)
{
	if(hand == NULL)
	  return NULL;

	void * entry = NULL;

	sem_wait(&hand->full);    //down
	sem_wait(&hand->mutex);   //down

	entry = hand->buffer[hand->out++];
	hand->out %= hand->buf_size;
	hand->count--;

	sem_post(&hand->mutex);//up
	sem_post(&hand->empty);//up

	return entry;
}

int buf_getall(buf_handle * hand, void *** entries)
{
	if(hand == NULL)
	  return -1;

	int buf_size = hand->count; //maybe bugs FIXME

	*entries = (void **)malloc(buf_size * sizeof(void *));
	memset(entries, 0, buf_size * sizeof(void *));

	int i=0;
	for(;i<buf_size;i++)
	{
		*entries[i] = buf_get(hand);	
	}

	return buf_size;
}

int buf_uninit(buf_handle * hand)
{
	if(hand == NULL)
	  return -1;

	if(hand->magic != sizeof(buf_handle))
	  return -1;

//	if(hand->mutex != 0)
	  sem_destroy(&hand->mutex);

//	if(hand->empty != 0)
	  sem_destroy(&hand->empty);

//	if(hand->full != 0)
	  sem_destroy(&hand->full);

	if(hand != NULL)
	  free(hand);

	return 0;
}
