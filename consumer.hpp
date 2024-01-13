#include <pthread.h>
#include <stdio.h>
#include "thread.hpp"
#include "ts_queue.hpp"
#include "item.hpp"
#include "transformer.hpp"

#ifndef CONSUMER_HPP
#define CONSUMER_HPP
// Every Consumer runs in a thread that takes Item from the Worker Queue, applies the Item
// Transformer::consumer_transform function, then puts the result Item into Output Queue.
class Consumer : public Thread
{
public:
	// constructor
	Consumer(TSQueue<Item *> *worker_queue, TSQueue<Item *> *output_queue, Transformer *transformer);

	// destructor
	~Consumer();

	virtual void start() override;
	// return the result of pthread_join
	virtual int cancel() override;

private:
	TSQueue<Item *> *worker_queue;
	TSQueue<Item *> *output_queue;

	Transformer *transformer;

	bool is_cancel;

	// the method for pthread to create a consumer thread
	static void *process(void *arg);
};

Consumer::Consumer(TSQueue<Item *> *worker_queue, TSQueue<Item *> *output_queue, Transformer *transformer)
	: worker_queue(worker_queue), output_queue(output_queue), transformer(transformer)
{
	is_cancel = false;
}

Consumer::~Consumer() {}

void Consumer::start()
{
	assert(!pthread_create(&t, nullptr, &Consumer::process, static_cast<void *>(this)));
}

int Consumer::cancel()
{
	is_cancel = true;
	return pthread_cancel(t);
}

void *Consumer::process(void *arg)
{
	Consumer *consumer = (Consumer *)arg;

	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, nullptr);

	while (!consumer->is_cancel)
	{
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);

		Item *item = consumer->worker_queue->dequeue();
		item->val = consumer->transformer->consumer_transform(item->opcode, item->val);
		consumer->output_queue->enqueue(item);

		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);
	}

	delete consumer;

	return nullptr;
}

#endif // CONSUMER_HPP
