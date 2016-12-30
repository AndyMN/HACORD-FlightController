#include <coincidence.h>


coincidence::coincidence(PinName gm1, PinName gm2, PinName gm3, PinName gm4, int num_gm_tubes, int num_coincidences)
    : num_gm_tubes_(num_gm_tubes), 
      num_coincidences_(num_coincidences) {

    gm_tubes_[0]= new DigitalIn(gm1);
    gm_tubes_[1]= new DigitalIn(gm2);
    gm_tubes_[2]= new DigitalIn(gm3);
    gm_tubes_[3]= new DigitalIn(gm4);
    
}

void coincidence::clear() {
    /*
    memset is sneller dan loopen over de arrays
    */
    memset(cnt_coinc, 0, sizeof(cnt_coinc));
    memset(gm_results_, 0, sizeof(gm_results_));
}


void coincidence::increment (int id) {    
    /*
    er is niets te doen voor gm met id 3, alle combinaties worden gemaakt met de andere gms
    (dit resulteert in een reductie van 5 -> 1 µs, jippie)
    */
    if (id != 3) { 
        wait_us(1);  
        
        /*
        Lees elke geiger muller tube pin. De resultaten 1 (HIGH) en 0 (LOW) worden opgeslagen en zullen gebruikt
        worden voor de coincidenties op te tellen
        */
        for ( int i=0 ; i < num_gm_tubes_ ; i++){
               gm_results_[i] = gm_tubes_[i]->read();                     
        }
        
        /*
         Afhankelijk van welke geiger muller tube een deeltjes detecteerd zullen er een aantal waarden
         binair worden opgeteld. Dit zullen de coincidenties zijn die worden weggeschreven.
         1&2 is hetzelfde als 2&1 daarom worden deze niet nog eens apart berekend.
        */
        switch(id){
            case 0: cnt_coinc[0] += ( gm_results_[0] & gm_results_[1]); // 1&2
                    cnt_coinc[1] += ( gm_results_[0] & gm_results_[2]); // 1&3
                    cnt_coinc[2] += ( gm_results_[0] & gm_results_[3]); // 1&4
                    cnt_coinc[3] += ( gm_results_[0] & gm_results_[1]  & gm_results_[2] ); // 1&2&3
                    cnt_coinc[4] += ( gm_results_[0] & gm_results_[1]  & gm_results_[3] ); // 1&2&4
                    cnt_coinc[5] += ( gm_results_[0] & gm_results_[2]  & gm_results_[3] ); // 1&3&4
                    cnt_coinc[6] += ( gm_results_[0] & gm_results_[1] & gm_results_[2]  & gm_results_[3] ); // 1&2&3&4
                    break;
            case 1: cnt_coinc[7] += ( gm_results_[1] & gm_results_[2]); // 2&3
                    cnt_coinc[8] += ( gm_results_[1] & gm_results_[3]); // 2&4
                    cnt_coinc[9] += ( gm_results_[1] & gm_results_[2]  & gm_results_[3] ); // 2&3&4
                    break;
            case 2: cnt_coinc[10] += ( gm_results_[2] & gm_results_[3]); // 3&4
                    break;
        } 
    }
}


