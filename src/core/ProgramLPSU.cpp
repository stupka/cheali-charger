/*
    cheali-charger - open source firmware for a variety of LiPo chargers
    Copyright (C) 2013  Paweł Stawicki. All right reserved.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "ProgramLPSU.h"
#include "Hardware.h"
#include "ProgramData.h"
#include "LcdPrint.h"
#include "Utils.h"
#include "Screen.h"
#include "ScreenMethods.h"
#include "ScreenCycle.h"
#include "LabPowerSupplyStrategy.h"
#include "Monitor.h"
#include "memory.h"
#include "StartInfoStrategy.h"
#include "Buzzer.h"
#include "StaticMenu.h"
#include "Settings.h"
#include "SerialLog.h"
#include "ProgramDCcycle.h"
#include "Calibrate.h"
#include "SMPS_PID.h"
#include "Blink.h"

namespace ProgramLPSU {
    ProgramState programState = ProgramLPSU::Done;
    const char * stopReason;
    bool exitImmediately;
    Blink blink;
    uint16_t LPSU_U_value, LPSU_Imax_value;

    enum LPSU_EditMode {LPSU_Edit_U, LPSU_Edit_Imax, LPSU_Edit_None};
	LPSU_EditMode lpsuEditMode;	
/*
    void setupStorage();
    void setupTheveninCharge();
    void setupDischarge();
    void setupBalance();
    void setupDeltaCharge();
*/
} //namespace ProgramLPSU


/*
void Program::setupStorage()
{
    Strategy::strategy = &StorageStrategy::vtable;
}
void Program::setupTheveninCharge()
{
    Strategy::setVI(ProgramData::VCharge, true);
    Strategy::strategy = &TheveninChargeStrategy::vtable;
}

void Program::setupDeltaCharge()
{
    Strategy::setVI(ProgramData::VCharge, true);
    Strategy::strategy = &DeltaChargeStrategy::vtable;
}

void Program::setupDischarge()
{
    Strategy::setVI(ProgramData::VDischarge, false);
    Strategy::strategy = &TheveninDischargeStrategy::vtable;
}

void Program::setupBalance()
{
    Strategy::strategy = &Balancer::vtable;
}

Strategy::statusType Program::runWithoutInfo(ProgramType prog)
{
    Strategy::minIdiv = settings.minIoutDiv;
    Strategy::doBalance = false;

    switch(prog) {
    case Program::Charge:
        if(ProgramData::currentProgramData.isNiXX()) {
            setupDeltaCharge();
        } else {
            setupTheveninCharge();
        }
        break;
    case Program::ChargeBalance:
        Strategy::doBalance = true;
        setupTheveninCharge();
        break;
    case Program::Balance:
        setupBalance();
        break;
    case Program::Discharge:
        setupDischarge();
        break;
    case Program::FastCharge:
        Strategy::minIdiv = 5;
        setupTheveninCharge();
        break;
    case Program::Storage:
        setupStorage();
        break;
    case Program::StorageBalance:
        Strategy::doBalance = true;
        setupStorage();
        break;
    case Program::CapacityCheck:
        return ProgramDCcycle::runDCcycle(1, 3);

    case Program::DischargeChargeCycle:
        return ProgramDCcycle::runDCcycle(0, settings.DCcycles*2 - 1);
    default:
        return Strategy::ERROR;
    }
    return Strategy::doStrategy();
}
*/

void ProgramLPSU::displayLPSUScreen()
{
/*
    lcdSetCursor0_0();
    PowerSupplyData::currentPowerSupplyData.printVoltageString();
    PowerSupplyData::currentPowerSupplyData.printMaxCurrentString();
    lcdPrintSpaces();
*/

    lcdSetCursor0_0();

    if (blink.getBlinkOff() and (lpsuEditMode == LPSU_Edit_U)) { lcdPrintSpaces(7); }
        else { lcdPrintAnalog(LPSU_U_value, getType(AnalogInputs::Vout), 7); }

    if (blink.getBlinkOff() and (lpsuEditMode == LPSU_Edit_Imax)) { lcdPrintSpaces(7); }
        else { lcdPrintAnalog(LPSU_Imax_value, getType(AnalogInputs::Iout), 7); }
    lcdPrintSpaces();


    lcdSetCursor0_1();
//    AnalogInputs::printRealValue(AnalogInputs::VoutBalancer,     7);

    AnalogInputs::printRealValue(AnalogInputs::Vout, 7);
    AnalogInputs::printRealValue(AnalogInputs::Iout,     7);
    lcdPrintSpaces();
}




