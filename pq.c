/**
 *	pq.c -- thread safety producer and consumer priority queue	
 *
 *	@author tony	<tunghai.huo@gmail.com>
 *
 */

#include "pq.h"

pq_handle *pq_init(int pq_size, int entry_size, int (*cb) (void *, void *))
{
	int ret;

	if (pq_size <= 0)
		return NULL;

	if (entry_size <= 0)
		return NULL;

	if (cb == NULL)
		return NULL;

	pq_handle *hand = (pq_handle *) malloc(sizeof(pq_handle));
	if (hand == NULL)
		return NULL;

	hand->buffer = (void **)malloc(sizeof(void *) * pq_size);
	hand->pq_size = pq_size;
	hand->count = 0;
	if (hand->buffer == NULL)
		return NULL;

	hand->magic = sizeof(pq_handle);
	hand->entry_size = entry_size;

	hand->compare = cb;

	ret = sem_init(&hand->mutex, 0, 1);
	if (ret != 0)
		return NULL;

	ret = sem_init(&hand->empty, 0, pq_size);	// maybe bug
	if (ret != 0)
		return NULL;

	ret = sem_init(&hand->full, 0, 0);
	if (ret != 0)
		return NULL;

	return hand;
}

void swap(void **buffer, int a, int b)
{
	int temp = a;
	buffer[a] = buffer[b];
	buffer[b] = buffer[temp];
}


/**
 * keep heap features
 *
 */
void heapify(int (*compare) (void *, void *), void **buffer, int count,
			 int index)
{
	int left = 2 * index + 1;
	int right = 2 * index + 1;

	if (left >= count)
		left = 0;

	if (right >= count)
		right = 0;

	int largest = index;

	if (left != 0 && compare(buffer[left], buffer[index]))
		largest = left;

	if (right != 0 && compare(buffer[right], buffer[index]))
		largest = right;

	if (largest != index)
	{
		swap(buffer, index, largest);
		heapify(compare, buffer, count, largest);
	}
}

int pq_put(pq_handle * hand, void *entry)
{
	if (hand == NULL)
		return -1;

	if (entry == NULL)
		return -1;

	if (hand->magic != sizeof(pq_handle))
		return -1;

//  if(sizeof(*entry) != hand->entry_size)
//    return -1;

	sem_wait(&hand->empty);
	sem_wait(&hand->mutex);

	hand->buffer[hand->count] = entry;

	int index = hand->count;
	int parent = (index-1)/2;
	while(index>0 && hand->compare(hand->buffer[index], hand->buffer[parent]))
	{
		swap(hand->buffer, index, parent);
		index = parent;
		parent = (index - 1) / 2;
	}

	hand->count++;

	sem_post(&hand->mutex);
	sem_post(&hand->full);

	return 0;
}

void *pq_get(pq_handle * hand)
{
	if (hand == NULL)
		return NULL;

	void *entry = NULL;

	sem_wait(&hand->full);		//down
	sem_wait(&hand->mutex);		//down

	entry = hand->buffer[0];
	hand->buffer[0] = hand->buffer[hand->count];
	hand->count--;
	heapify(hand->compare, hand->buffer, hand->count, 0);

	sem_post(&hand->mutex);		//up
	sem_post(&hand->empty);		//up

	return entry;
}

int pq_uninit(pq_handle * hand)
{
	if (hand == NULL)
		return -1;

	if (hand->magic != sizeof(pq_handle))
		return -1;

//  if(hand->mutex != 0)
	sem_destroy(&hand->mutex);

//  if(hand->empty != 0)
	sem_destroy(&hand->empty);

//  if(hand->full != 0)
	sem_destroy(&hand->full);

	if (hand != NULL)
		free(hand);

	return 0;
}
