#include "interface.h"

void setup(){
	intializationSetup();
	displayButtons();
	bluetoothStatusInfo();
	startBus();
}
battery batteryData;
void loop(){
	getParam_B(b_DEVICE_NAME, &batteryData.DeviceName);
	getParam_U(SERIAL_NUMBER, &batteryData.SerialNumber);
	getParam_S(s_CURRENT, &batteryData.Current);
	getParam_U(VOLTAGE, &batteryData.Voltage);
	getParam_U(TEMPERATURE,&batteryData.Temperature);
	getParam_U(DESIGN_CAPACITY, &batteryData.DesignCapacity);
	getParam_U(RELATIVE_STATE_OF_CHARGE, &batteryData.RelativeStateOfCharge);
	getParam_U(FULL_CHARGE_CAPACITY, &batteryData.FullChargeCapacity);
	getParam_U(MANUFACTURER_ACCESS, &batteryData.ManufacturerAccess);
	getParam_U(REMAINING_CAPACITY_AlARM, &batteryData.RemainingCapacityAlarm);
	getParam_U(REMAINING_TIME_ALARM, &batteryData.RemainingTimeAlarm);
	getParam_U(BATTERY_MODE, &batteryData.BatteryMode);
	getParam_S(s_AT_RATE, &batteryData.AtRate);
	getParam_U(AT_RATE_TIME_TO_FULL, &batteryData.AtRateTimeToFull);
	getParam_U(AT_RATE_TIME_TO_EMPTY, &batteryData.AtRateTimeToEmpty);
	getParam_U(AT_RATE_OK, &batteryData.AtRateOK);
	getParam_B(b_DEVICE_CHEMISTRY, &batteryData.DeviceChemistry);
	displayData(currentPage,batteryData);
	ts.sample();
	isPressed = ts.isPressed();

	if (isPressed && !previousPressed) {

			tft.fillScreen(Color::White);
			displayButtons();
			bluetoothStatusInfo();

			currentPage++;

			if (currentPage > 1) {
				currentPage = 0;				
			} // end if
		previousPressed = true;
	} // end if 
	
	if (!isPressed) {
		previousPressed = false;
	} // end if
} // end loop
