#include <fstream>
#include <cassert>
#include "thread.hpp"
#include "ts_queue.hpp"
#include "item.hpp"

#ifndef WRITER_HPP
#define WRITER_HPP

// Writer runs in a thread that reads Item from the Output Queue then writes Items into the output file.
class Writer : public Thread
{
public:
	// constructor
	Writer(int expected_lines, std::string output_file, TSQueue<Item *> *output_queue);

	// destructor
	~Writer();

	virtual void start() override;

private:
	// the expected lines to write,
	// the writer thread finished after output expected lines of item
	int expected_lines;

	std::ofstream ofs;
	TSQueue<Item *> *output_queue;

	// the method for pthread to create a writer thread
	static void *process(void *arg);
};

// Implementation start

Writer::Writer(int expected_lines, std::string output_file, TSQueue<Item *> *output_queue)
	: expected_lines(expected_lines), output_queue(output_queue)
{
	ofs = std::ofstream(output_file);
}

Writer::~Writer()
{
	ofs.close();
}

void Writer::start()
{
	// pass this, so the thread can access the same Writer instance
	assert(!pthread_create(&t, nullptr, &Writer::process, static_cast<void *>(this)));
}

void *Writer::process(void *arg)
{
	// this is a static method, we have to specify the Writer instance
	Writer *writer = static_cast<Writer *>(arg);
	while (writer->expected_lines--) // end after writing all lines
	{
		writer->ofs << *writer->output_queue->dequeue();
	}
	return nullptr;
}

#endif // WRITER_HPP
