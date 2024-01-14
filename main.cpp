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
	assert(argc >= 4);

	int n = atoi(argv[1]);
	std::string input_file_name(argv[2]);
	std::string output_file_name(argv[3]);

	int rq_sz = READER_QUEUE_SIZE;
	int wq_sz = WORKER_QUEUE_SIZE;
	int wrq_sz = WRITER_QUEUE_SIZE;
	int cc_lo_p = CONSUMER_CONTROLLER_LOW_THRESHOLD_PERCENTAGE;
	int cc_hi_p = CONSUMER_CONTROLLER_HIGH_THRESHOLD_PERCENTAGE;
	int cc_period = CONSUMER_CONTROLLER_CHECK_PERIOD;
	int prod_num = PRODUCER_NUM;
	if (argc >= 11)
	{
		rq_sz = atoi(argv[4]);
		wq_sz = atoi(argv[5]);
		wrq_sz = atoi(argv[6]);
		cc_lo_p = atoi(argv[7]);
		cc_hi_p = atoi(argv[8]);
		cc_period = atoi(argv[9]);
		prod_num = atoi(argv[10]);
	}
	assert(rq_sz >= 1);
	assert(wq_sz >= 1);
	assert(wrq_sz >= 1);
	assert(cc_lo_p >= 0 && cc_lo_p <= 100);
	assert(cc_hi_p >= 0 && cc_hi_p <= 100);
	assert(cc_period >= 1);
	assert(prod_num >= 1);

	int consumer_lo_num = wq_sz * cc_lo_p / 100;
	int consumer_hi_num = wq_sz * cc_hi_p / 100;

	TSQueue<Item *> reader_q = TSQueue<Item *>(rq_sz);
	TSQueue<Item *> worker_q = TSQueue<Item *>(wq_sz);
	TSQueue<Item *> writer_q = TSQueue<Item *>(wrq_sz);
	Transformer transformer = Transformer();

	Reader *reader = new Reader(n, input_file_name, &reader_q);
	Writer *writer = new Writer(n, output_file_name, &writer_q);
	ConsumerController cc = ConsumerController(&worker_q, &writer_q, &transformer, cc_period, consumer_lo_num, consumer_hi_num);
	std::vector<Producer> producers(prod_num, Producer(&reader_q, &worker_q, &transformer));
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
	return 0;
}
