#include <pthread.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include "consumer.hpp"
#include "ts_queue.hpp"
#include "item.hpp"
#include "transformer.hpp"

#ifndef CONSUMER_CONTROLLER
#define CONSUMER_CONTROLLER
// ConsumerController runs in a thread that controls the number of consumer threads dynamically.
class ConsumerController : public Thread
{
public:
	// constructor
	ConsumerController(
		TSQueue<Item *> *worker_queue,
		TSQueue<Item *> *writer_queue,
		Transformer *transformer,
		int check_period,
		int low_threshold,
		int high_threshold);

	// destructor
	~ConsumerController();

	virtual void start();

private:
	std::vector<Consumer *> consumers;

	TSQueue<Item *> *worker_queue;
	TSQueue<Item *> *writer_queue;

	Transformer *transformer;

	// Check to scale down or scale up every check period in microseconds.
	int check_period;
	// When the number of items in the worker queue is lower than low_threshold,
	// the number of consumers scaled down by 1.
	int low_threshold;
	// When the number of items in the worker queue is higher than high_threshold,
	// the number of consumers scaled up by 1.
	int high_threshold;

	static void *process(void *arg);
};

// Implementation start

ConsumerController::ConsumerController(
	TSQueue<Item *> *worker_queue,
	TSQueue<Item *> *writer_queue,
	Transformer *transformer,
	int check_period,
	int low_threshold,
	int high_threshold) : worker_queue(worker_queue),
						  writer_queue(writer_queue),
						  transformer(transformer),
						  check_period(check_period),
						  low_threshold(low_threshold),
						  high_threshold(high_threshold)
{
}

ConsumerController::~ConsumerController() {}

void ConsumerController::start()
{
	assert(!pthread_create(&t, nullptr, &ConsumerController::process, static_cast<void *>(this)));
}

void *ConsumerController::process(void *arg)
{
	ConsumerController *cc = static_cast<ConsumerController *>(arg);
	while (true)
	{
		// store transient state of worker queue
		int wn = cc->worker_queue->get_size();
		// The variable cn is just for readability. Since the number of consumers is not going to be changed by anyone
		// except this one and only instance of ConsumerController.
		int cn = cc->consumers.size();
		if (wn > cc->high_threshold)
		{
			cc->consumers.push_back(new Consumer(cc->worker_queue, cc->writer_queue, cc->transformer));
			cc->consumers.back()->start();
			std::cout << "Scaling up consumers from " << cn << " to " << (cn + 1) << std::endl;
		}
		// make sure there is at least one Consumer until the program ends
		else if (wn < cc->low_threshold && cn > 1)
		{
			cc->consumers.back()->cancel();
			cc->consumers.back()->join(); // wait until finish
			cc->consumers.pop_back();
			std::cout << "Scaling down consumers from " << cn << " to " << (cn - 1) << std::endl;
		}
		usleep(cc->check_period); // the timing is not very accurate, but acceptable.
	}
	return nullptr;
}

#endif // CONSUMER_CONTROLLER_HPP
