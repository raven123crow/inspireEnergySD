#include <Picadillo.h>
#include <Arial.h>
#include <Liberation.h>
#include <Widgets.h>
#include <Wire.h>
//#include "Wire3\Wire.h"

// Constants

#define PORTRAIT  0  	                  // 0 degrees
#define ORIGIN    0     			      // zero coordinate
#define MAX_LIGHT 255                     // turn on maximum light for screen
#define UPPER_DISPLAY_SCREEN_OFFSET 200
#define RECT_BUTTON_OFFSET 40
#define BT_START_BOARDER 41
#define BT_END_BOARDER 65
#define BATT_BOARDER 1
#define FLIP ((unsigned char) 0xFF)      // used to flip bits for 2's complement
#define ONE 0x01                           // used to complete 2's complement
#define BATTERY_ADDR 11                    // battery location  // battery i2c write address, the read address is (BattAddr + 1) 11 battery, 26 mcu
#define CHAR_LENGTH 32                     // maximum character string for a Param_block read

#define MANUFACTURER_ACCESS        0x00 
#define REMAINING_CAPACITY_AlARM   0x01
#define REMAINING_TIME_ALARM       0x02
#define BATTERY_MODE               0x03
#define s_AT_RATE                   0X04
#define AT_RATE_TIME_TO_FULL       0x05
#define AT_RATE_TIME_TO_EMPTY      0x06
#define AT_RATE_OK                 0X07
#define TEMPERATURE                0x08
#define VOLTAGE                    0X09 
#define s_CURRENT                  0X0A 
#define s_AVERAGE_CURRENT          0x0B 
#define MAX_ERROR                  0x0C
#define RELATIVE_STATE_OF_CHARGE   0x0D
#define ABSOLUTE_STATE_OF_CHARGE   0x0E 
#define REMAINING_CAPACITY         0x0F 
#define FULL_CHARGE_CAPACITY       0x10
#define RUNTIME_TO_EMPTY           0x11 
#define AVERAGE_TIME_TO_EMPTY      0x12 
#define AVERAGE_TIME_TO_FULL       0x13
#define CHARGING_CURRENT           0x14 
#define CHARGING_VOLTAGE           0x15
#define BATTERY_STATUS             0x16
#define CYCLE_COUNT                0x17
#define DESIGN_CAPACITY            0x18
#define DESIGN_VOLTAGE             0x19
#define SPECIFICATION_INFO         0x1A
#define MANUFACTURER_DATE          0x1B
#define SERIAL_NUMBER              0x1C
#define b_MANUFACTURER_NAME        0x20
#define b_DEVICE_NAME              0x21
#define b_DEVICE_CHEMISTRY         0x22
#define b_MANUFACTURER_DATA        0x23 

// global variables

Picadillo tft;
AnalogTouch ts(LCD_XL, LCD_XR, LCD_YU, LCD_YD, 320, 480);  
twButton    up(ts, tft, ORIGIN, ORIGIN, 320, 40, "");
twButton    down(ts, tft, ORIGIN, 440, 320, 480, "");
bool isPressed = false;
bool previousPressed = false; 				// stores previous loops pressed value
bool isPreviousPageCleared = 0;
int pressure = 0;
int currentPage = 0; 						// currently displayed page counter
String s = " "; 

/*********************Structure*************************************/
struct { // battery parameter - signed word
    unsigned char lsb;
    unsigned char msb;
    signed short value;
} typedef Param_sword;


struct { // battery parameter - unsigned word
    unsigned char lsb;
    unsigned char msb;
    unsigned int value;
} typedef Param_uword;

struct { // battery parameter that is a character string
    char value[CHAR_LENGTH];
    int len;
} typedef Param_block;


