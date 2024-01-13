#include <pthread.h>

#ifndef TS_QUEUE_HPP
#define TS_QUEUE_HPP

#define DEFAULT_BUFFER_SIZE 200

// A thread safe queue that allows multi-thread access.
template <class T>
class TSQueue
{
public:
	// constructor
	TSQueue();

	explicit TSQueue(int max_buffer_size);

	// destructor
	~TSQueue();

	// add an element to the end of the queue
	void enqueue(T item);

	// remove and return the first element of the queue
	T dequeue();

	// Return the number of elements in the queue. The result is useful only when this queue not undergoing concurrent updates
	// in other threads. Otherwise the result is just a transient state that may be adequate for monitoring or estimation purposes,
	// but not for program control.
	int get_size();

private:
	// the maximum buffer size
	int buffer_size;
	// the buffer containing values of the queue
	T *buffer;
	// the current size of the buffer
	int size;
	// the index of first item in the queue
	int head;
	// the index of last item in the queue
	int tail;

	// pthread mutex lock
	pthread_mutex_t mutex;
	// pthread conditional variable
	pthread_cond_t cond_enqueue, cond_dequeue;
};

// Implementation start

template <class T>
TSQueue<T>::TSQueue() : TSQueue(DEFAULT_BUFFER_SIZE)
{
}

template <class T>
TSQueue<T>::TSQueue(int buffer_size) : buffer_size(buffer_size), buffer(new T[buffer_size]), size(0), head(0), tail(buffer_size - 1)
{
	pthread_mutex_init(&mutex, nullptr);
	pthread_cond_init(&cond_enqueue, nullptr);
	pthread_cond_init(&cond_dequeue, nullptr);
}

template <class T>
TSQueue<T>::~TSQueue()
{
	delete[] buffer;
	pthread_cond_destroy(&cond_enqueue);
	pthread_cond_destroy(&cond_dequeue);
	pthread_mutex_destroy(&mutex);
}

template <class T>
void TSQueue<T>::enqueue(T item)
{
	pthread_mutex_lock(&mutex);
	while (size == buffer_size) // the queue is full, wait until dequeue
	{
		pthread_cond_wait(&cond_enqueue, &mutex);
	}
	buffer[head] = item;
	head = (head + 1) % buffer_size;
	++size;
	pthread_cond_signal(&cond_dequeue); // we have item(s) in queue, notify dequeue
	pthread_mutex_unlock(&mutex);
}

template <class T>
T TSQueue<T>::dequeue()
{
	T res;
	pthread_mutex_lock(&mutex);
	while (!size) // the queue is empty, wait until enqueue
	{
		pthread_cond_wait(&cond_dequeue, &mutex);
	}
	tail = (tail + 1) % buffer_size;
	res = buffer[tail];
	--size;
	pthread_cond_signal(&cond_enqueue); // we have slot(s), notify enqueue
	pthread_mutex_unlock(&mutex);
	return res;
}

template <class T>
int TSQueue<T>::get_size()
{
	return size;
}

#endif // TS_QUEUE_HPP