void ProgramLPSU::run()
{
    if(!Calibrate::check())
        return;

    stopReason = NULL;
    programState = Info;

    AnalogInputs::powerOn();

    programState = InProgress;

    lpsuEditMode = LPSU_Edit_None;

    Monitor::powerOn();
    SerialLog::powerOn();
    Screen::powerOn();

    Buzzer::soundStartProgram();

/*
//    Strategy::strategy = &LabPowerSupplyStrategy::vtable;
    Strategy::exitImmediately = false;
    Strategy::minIdiv = settings.minIoutDiv;
    Strategy::doBalance = false;

    Strategy::doStrategy();
*/

//  doStrategy

        Screen::keyboardButton = BUTTON_NONE;
        bool run = true;
        uint16_t newMesurmentData = 0;
        Strategy::statusType status = Strategy::RUNNING;
        LPSU_U_value = ANALOG_VOLT(5.000); 
        LPSU_Imax_value = ANALOG_AMP(0.500);
//        strategyPowerOn();
        blink.startBlinkOff(0);
        SMPS::powerOn(true);
        do {
            Screen::keyboardButton =  Keyboard::getPressedWithSpeed();

		    switch(lpsuEditMode) {
				case LPSU_Edit_None: 
					if(Screen::keyboardButton == BUTTON_INC) lpsuEditMode = LPSU_Edit_U; break;
				case LPSU_Edit_U: {
					if(Screen::keyboardButton == BUTTON_START) lpsuEditMode = LPSU_Edit_Imax; 
					if(Screen::keyboardButton == BUTTON_INC) LPSU_U_value += ANALOG_VOLT(0.100); 
					if(Screen::keyboardButton == BUTTON_DEC) LPSU_U_value -= ANALOG_VOLT(0.100); 
					break;
				}
				case LPSU_Edit_Imax:
					if(Screen::keyboardButton == BUTTON_START) lpsuEditMode = LPSU_Edit_None;
					if(Screen::keyboardButton == BUTTON_INC) LPSU_Imax_value += ANALOG_AMP(0.100);
					if(Screen::keyboardButton == BUTTON_DEC) LPSU_Imax_value -= ANALOG_AMP(0.100);
					break;

			}
			
			
//            if(Screen::keyboardButton == BUTTON_INC) LPSU_U_value +=1000;
//            if(Screen::keyboardButton == BUTTON_DEC) LPSU_U_value -=1000;

//            Screen::doStrategy();
            displayLPSUScreen();

            if(run) {
                status = Monitor::run();
//                run = analizeStrategyStatus(status);

                if(run && newMesurmentData != AnalogInputs::getFullMeasurementCount()) {
                    newMesurmentData = AnalogInputs::getFullMeasurementCount();
//                      SMPS_PID::setPID_MV(4000);
                      SMPS::setValue(AnalogInputs::reverseCalibrateValue(AnalogInputs::Vout_plus_pin, LPSU_U_value));
//		      SMPS::trySetIout(1000);
//                    status = strategyDoStrategy();
//                    run = analizeStrategyStatus(status);
                }
            }
            if(!run && exitImmediately && status != Strategy::ERROR)
                break;
            blink.incBlinkTime();
        } while(Screen::keyboardButton != BUTTON_STOP);
        blink.stopBlink();
        SMPS::powerOff();
//        strategyPowerOff();

// end doStrategy


    Monitor::powerOff();

    AnalogInputs::powerOff();
    SerialLog::powerOff();
    Screen::powerOff();
}
