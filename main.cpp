#include "mbed.h"
#include "EthernetInterface.h"
#include "ASensor.h"
#include "PressureSensor.h"
#include "PressureSensorSystem.h"
#include "coincidence.h"
#include "counter.h"
#include "ntc.hpp"
#include "ds1621.hpp"
#include "SDFileSystem.h"
#include <string>

extern "C" void mbed_reset(); // NEEDED TO BE ABLE TO RESET THE MBED. THIS IS UNDOCUMENTED

DigitalOut watchdog1(p25, 0);
DigitalOut watchdog2(p24, 0);
DigitalOut watchdog3(p23, 0);


Serial pc(USBTX, USBRX); // tx, rx
SDFileSystem sd(p5, p6, p7, p8, "sd");
DigitalOut sd_error(LED4);
DigitalOut sd_succes(LED3);

// ONLY FOR SIMULATION
////////////////////////////////
//int num_datapoints = 0;
//int max_num_datapoints = 20442; // Bepaald door dataset
//DigitalOut data_trigger(p13);
////////////////////////////////

ASensor source_voltage(p19, 15000, 15000);


PressureSensor sensor_1(p18, 10000, 13000); 
PressureSensor sensor_2(p17, 10000, 13000); // CHANGE TO P17 FOR FLIGHT
PressureSensor sensor_3(p16, 10000, 13000); // CHANGE TO P16 FOR FLIGHT

std::vector<PressureSensor*> pressure_sensors;

coincidence coin(p21, p22, p12, p11);     // initialize the class with all possible coincedences

Counter counter1(p21, LED1, &coin, 0);   
Counter counter2(p22, LED2, &coin, 1);   
Counter counter3(p12, LED3, &coin, 2);   
Counter counter4(p11, LED4, &coin, 3);  




DigitalOut hv_state(p9); // CHANGE TO P9 FOR FLIGHT

ASensor ntc_sensor(p20);
ntc ntc1;


I2C i2cT(p28, p27);
DS1621 digT(&i2cT, 0x90);

EthernetInterface eth;


UDPSocket dataSocket;
UDPSocket commSocket;
UDPSocket debugSocket;

Endpoint remoteDataPC;
Endpoint remoteCommPC;
Endpoint remoteDebugPC;

Timer total_timer;


//const char* MYIP = "192.168.2.10"; // IN WIM LABO
const char* MYIP = "172.16.18.100"; 
//const char* REMOTEIP = "192.168.1.10"; // IN WIM LABO
const char* REMOTEIP = "172.16.18.101"; 

const char* MYMASK = "255.255.255.0";
const char* MYGATEWAY = "255.255.255.0";

const int DEBUGPORT = 1336;
const int DATAPORT = 1338;
const int COMMPORT = 1337;



bool ethernet_connected = false;

bool listen_to_pbs = true;
bool send_over_udp = true;
bool save_to_sd = true;

int received_packets = 0;
int sent_packets = 0;
bool connection_stable = true;

void processMessage(char[],int); // Function to handle incoming commands
void executionCheck(int); // Checks if the previous process executed succesfully
void commandSocketSend(char[]); // Sends a string via command socket
void open_sd_dir(int&); // Checks if it can open the SD directory
void debugPrint(std::string); // Sends string over debug socket
void watchdogReset(); // Resets the watchdogs


