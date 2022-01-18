#ifndef __FIFO__
#define __FIFO__

#include "mbed.h"
#include "uop_msb.h"
#include "SD_WRAPPER.hpp"
#include <cstdint>
using namespace uop_msb;

    const int _FIFO_size = 6;
    //static int _startWrite;
    static ENVDATA _env_data;
    static ENVDATA _env_data_arr[_FIFO_size];
    
    class FIFOmessage_t {
    public:
        ENVDATA _msg_env_data;

        //Constructor
        FIFOmessage_t(float t, float p, float l){
            _msg_env_data.temp = t;
            _msg_env_data.pres = p;
            _msg_env_data.light = l;
            _msg_env_data.time = 1200;
        }
    };

    class UOP_MSB_SENSORDATA {
    protected:
        typedef enum {START_WRITE=1, NO_PRESS=2} FIFO_EVENTS;
        Thread sensorThread, writeThread, readThread, alarmThread;
        //Light Levels
        AnalogIn ldr_sensors;
        //Environmental Sensor
        uop_msb::EnvSensor env;
        //LED Matrix
        SPI matrix_spi;   // MOSI, MISO, SCLK
        DigitalOut matrix_spi_cs;           //Chip Select ACTIVE LOW
        DigitalOut matrix_spi_oe;           //Output Enable ACTIVE LOW
        //User Switch
        DigitalIn userButton;
        //temperature upper/lower
        float t_up = 27;
        float t_low = 18;
        //pressure upper/lower
        float p_up = 1200;
        float p_low = 900;
        //light level upper/lower
        float l_up = 0.4;
        float l_low = 0.1;

    public:
        //Constructor
        UOP_MSB_SENSORDATA() : 
        ldr_sensors(AN_LDR_PIN), matrix_spi(PC_12, PC_11, PC_10), matrix_spi_cs(PB_6), matrix_spi_oe(PB_12), userButton(USER_BUTTON){
            alarmThread.start(callback(this, &UOP_MSB_SENSORDATA::alarm));
        }

        void alarm();

    };

    class FIFO : UOP_MSB_SENSORDATA {
    private:
        Mutex sdLock;
        Semaphore samplesInBuffer;
        Semaphore spaceInBuffer;

        Mail<FIFOmessage_t, _FIFO_size> FIFO_mail;

        UOP_MSB_SENSORDATA FIFO_board;

        SD_WRAPPER SD_abstract;

    public:

        void write_FIFO();
        void read_FIFO();
        void read_sensors();

        //Constructor
        FIFO() : 
        sdLock(), samplesInBuffer(), spaceInBuffer(_FIFO_size), FIFO_board(){
            sensorThread.start(callback(this, &FIFO::read_sensors));
            sensorThread.set_priority(osPriorityHigh);
            writeThread.start(callback(this, &FIFO::write_FIFO));
            readThread.start(callback(this, &FIFO::read_FIFO));
        }
    };

    /*class SD {
    public:
        void static write_sdcard(FIFOmessage_t* readFIFO);
        void static read_sdcard();
    };*/

#endif