struct {
    Param_uword ManufacturerAccess; 	// Code 0x00 - ManufacturerAccess()
    Param_uword RemainingCapacityAlarm; // Code 0x01 - RemainingCapacityAlarm()
    Param_uword RemainingTimeAlarm; 	// Code 0x02 - RemainingTimeAlarm()
    Param_uword BatteryMode; 			// Code 0x03 - BatteryMode()
    Param_sword AtRate; 				// Code 0x04 - AtRate()
    Param_uword AtRateTimeToFull; 		// Code 0x05 - AtRateTimeToFull() 
    Param_uword AtRateTimeToEmpty; 		// Code 0x06 - AtRateTimeToEmpty()
    Param_uword AtRateOK; 				// Code 0x07 - AtRateOK()
    Param_uword Temperature; 			// Code 0x08 - Temperature()
    Param_uword Voltage; 				// Code 0x09 - Voltage()
    Param_sword Current; 				// Code 0x0A - Current()
    Param_sword AverageCurrent;			// Code 0x0B - AverageCurrent()
    Param_uword MaxError; 				// Code 0x0C - MaxError()
    Param_uword RelativeStateOfCharge; 	// Code 0x0D - RelativeStateOfCharge()
    Param_uword AbsoluteStateOfCharge; 	// Code 0x0E - AbsoluteStateOfCharge()
    Param_uword RemainingCapacity; 		// Code 0x0F - RemainingCapacity()
    Param_uword FullChargeCapacity; 	// Code 0x10 - FullChargeCapacity()
    Param_uword RunTimeToEmpty; 		// Code 0x11 - RunTimeToEmpty()
    Param_uword AverageTimeToEmpty; 	// Code 0x12 - AverageTimeToEmpty()
    Param_uword AverageTimeToFull; 		// Code 0x13 - AverageTimeToFull()
    Param_uword ChargingCurrent; 		// Code 0x14 - ChargingCurrent()
    Param_uword ChargingVoltage; 		// Code 0x15 - ChargingVoltage()
    Param_uword BatteryStatus; 			// Code 0x16 - BatteryStatus()
    Param_uword CycleCount; 			// Code 0x17 - CycleCount()
    Param_uword DesignCapacity; 		// Code 0x18 - DesignCapacity()
    Param_uword DesignVoltage; 			// Code 0x19 - DesignVoltage()
    Param_uword SpecificationInfo; 		// Code 0x1A - SpecificationInfo()
    Param_uword ManufacturerDate; 		// Code 0x1B - ManufacturerDate()
    Param_uword SerialNumber; 			// Code 0x1C - SerialNumber()
    // Code 0x1D - Reserved
    // Code 0x1E - Reserved
    // Code 0x1F - Reserved
    Param_block ManufacturerName; 		// Code 0x20 - ManufacturerName()
    Param_block DeviceName; 			// Code 0x21 - DeviceName()
    Param_block DeviceChemistry; 		// Code 0x22 - DeviceChemistry()
    Param_block ManufacturerData; 		// Code 0x23 - ManufacturerData()
} typedef battery;

battery batt;

/*********************Function Declarations*************************/
void intializationSetup();
void displayButtons();
void drawUpArrow();
void drawDownArrow();
void bluetoothStatusInfo();
void drawBtBoarder();
void displayData (int pg);
void displayBlock(Param_block pb);


/*********************Function Definitions*************************/

/*****************intializationSetup******************************/
void intializationSetup(){
	analogWrite(PIN_BACKLIGHT, MAX_LIGHT); // 0 is off and 255 is on
 
	tft.initializeDevice();
	tft.setRotation(PORTRAIT);   			// sets orientation of the display
	
	ts.initializeDevice();
	ts.setRotation(PORTRAIT);   			// sets the orientation of the touch screen
	//ts.scaleX(4.3);              			// set the touch screen area x // 4.3
	//ts.scaleY(3.3);             			// set the touch screen area y  // 3.3

	tft.fillScreen(Color::White);
	tft.setTextColor(Color::Black, Color::White);
	tft.setFont(Fonts::Arial30);  				// 20
	int centerX = tft.getWidth()/2 - 90;    	// set center coordinate for x axis    
	int centerY = tft.getHeight()/2.5;      	// set center coordinate for y axis
	tft.setCursor(centerX,centerY);        		// set location of cursor to center
	tft.print("Inspired\n");	
	tft.setCursor(centerX, centerY+ 40); 		// move cursor to the next available line
	tft.print("Energy\n");
	
    delay(1000);
	tft.fillScreen(Color::White);
	tft.setCursor(ORIGIN,ORIGIN);
	
	ts.sample();
}