int main(){
    //wait(10); // becuz i needz tha time to open up serial program
    watchdogReset();
  
    hv_state = 0; // inverted on dc/dc. 0 is ON
    
    // ETHERNET INTERFACES
    ///////////////////////////////////////////////
    pc.printf("Initializing Ethernetinterface \n\r");
    int ethInit = eth.init(MYIP,MYMASK,MYGATEWAY);
    executionCheck(ethInit);
    watchdogReset();
    
    pc.printf("Connecting Ethernet \n\r");
    int ethConnect = eth.connect(1);
    executionCheck(ethConnect);
    if (ethConnect == 0) {
        ethernet_connected = true;
    }    
    watchdogReset();
    
    pc.printf("Binding Data Socket \n\r");
    int udpBind = dataSocket.bind(DATAPORT);
    executionCheck(udpBind);
    watchdogReset();
    
    pc.printf("Binding Command Socket \n\r");
    int commUdpBind = commSocket.bind(COMMPORT);
    executionCheck(commUdpBind);
    watchdogReset();
    
    pc.printf("Binding Debug Socket \n\r");
    int debugUdpBind = debugSocket.bind(DEBUGPORT);
    executionCheck(debugUdpBind);
    watchdogReset();
    
    pc.printf("Setting Address Data Endpoint \n\r");
    int dataEndPointSet = remoteDataPC.set_address(REMOTEIP,DATAPORT);
    executionCheck(dataEndPointSet);
    watchdogReset();
    
    pc.printf("Setting Address Command Endpoint \n\r");
    int commEndPointSet = remoteCommPC.set_address(REMOTEIP,COMMPORT);  
    executionCheck(commEndPointSet);
    watchdogReset();
    
    pc.printf("Setting Address Debug Endpoint \n\r");
    int debugEndPointSet = remoteDebugPC.set_address(REMOTEIP, DEBUGPORT);
    // Buffer that will get filled with our data and then gets sent via that data socket
    const unsigned int dataBufferSize = 200;
    char dataBuffer[dataBufferSize];
    memset(dataBuffer, 0, dataBufferSize);
    watchdogReset();
    
    // Buffer that will hold received messages
    const unsigned int commandBufferSize = 15;
    char commandBuffer[commandBufferSize];
    memset(commandBuffer, 0, commandBufferSize); // This was needed right after initialization because otherwise the first received message will be analyzed incorrectly, i don't know why but it works like this
    watchdogReset();
    
    // Header array
    char header[] = "time\tcnt1\tcnt2\tcnt3\tcnt4\t1&2\t1&3\t1&4\t2&3\t2&4\t3&4\t1&2&3\t1&2&4\t1&3&4\t2&3&4\t1&2&3&4\tadcr\tadcp1\tadcp2\tadcp3\tadcntc\tntcT\tdigT\tp1\tp2\tp3\thvstate\tpbsstate\tethstate\tlistpbs\tsdstate\tlooptime\n";
    
    commSocket.set_blocking(false,1); // Otherwise the program will stay on the "receiveFrom" part forever until it receives something. 
    debugPrint("Debug initialized");
    ///////////////////////////////////////////////
    
    // VARIABLE INIT
    ///////////////////////////
    int socket_send = 0;
    int reset_loops = 0;
    
    int count1 = 0;
    int count2 = 0;
    int count3 = 0;
    int count4 = 0;
    float pressure1 = 0;
    float pressure2 = 0;
    float pressure3 = 0;
    float adc_p1 = 0;
    float adc_p2 = 0;
    float adc_p3 = 0;
    float adc_ref = 0;
    float adc_T = 0;
    float ntc_temp = 0;
    float dig_temp = 0;
    bool pbs_state = 0;
    time_t current_time = time(NULL);
    int total_time = 0;
    
    // SD CARD VARIABLES
    ///////////////////////////////
    int file_number = 0;
    int measurement = 0;
    int measurements_per_file = 600;
    int sd_print = -1;
    char file_name [20];  
    FILE *fp = NULL;
    watchdogReset();
    open_sd_dir(file_number);
    watchdogReset();
    sprintf(file_name, "/sd/M_%d.TXT", file_number); 
    fp = fopen(file_name, "a");
    fprintf(fp, header);
    //////////////////////////////////////////////////////////////////////
    
    
    // PRESSURE SENSORS AND PBS
    ///////////////////////////////////////////////////////
    pressure_sensors.push_back(&sensor_1);
    pressure_sensors.push_back(&sensor_2);
    pressure_sensors.push_back(&sensor_3);
    PressureSensorSystem PBS(pressure_sensors, &source_voltage);
    ////////////////////////////////////////////////////////
    
    set_time(0);
    while(1){  
            //num_datapoints++; // FOR SIMULATION
            total_timer.start();
            // SET WATCHDOGS HIGH
            ///////////////////////
            watchdogReset();
            ////////////////////////
            
            // START COUNTER MEASUREMENTS
            ////////////////////////////////////
            coin.clear();
            counter1.reset();//assigns interrupt vector 
            counter2.reset();
            counter3.reset();
            counter4.reset();
            //data_trigger = !data_trigger;   // Get another data point from simulation
            wait_ms(1000);
            count1 = counter1.read();//de-assign interrupt vector. 
            count2 = counter2.read();
            count3 = counter3.read();
            count4 = counter4.read(); 
            //////////////////////////////////////
            
            
            // SENSOR MEASUREMENTS
            ////////////////////////////////////            
            PBS.measure(); // Do a measurement for PBS every loop
            adc_ref = source_voltage.voltage();
            adc_p1 = sensor_1.voltage();
            adc_p2 = sensor_2.voltage();
            adc_p3 = sensor_3.voltage();
            adc_T = ntc_sensor.voltage();
            pressure1 = sensor_1.pressure(adc_ref);
            pressure2 = sensor_2.pressure(adc_ref);
            pressure3 = sensor_3.pressure(adc_ref);
            ntc_temp = ntc1.get_temperature(adc_T, adc_ref);
            
            digT.init_meassure();
            digT.init_read();
            dig_temp = digT.read_T();
            
            pbs_state = PBS.HV_state();
            current_time = time(NULL);
            ///////////////////////////////////////
            
            // SEND OVER UDP
            ////////////////////////////////////
            sprintf(dataBuffer,"%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%d\t%d\t%d\t%d\t%d\t%d\n", current_time, count1, count2, count3, count4, coin.cnt_coinc[0], coin.cnt_coinc[1], coin.cnt_coinc[2], coin.cnt_coinc[7], coin.cnt_coinc[8], coin.cnt_coinc[10], coin.cnt_coinc[3], coin.cnt_coinc[4], coin.cnt_coinc[5], coin.cnt_coinc[9], coin.cnt_coinc[6], adc_ref, adc_p1, adc_p2, adc_p3, adc_T, ntc_temp, dig_temp, pressure1, pressure2, pressure3, hv_state.read(), pbs_state, connection_stable, listen_to_pbs, sd_print, total_time);
            if (send_over_udp == true){
                socket_send = dataSocket.sendTo(remoteDataPC,dataBuffer,dataBufferSize);
                if (socket_send == -1) {
                    //pc.printf("Socket send failed \n\r");
                    connection_stable = false;
                    // HAVE TO CLOSE SOCKETS AND THEN REOPEN THEM !
                    pc.printf("Closing Data Socket \n\r");
                    int close_data = dataSocket.close();
                    executionCheck(close_data);
                    
                    pc.printf("Closing Command Socket \n\r");
                    int close_command = commSocket.close();
                    executionCheck(close_command);
                    
                    int close_debug = debugSocket.close();
                    executionCheck(close_debug);
                    
                    // THIS NEEDS TO BE CONNECTED ONCE DURING THE ENTIRE FLIGHT
                    if (!ethernet_connected){
                        pc.printf("Connecting Ethernet \n\r");
                        int ethConnect = eth.connect(1);
                        executionCheck(ethConnect);
                        if (ethConnect == 0) {
                            ethernet_connected = true;
                        }
                    }   
                                    
                    pc.printf("Binding Data Socket \n\r");
                    udpBind = dataSocket.bind(DATAPORT);
                    executionCheck(udpBind);
                    
                    pc.printf("Binding Command Socket \n\r");
                    commUdpBind = commSocket.bind(COMMPORT);
                    executionCheck(commUdpBind);
                    
                    pc.printf("Binding Debug Socket \n\r");
                    debugUdpBind = debugSocket.bind(DEBUGPORT);
                    executionCheck(debugUdpBind);
                    
                    // TRY ONCE MORE
                    socket_send = dataSocket.sendTo(remoteDataPC,dataBuffer,dataBufferSize);                
                }else{
                    sent_packets++;
                    
                    if (sent_packets - received_packets <= 5 && sent_packets > 0 && received_packets > 0){
                        connection_stable = true;
                    }else{
                        if (reset_loops >= 1 && reset_loops <= 5){ // if we have just reset the counters, don't change the connection state yet
                            reset_loops++;
                        }else{
                            connection_stable = false;
                        }
                    }
                    if (sent_packets >= 20){ // reset the counters 
                        sent_packets = 0;
                        received_packets = 0;
                        reset_loops = 1; // we are using this to not reset the connection state when we reset counters
                    }
                }
            }
            
            /////////////////////////////////////////////////////////
            
            // SD CARD STORAGE
            //////////////////////////////////////////
            pc.printf(dataBuffer);
            if (save_to_sd){
                if (measurement >= measurements_per_file){
                    measurement = 0;
                    file_number++;
                    sprintf(file_name, "/sd/M_%d.TXT", file_number);
                    if (fp != NULL){
                        fclose(fp);
                        fp = fopen(file_name, "a");
                        fprintf(fp, header);
                    }
                }            
                sd_print = fprintf(fp, dataBuffer);
                measurement++;      
            }else{
                sd_print = -1;
            }            
            ////////////////////////////////////////////
                        
            // IF UDP DOESNT WORK AND HV IS ON LET THE PBS DETERMINE STATE OF HV
            ////////////////////////////////////////////////////
            if (connection_stable == false && hv_state == 0 && listen_to_pbs == true){
                hv_state = pbs_state;
            }
            ////////////////////////////////////////////////////
            
             // IF UDP DOESNT WORK AND SD IS ON LET THE PBS DETERMINE STATE OF SD
            ////////////////////////////////////////////////////
            if (connection_stable == false && save_to_sd == 1 && listen_to_pbs == true){
                save_to_sd = !pbs_state;
            }
            ////////////////////////////////////////////////////
            
            // CHECK FOR UDP COMMANDS
            ////////////////////////////////////////////////////
            int receivedBytes = commSocket.receiveFrom(remoteCommPC,commandBuffer,commandBufferSize);     
            if (receivedBytes > 0){
                processMessage(commandBuffer,receivedBytes);
                memset(commandBuffer,0,commandBufferSize);               
            }     
            
            total_timer.stop();
            total_time = total_timer.read_us();
            total_timer.reset();
           ////////////////////////////////////////////////////
           pc.printf("Packets sent: %d \t Answers received: %d", sent_packets, received_packets);
    }    
}

