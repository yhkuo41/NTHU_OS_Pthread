#include <pthread.h>
#include <iostream>

#ifndef THREAD_HPP
#define THREAD_HPP

class Thread
{
public:
	// to start a new pthread work
	virtual void start() = 0;

	// to wait for the pthread work to complete
	virtual int join();

	// to cancel the pthread work
	virtual int cancel();

protected:
	pthread_t t;
};

int Thread::join()
{
	int res = pthread_join(t, 0);
	if (res)
	{
		std::cerr << "pthread_join error: " << res << std::endl;
	}
	return res;
}

int Thread::cancel()
{
	int res = pthread_cancel(t);
	if (res)
	{
		std::cerr << "pthread_cancel error: " << res << std::endl;
	}
	return res;
}

#endif // THREAD_HPP