/*****************setButton******************************************/
void displayButtons(){
	drawUpArrow();
	drawDownArrow();
}

/*****************drawUpArrow***************************************/
void drawUpArrow(){
	up.setBevel(4);
	up.render();
	int centerX = tft.getWidth()/2;
	int x0 = centerX;
	int x1 = centerX - 10;     // shift x coordinates left by 10 pixels from center
	int x2 = centerX + 10;     // shift x coordinates right by 10 pixels from center 
    int y0 = ORIGIN + 5;       // move y coordinate 5 pixels down from orgin
    int y1 = ORIGIN + 35;      // move y coordinate 35 pixels down from orgin
	int y2 = y1;
	uint16_t color = rand();
	tft.fillTriangle(x0,y0,x1,y1,x2,y2,color);
	color = rand();
	tft.drawTriangle(x0,y0,x1,y1,x2,y2,color);
}

/*****************drawDownArrow**************************************/
void drawDownArrow(){
	down.setBevel(4);
	down.render();
	int centerX = tft.getWidth()/2;
	int centerY = tft.getHeight()/2;
	int x0 = centerX;
	int x1 = centerX + 10;     		// shift x coordinates left by 10 pixels from center
	int x2 = centerX - 10;     		// shift x coordinates right by 10 pixels from center 
    int y0 = centerY + 235;        // move y coordinate 5 pixels down from orgin
    int y1 = centerY + 205;        // move y coordinate 35 pixels down from orgin
	int y2 = y1;
	uint16_t color = rand();       //237 colors
	tft.fillTriangle(x0,y0,x1,y1,x2,y2,color);
	color = rand();
	tft.drawTriangle(x0,y0,x1,y1,x2,y2,color);
}

/*****************bluetoothStatusInfo***********************************/
void bluetoothStatusInfo(){
	drawBtBoarder();	
}

/******************drawBtBoarder**************************************/
void drawBtBoarder(){
	int width = tft.getWidth();
	tft.drawHorizontalLine(ORIGIN, BT_START_BOARDER, width, Color::NavyBlue);
	tft.drawHorizontalLine(ORIGIN, BT_END_BOARDER + RECT_BUTTON_OFFSET, width, Color::NavyBlue);
}

/******************bluetoothData**************************************/
void bluetoothData(){
	
}

/******************batteryinfo****************************************/
void batteryInfo(){
	
}

/******************drawBatteryBoarder**************************************/
void drawBatteryBoarder(){
	int width = tft.getWidth();
	tft.drawHorizontalLine(ORIGIN, BT_START_BOARDER, width, Color::Peru);
}