// Any command and there function has to go in here.
void processMessage(char commandBuffer[],int receivedBytes){    
    std::string message(commandBuffer);
    
    for(int i = 0; i<receivedBytes; i++){
        pc.printf("%c",commandBuffer[i]);
    }
    pc.printf("\n\r");
    
    std::string substr_message = message.substr(2);
    
    std::string utc_check_string = substr_message.substr(0, 3);
    
    
    if (utc_check_string == "UTC"){
        std::string utc_timestamp = substr_message.substr(3);
        int utc_time = atoi(utc_timestamp.c_str());
        set_time(utc_time);
        commandSocketSend(commandBuffer);
    }else if (substr_message == "Ping"){  
        commandSocketSend(commandBuffer);     
    }else if (substr_message == "ListenPBS"){
        listen_to_pbs = true;
        commandSocketSend(commandBuffer);
    }else if (substr_message == "NListenPBS"){
        listen_to_pbs = false;
        commandSocketSend(commandBuffer);
    }else if (substr_message == "SUDP"){
        send_over_udp = true;
        commandSocketSend(commandBuffer);
    }else if (substr_message == "NSUDP"){
        send_over_udp = false;
        commandSocketSend(commandBuffer);
    }else if (substr_message == "HVON"){
        hv_state = 0;
        commandSocketSend(commandBuffer);
    }else if (substr_message == "HVOFF"){
        hv_state = 1;
        commandSocketSend(commandBuffer);
    }else if (substr_message == "SDON"){
        save_to_sd = true;
        commandSocketSend(commandBuffer);
    }else if (substr_message == "SDOFF"){
        save_to_sd = false;
        commandSocketSend(commandBuffer);
    }else if (substr_message == "DATA"){
        received_packets++;
    }else if (substr_message == "RESET"){
        commandSocketSend(commandBuffer);
        mbed_reset();
    }else{
         pc.printf("Not recognized \n\r");
    }
    
}

