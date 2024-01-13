#include <pthread.h>
#include <cassert>
#include "thread.hpp"
#include "ts_queue.hpp"
#include "item.hpp"
#include "transformer.hpp"

#ifndef PRODUCER_HPP
#define PRODUCER_HPP
// Every Producer runs in a thread that takes Item from the Input Queue,
// applies the Item with the Transformer::producer_transform function,
// then puts the result Item into the Worker Queue.
class Producer : public Thread
{
public:
	// constructor
	Producer(TSQueue<Item *> *input_queue, TSQueue<Item *> *worker_queue, Transformer *transfomrer);

	// destructor
	~Producer();

	virtual void start();

private:
	TSQueue<Item *> *input_queue;
	TSQueue<Item *> *worker_queue;

	Transformer *transformer;

	// the method for pthread to create a producer thread
	static void *process(void *arg);
};

Producer::Producer(TSQueue<Item *> *input_queue, TSQueue<Item *> *worker_queue, Transformer *transformer)
	: input_queue(input_queue), worker_queue(worker_queue), transformer(transformer)
{
}

Producer::~Producer() {}

void Producer::start()
{
	assert(!pthread_create(&t, nullptr, &Producer::process, static_cast<void *>(this)));
}

void *Producer::process(void *arg)
{
	Producer *producer = static_cast<Producer *>(arg);
	while (true)
	{
		Item *item = producer->input_queue->dequeue();
		item->val = producer->transformer->producer_transform(item->opcode, item->val);
		producer->worker_queue->enqueue(item);
	}
	return nullptr;
}

#endif // PRODUCER_HPP
