#ifndef __NTC_HPP__
#define __NTC_HPP__

#include <map>

class ntc {

public:
    ntc();
    float get_temperature (float adcvalue,float voltage);

private:
    std::map<short,short > ntcmap;



};





#endif

