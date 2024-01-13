#include <assert.h>
#include <stdlib.h>
#include "ts_queue.hpp"
#include "item.hpp"
#include "reader.hpp"
#include "writer.hpp"
#include "producer.hpp"
#include "consumer_controller.hpp"

#define READER_QUEUE_SIZE 200
#define WORKER_QUEUE_SIZE 200
#define WRITER_QUEUE_SIZE 4000
#define CONSUMER_CONTROLLER_LOW_THRESHOLD_PERCENTAGE 20
#define CONSUMER_CONTROLLER_HIGH_THRESHOLD_PERCENTAGE 80
#define CONSUMER_CONTROLLER_CHECK_PERIOD 1000000
#define PRODUCER_NUM 4

int main(int argc, char **argv)
{
	assert(argc == 4);

	int n = atoi(argv[1]);
	std::string input_file_name(argv[2]);
	std::string output_file_name(argv[3]);
	int consumer_lo_num = WORKER_QUEUE_SIZE * CONSUMER_CONTROLLER_LOW_THRESHOLD_PERCENTAGE / 100;
	int consumer_hi_num = WORKER_QUEUE_SIZE * CONSUMER_CONTROLLER_HIGH_THRESHOLD_PERCENTAGE / 100;

	TSQueue<Item *> reader_q = TSQueue<Item *>(READER_QUEUE_SIZE);
	TSQueue<Item *> worker_q = TSQueue<Item *>(WORKER_QUEUE_SIZE);
	TSQueue<Item *> writer_q = TSQueue<Item *>(WRITER_QUEUE_SIZE);
	Transformer transformer = Transformer();

	Reader *reader = new Reader(n, input_file_name, &reader_q); // TODO basic_ifstream and delete function?
	Writer *writer = new Writer(n, output_file_name, &writer_q);
	ConsumerController cc = ConsumerController(&worker_q, &writer_q, &transformer, CONSUMER_CONTROLLER_CHECK_PERIOD, consumer_hi_num, consumer_lo_num);
	std::vector<Producer> producers(PRODUCER_NUM, Producer(&reader_q, &worker_q, &transformer));
	reader->start();
	writer->start();
	cc.start();
	for (auto &producer : producers)
	{
		producer.start();
	}

	// finish after reader and writer process every line
	reader->join();
	writer->join();
	delete writer;
	delete reader;
	// TODO experiment & log
	return 0;
}