// Easy function to send a string to our groundstation
void commandSocketSend(char responseBuffer[]){
    int socketSend = commSocket.sendTo(remoteCommPC,responseBuffer,strlen(responseBuffer));
    if (socketSend == -1){
        pc.printf("Send Failed \n\r");
    }
    else{
        pc.printf("Send Successful \n\r");
    }
}

// Methods called at start return 0 on success so with this we check if it succeeded or not
void executionCheck(int executedMethod){
    if (executedMethod == 0){
       pc.printf("Success \n\r");
    }
    else{
       pc.printf("Failed \n\r");
    }
}

void open_sd_dir(int& file_number){    
    DIR *d;
    struct dirent *p;
    while( (d = opendir("/sd/")) == NULL){
        debugPrint("SD failed to open");
        watchdogReset();
        pc.printf("Couldn't open SD dir\n\r");
        sd_error = 1;
        wait(1);
        sd_error = 0;
    }
    if ( (d = opendir("/sd/")) != NULL){            
        while( (p = readdir(d)) != NULL) { // als readdir faalt of op het einde van de directory is geeft het NULL
            string name = p->d_name; // name bevat de naam van de file in de directory d
            if (name.compare(0,2,"M_") == 0) { // als de eerste 2 chars van name hetzelfde zijn als M_ dan geeft het 0
                string num_str = name.substr(2, name.find(".")-2); //Zal vanaf het 2e element in name (vlak na _) het aantal charachters bepaald door 2e argument eruit halen. 2e Argument zoekt in name tot aan . en geeft de positie van . terug, als we hier -2 van trekken weten we hoeveel getalen er na _ kwamen (vb: M_12.txt, . op plaats 4 als we hiervan 2 trekken krijgen we 2 en dit zijn het aantal cijfers die er in de naam staan
                int num = atoi(num_str.c_str()); // zal eerst num_str naar een C-string converteren waarna atoi kan gebruikt worden. atoi zal de getallen in de string omzetten naar een int. 
                file_number = (num >= file_number)? num : file_number; // condition ? result_if_true : result_if_false
                file_number++;
            }
         }
         closedir(d); //Sluit als we aan het einde zijn van de directory
         pc.printf("Opened sd dir \n\r");
         debugPrint("SD opened succesfully");   
    }
}

void debugPrint(std::string debugMessage){
        const int debugBufferSize = 50;
        char debugBuffer[debugBufferSize];
        sprintf(debugBuffer, debugMessage.c_str());
        int socket_send = debugSocket.sendTo(remoteDebugPC, debugBuffer, debugBufferSize); 
}

void watchdogReset(){
    watchdog1 = !watchdog1;
    watchdog2 = !watchdog2;
    watchdog3 = !watchdog3;
}
