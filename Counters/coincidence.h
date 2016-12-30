#ifndef COINCIDENCE_H
#define COINCIDENCE_H

#include "mbed.h"

/*
OPGEPAST!! VIEZE CODE HIER!!:
num_gm_tubes, gm_tubes_ en gm_results_ zijn gelinkt, als 1 van deze getallen verandert wordt, moeten ze allemaal veranderd worden.
idem voor num_coincidences en cnt_coinc.
ENKEL IN HEADER NODIG, IN CPP ZIJN ER GEEN MAGIC NUMBERS
*/

class coincidence
{
public:
    coincidence(PinName gm1, PinName gm2, PinName gm3, PinName gm4, int num_gm_tubes = 4, int num_coincidences = 11);
    void increment(int id);     
    unsigned int cnt_coinc[11];
    void clear(); 
private:
    DigitalIn* gm_tubes_[4];
    unsigned int gm_results_[4];
    int num_gm_tubes_;
    int num_coincidences_;
};

#endif
