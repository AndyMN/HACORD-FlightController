#include "counter.h"

Counter::Counter(PinName pin, PinName led, coincidence* coin, int id) : pulse_(pin), led_(led), coincidence_handler_(coin), id_(id) {
	pulse_.mode(PullUp);
}

void Counter::reset(){
	counts_ = 0;
	pulse_.rise(this, &Counter::increment);
}

void Counter::increment(){	
		ISR_time_.start();
		counts_++;
		led_ = !led_;
		coincidence_handler_->increment(id_);
		ISR_time_.stop();	
}

int Counter::read(){
	pulse_.rise(NULL);
	return counts_;
}


int Counter::read_ISR_time(){
	int ISR_duration = ISR_time_.read_us();
	ISR_time_.reset();
	return ISR_duration;
}
