#include "interface.h"

void setup(){
	intializationSetup();
	displayButtons();
	bluetoothStatusInfo();
	startBus();
}
battery batteryData;
int y = 0;   // y coordinate. no need to check x coordinate
void loop(){
	setBatteryParams();
	displayData(g_currentPage,batteryData);
	bluetoothStatusInfo();
	ts.sample();
	isPressed = ts.isPressed();

	if (isPressed && !previousPressed) {
		y = ts.y();

		if (y > 0 && y < 241){            // magic number 0 is the start of the bluetooth line (y axis), magic number 241 is the middle of the y axis
			tft.fillScreen(Color::White); 
			displayButtons();
			g_currentPage++;
		} else if (y > 240 && y < 506){  // magic number 240 the midway of the y axis and magic number 506 is the bottom of the y-axis
			tft.fillScreen(Color::White);
			displayButtons();
			g_currentPage--;
		}
		  else {                         // for some reason, the very top of the lcd to the buttom end of the up arrow is reading a range of 65507-65554
		  	tft.fillScreen(Color::White);
			displayButtons();
			g_currentPage++; 
		}
		
		if (g_currentPage > 3) {
			g_currentPage = 1;				
		} else if (g_currentPage < 1) {
			g_currentPage = 3;
		}
		previousPressed = true;
	} // end if 
	
	if (!isPressed) {
		previousPressed = false;
	} // end if
} // end loop

void setBatteryParams(){
	// page 1
	setParam_B(b_DEVICE_NAME, &batteryData.DeviceName);
	setParam_U(SERIAL_NUMBER, &batteryData.SerialNumber);
	setParam_S(s_CURRENT, &batteryData.Current);
	setParam_U(VOLTAGE, &batteryData.Voltage);
	setParam_U(TEMPERATURE,&batteryData.Temperature);
	setParam_U(DESIGN_CAPACITY, &batteryData.DesignCapacity);
	setParam_U(RELATIVE_STATE_OF_CHARGE, &batteryData.RelativeStateOfCharge);
	setParam_U(FULL_CHARGE_CAPACITY, &batteryData.FullChargeCapacity);
	setParam_U(MANUFACTURER_ACCESS, &batteryData.ManufacturerAccess);
	setParam_U(REMAINING_CAPACITY_AlARM, &batteryData.RemainingCapacityAlarm);
	setParam_U(REMAINING_TIME_ALARM, &batteryData.RemainingTimeAlarm);
	
 	// page 2
 	setParam_U(BATTERY_MODE, &batteryData.BatteryMode);
	setParam_S(s_AT_RATE, &batteryData.AtRate);
	setParam_U(AT_RATE_TIME_TO_FULL, &batteryData.AtRateTimeToFull);
	setParam_U(AT_RATE_TIME_TO_EMPTY, &batteryData.AtRateTimeToEmpty);
	setParam_U(AT_RATE_OK, &batteryData.AtRateOK);
	setParam_S(s_AVERAGE_CURRENT, &batteryData.AverageCurrent);
	setParam_U(MAX_ERROR, &batteryData.MaxError);
	setParam_U(ABSOLUTE_STATE_OF_CHARGE, &batteryData.AbsoluteStateOfCharge);
	setParam_U(REMAINING_CAPACITY, &batteryData.RemainingCapacity);
	setParam_U(RUNTIME_TO_EMPTY, &batteryData.RunTimeToEmpty);
	setParam_U(AVERAGE_TIME_TO_EMPTY, &batteryData.AverageTimeToEmpty);

	// page 3	
	setParam_U(AVERAGE_TIME_TO_FULL, &batteryData.AverageTimeToFull);
	setParam_U(CHARGING_CURRENT, &batteryData.ChargingCurrent);
	setParam_U(CHARGING_VOLTAGE, &batteryData.ChargingVoltage);
	setParam_U(BATTERY_STATUS, &batteryData.BatteryStatus);
	setParam_U(CYCLE_COUNT, &batteryData.CycleCount);
	setParam_U(DESIGN_VOLTAGE, &batteryData.DesignVoltage);
	setParam_U(SPECIFICATION_INFO, &batteryData.SpecificationInfo);
	setParam_U(MANUFACTURER_DATE, &batteryData.ManufacturerDate);
	setParam_B(b_MANUFACTURER_NAME, &batteryData.ManufacturerName);
	setParam_B(b_DEVICE_CHEMISTRY, &batteryData.DeviceChemistry);
	setParam_B(b_MANUFACTURER_DATA, &batteryData.ManufacturerData); 
}