void displayData (int pg, battery data) {
	tft.setFont(Fonts::Liberation14);
	tft.setTextColor(Color::Black, Color::White);

/*
	int page = 1;
	
	switch page {
		case 1 :
			showPage(1);
		break;

		case 2 :
			showPage(2);
		break;

		case 3 :
			showPage(3);
		break;

		case 4 :
			showPage(4);
		break;

		case default :
			page = 1;
			showPage(1);
		break;
		
	}
*/
	
		if (pg == 0) {
			tft.setCursor(ORIGIN,BATT_BOARDER + BT_END_BOARDER + RECT_BUTTON_OFFSET);
			delay(200);

			tft.print("Device Name : ");
			tft.print(data.DeviceName.len);
			tft.print("\n");

			tft.print("Serial Num  : ");
			tft.print(data.SerialNumber.value);
			tft.print("\n");

			tft.print("Current     : ");
			tft.print(((float)data.Current.value)/1000); 
			tft.print(" A\n");

			tft.print("Voltage     : ");
			tft.print(((float)data.Voltage.value)/1000);
			tft.print(" V\n");

            tft.print("Temp        : ");
			tft.print(((float)data.Temperature.value / 10)-273.15);
			tft.print(" C\n");

			tft.print("Design Cap  : ");
			tft.print(data.DesignCapacity.value);
			tft.print("U\n");

            tft.print("Rel SoC     : ");
            tft.print(data.RelativeStateOfCharge.value);
			tft.print(" %\n"); 

            tft.print("Full Chg Cap: ");
            tft.print(data.FullChargeCapacity.value);
            tft.print(" \n");

            tft.print("Man Access  : ");
            tft.print(data.ManufacturerAccess.value);
            tft.print("\n");

			tft.print("Rem Cap Alm : ");
			tft.print(data.RemainingCapacityAlarm.value);
			tft.print("\n");

            tft.print("Rem Time Alm: ");
            tft.print(data.RemainingTimeAlarm.value);
            tft.print("\n");

            tft.print("Batt Mode  : ");
            tft.print(data.BatteryMode.value);
            tft.print("\n");

			tft.print("AtRate     : ");
			tft.print(data.AtRate.value);
			tft.print("\n");

			tft.print("AtRate TTF : ");
			tft.print(data.AtRateTimeToFull.value);
			tft.print("\n");
			
			tft.print("AtRate TTE : ");
			tft.print(data.AtRateTimeToEmpty.value);
			tft.print("\n");

      		tft.print("AtRate OK  : ");
      		tft.print(data.AtRateOK.value);
      		tft.print("\n");

      		
			
		} else {
			tft.setCursor(ORIGIN,BATT_BOARDER + BT_END_BOARDER + RECT_BUTTON_OFFSET);

			tft.print("Avg Current : ");
      		tft.print(data.AverageCurrent.value);
      		tft.print("\n");
			
			tft.print("Abs SoC     : ");
			tft.print(" 81.2 %\n"); 

			tft.print("Device Chem : ");
			displayBlock(data.DeviceChemistry);
			tft.print("\n");
			
			tft.print("Relative State of Charge:\n");
			tft.print(" 80 %\n\n"); 
			tft.print("Absolute State of Charge:\n");
			tft.print(" 81.2 %\n\n"); 
			tft.print("Average Time to Empty:\n");
			tft.print(" 73 min\n\n");
		}

	}
/*********************startBus()*********************************************/
void startBus() {
	Wire.begin(); 						// join i2c bus (address optional for master)
}

void getParam_U(unsigned char cmd, Param_uword* pu) {
	Wire.beginTransmission(BATTERY_ADDR);
	Wire.write(cmd);
	Wire.endTransmission();
	Wire.requestFrom(BATTERY_ADDR, 2); 	// request 2 bytes from battery
	pu->lsb = Wire.read();
	pu->msb = Wire.read();
    									// combine the MSB + LSB into one value for displaying
    									// bit-shift the MSB to the left by 8 bits, moving it to the MSB position
	pu->value = pu->msb << 8;
	pu->value = pu->value ^ pu->lsb; 	// bitwise XOR the LSB with the value to place it in the LSB position
}

void getParam_S(unsigned char cmd, Param_sword* ps) {
	Wire.beginTransmission(BATTERY_ADDR);
	Wire.write(cmd);
	Wire.endTransmission();
	Wire.requestFrom(BATTERY_ADDR, 2); 	// request 2 bytes from battery
	ps->lsb = Wire.read();
	ps->msb = Wire.read();
	ps->value = ps->msb << 8;
	ps->value = ps->value ^ ps->lsb; 	// bitwise XOR the LSB with the value to place it in the LSB position
}

void getParam_B(unsigned char cmd, Param_block* pb) {
	Wire.beginTransmission(BATTERY_ADDR);
	Wire.write(cmd);
	Wire.endTransmission();
	Wire.requestFrom(BATTERY_ADDR, 1);	 // request the first byte which holds the length of the character array
	pb->len = Wire.read();
	Wire.requestFrom(BATTERY_ADDR, pb->len);

	for(int i = 0; i < pb->len; i++) {
		pb->value[i] = Wire.read();
	}
}

void displayBlock(Param_block pb) {
	for (int i = 0; i < pb.len; i++){
		tft.print(pb.value[i]);
	}
}