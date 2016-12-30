#ifndef _COUNTER_H_
#define _COUNTER_H_
#include <coincidence.h> 


class Counter {
public:
	Counter(PinName pin, PinName led, coincidence* coincidence_handler, int id);
    void increment();
	int read();
	void reset();
	int read_ISR_time();
private:
    InterruptIn pulse_;
    DigitalOut led_;
    volatile int counts_;
    coincidence* coincidence_handler_;
    int id_;
    Timer ISR_time_;   
};

#endif


