#ifndef SMPS_PID_H_
#define SMPS_PID_H_

#include "Timer1.h"
#include "Hardware.h"

//MV - manipulated variable in PID
#define MAX_PID_MV_FACTOR 1.5
#define MAX_PID_MV ((uint16_t) (TIMER1_PRECISION_PERIOD * MAX_PID_MV_FACTOR))
#define PID_MV_PRECISION 8
#define MAX_PID_MV_PRECISION (((uint32_t) MAX_PID_MV)<<PID_MV_PRECISION)

namespace SMPS_PID
{
//    void init(uint16_t Vin, uint16_t Vout);
	void init(bool PID_V_mode, uint16_t Vin, uint16_t Vout);
    void setPID_MV(uint16_t value);
    void powerOn();
    void powerOff();
    void update();
};

#endif //SMPS_PID_H_
