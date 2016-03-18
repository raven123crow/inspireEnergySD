#include <Picadillo.h>
#include <Arial.h>
#include <Liberation.h>
#include <Widgets.h>
#include <Wire.h>
#include <SD.h>
#include <BMPFile.h>
#include <Raw565.h>
#include "IEBMP.h"



//#include "Wire3\Wire.h"

// Constants

#define PORTRAIT  0  	                      // 0 degrees
#define ORIGIN    0     			              // zero coordinate
#define MAX_LIGHT 255                       // turn on maximum light for screen
#define UPPER_DISPLAY_SCREEN_OFFSET 200
#define RECT_BUTTON_OFFSET 40
#define BT_START_BOARDER 41
#define BT_END_BOARDER 25                   // 65
#define BATT_BOARDER 1
#define FLIP ((unsigned char) 0xFF)         // used to flip bits for 2's complement
#define ONE 0x01                            // used to complete 2's complement
#define BATTERY_ADDR 26                     // battery location  // battery i2c write address, the read address is (BattAddr + 1) 11 battery(22 without loss of bit), 26 mcu
#define CHAR_LENGTH 32                      // maximum character string for a Param_block read
#define LAST_PAGE 3
#define BYTE 8
#define FLUSH getchar() != '\n'
#define PIN_SD_SS 48

#define BT_STATUS	17

#define MANUFACTURER_ACCESS        0x00 
#define REMAINING_CAPACITY_AlARM   0x01
#define REMAINING_TIME_ALARM       0x02
#define BATTERY_MODE               0x03
#define s_AT_RATE                  0X04
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
twButton    up(ts, tft, ORIGIN+1, ORIGIN+1, 316, 35, "");       // brought the buttons in 1 pixel from the edges, looks better to me. -MW
twButton    down(ts, tft, ORIGIN+1, 444, 316, 35, "");
bool isPressed = false;
bool previousPressed = false; 				                          // stores previous loops pressed value
bool isPreviousPageCleared = 0;
int pressure = 0;
int g_currentPage = 1; 						                              // currently displayed page counter
String s = " "; 
File f;
Raw565 ieLogo(ieLogo_data, 320, 160);                           // Inspired Energy Logo


/*********************Structure*************************************/
struct {                                                        // battery parameter - signed word
    unsigned char lsb;
    unsigned char msb;
    signed short value;
} typedef Param_sword;


struct {                                                       // battery parameter - unsigned word
    unsigned char lsb;
    unsigned char msb;
    unsigned int value;
} typedef Param_uword;

struct {                                                      // battery parameter that is a character string
    char value[CHAR_LENGTH];
    int len;
} typedef Param_block;


struct {
    Param_uword ManufacturerAccess; 	                        // Code 0x00 - ManufacturerAccess()
    Param_uword RemainingCapacityAlarm;                       // Code 0x01 - RemainingCapacityAlarm()
    Param_uword RemainingTimeAlarm; 	                        // Code 0x02 - RemainingTimeAlarm()
    Param_uword BatteryMode; 			                            // Code 0x03 - BatteryMode()
    Param_sword AtRate; 				                              // Code 0x04 - AtRate()
    Param_uword AtRateTimeToFull; 		                        // Code 0x05 - AtRateTimeToFull() 
    Param_uword AtRateTimeToEmpty; 		                        // Code 0x06 - AtRateTimeToEmpty()
    Param_uword AtRateOK; 				                            // Code 0x07 - AtRateOK()
    Param_uword Temperature; 			                            // Code 0x08 - Temperature()
    Param_uword Voltage; 				                              // Code 0x09 - Voltage()
    Param_sword Current; 				                              // Code 0x0A - Current()
    Param_sword AverageCurrent;			                          // Code 0x0B - AverageCurrent()
    Param_uword MaxError; 				                            // Code 0x0C - MaxError()
    Param_uword RelativeStateOfCharge; 	                      // Code 0x0D - RelativeStateOfCharge()
    Param_uword AbsoluteStateOfCharge; 	                      // Code 0x0E - AbsoluteStateOfCharge()
    Param_uword RemainingCapacity; 		                        // Code 0x0F - RemainingCapacity()
    Param_uword FullChargeCapacity; 	                        // Code 0x10 - FullChargeCapacity()
    Param_uword RunTimeToEmpty; 		                          // Code 0x11 - RunTimeToEmpty()
    Param_uword AverageTimeToEmpty; 	                        // Code 0x12 - AverageTimeToEmpty()
    Param_uword AverageTimeToFull; 		                        // Code 0x13 - AverageTimeToFull()
    Param_uword ChargingCurrent; 		                          // Code 0x14 - ChargingCurrent()
    Param_uword ChargingVoltage; 		                          // Code 0x15 - ChargingVoltage()
    Param_uword BatteryStatus; 			                          // Code 0x16 - BatteryStatus()
    Param_uword CycleCount; 			                            // Code 0x17 - CycleCount()
    Param_uword DesignCapacity; 		                          // Code 0x18 - DesignCapacity()
    Param_uword DesignVoltage; 			                          // Code 0x19 - DesignVoltage()
    Param_uword SpecificationInfo; 		                        // Code 0x1A - SpecificationInfo()
    Param_uword ManufacturerDate; 		                        // Code 0x1B - ManufacturerDate()
    Param_uword SerialNumber; 			                          // Code 0x1C - SerialNumber()
    // Code 0x1D - Reserved
    // Code 0x1E - Reserved
    // Code 0x1F - Reserved
    Param_block ManufacturerName; 		                        // Code 0x20 - ManufacturerName()
    Param_block DeviceName; 			                            // Code 0x21 - DeviceName()
    Param_block DeviceChemistry; 		                          // Code 0x22 - DeviceChemistry()
    Param_block ManufacturerData; 		                        // Code 0x23 - ManufacturerData()
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
void showPage(int pg, battery data);
void setWaterMarkBackground();
void getBluetoothData();

/*********************Function Definitions*************************/

/*****************intializationSetup******************************/
void intializationSetup(){
	analogWrite(PIN_BACKLIGHT, MAX_LIGHT);                      // 0 is off and 255 is on
	tft.initializeDevice();
	tft.setRotation(PORTRAIT);   			                          // sets orientation of the display
	ts.initializeDevice();
	ts.setRotation(PORTRAIT);   			                          // sets the orientation of the touch screen
	tft.fillScreen(Color::White);
	tft.setFont(Fonts::Arial30);  				                      // 20
	int x = 0;                          	                      //  set center coordinate for x axis    
	int y = tft.getHeight()/3;      	                          // set center coordinate for y axis
  ieLogo.draw(&tft, x, y);
  delay(5000);
	tft.fillScreen(Color::Black);
	tft.setCursor(ORIGIN,ORIGIN);
	ts.sample();
}

/*****************setButton******************************************/
void displayButtons(){
	up.draw(&tft, ORIGIN+1, ORIGIN+1);                         // thou shalt redraw thine buttons if thou desirest them to not disappear when changing pages :) -MW
	down.draw(&tft, ORIGIN+1, 444);

	drawUpArrow();
	drawDownArrow();
}

/*****************drawUpArrow***************************************/
void drawUpArrow(){
	up.setBevel(4);
	up.render();
	int centerX = tft.getWidth()/2;
	int x0 = centerX;
	int x1 = centerX - 10;                                    // shift x coordinates left by 10 pixels from center
	int x2 = centerX + 10;                                    // shift x coordinates right by 10 pixels from center 
    int y0 = ORIGIN + 5;                                    // move y coordinate 5 pixels down from orgin
    int y1 = ORIGIN + 30;                                   // move y coordinate 35 pixels down from orgin
	int y2 = y1;
	tft.fillTriangle(x0,y0,x1,y1,x2,y2,Color::Wheat);
	tft.drawTriangle(x0,y0,x1,y1,x2,y2,Color::Gold);
}

/*****************drawDownArrow**************************************/
void drawDownArrow(){
	down.setBevel(4);
	down.render();
	int centerX = tft.getWidth()/2;
	int centerY = tft.getHeight()/2;
	int x0 = centerX;
	int x1 = centerX + 10;     		                          // shift x coordinates left by 10 pixels from center
	int x2 = centerX - 10;     		                          // shift x coordinates right by 10 pixels from center 
    int y0 = centerY + 235;                               // move y coordinate 5 pixels down from orgin
    int y1 = centerY + 210;                               // move y coordinate 35 pixels down from orgin  205
	int y2 = y1;
	tft.fillTriangle(x0,y0,x1,y1,x2,y2,Color::Wheat);
	tft.drawTriangle(x0,y0,x1,y1,x2,y2,Color::Gold);
}

/*****************bluetoothStatusInfo***********************************/
void bluetoothStatusInfo(){
	getBluetoothData();
	drawBtBoarder();	
}

/******************drawBtBoarder**************************************/
void drawBtBoarder(){
	int width = tft.getWidth();
	tft.drawHorizontalLine(ORIGIN, BT_START_BOARDER, width, Color::NavyBlue);
	tft.drawHorizontalLine(ORIGIN, BT_END_BOARDER + RECT_BUTTON_OFFSET, width, Color::NavyBlue);
}

/******************bluetoothData**************************************/
void getBluetoothData(){
	int x_status = 0;
	int y_status = 44;

	tft.setFont(Fonts::Liberation12);                       
	tft.setTextColor(Color::Orange, Color::Black); 
	tft.setCursor(x_status, y_status); 
	tft.print("BT:");
	if (digitalRead(BT_STATUS)){                             // display bt connection status by reading pin 17 (A3)
		tft.print(" PAIRED    ");
		
	} else {
		tft.print(" NOT PAIRED");
	}
}

/******************batteryinfo****************************************/
void batteryInfo(){
	// call displayData
}

/******************drawBatteryBoarder**************************************/
void drawBatteryBoarder(){
	int width = tft.getWidth();
	tft.drawHorizontalLine(ORIGIN, BT_START_BOARDER, width, Color::Peru);
}

void displayData (int pg, battery data) {

	tft.setFont(Fonts::Liberation12);                       
	
	switch (pg) {
		case 1 :
			showPage(pg, data);
		break;

		case 2 :
			showPage(pg, data);
		break;

		case 3 :
			showPage(pg, data);
		break;

		default :                                               // This shouldn't happen, but just in case it does!
			if (pg > LAST_PAGE) {                                 // currently in the last page circle back to first page
				pg = 1;                                             // set to first page
				showPage(pg, data);
			} else {
				pg = LAST_PAGE;                                     // currently in the first page circle back to last page
				showPage(pg, data);
			}
		break;
	}	
}
/*********************startBus()*********************************************/
void startBus() {
	Wire.begin(); 						                                // join i2c bus (address optional for master)
}

void setParam_U(unsigned char cmd, Param_uword* pu) {
 
	Wire.beginTransmission(BATTERY_ADDR);
	Wire.send(cmd);
	Wire.endTransmission();
	Wire.requestFrom(BATTERY_ADDR, 2);                      	// request 2 bytes from battery
	pu->lsb = Wire.receive();
	pu->msb = Wire.receive();
    									                                      // combine the MSB + LSB into one value for displaying
	pu->value = pu->msb << BYTE;                              // bit-shift the MSB to the left by 8 bits, moving it to the MSB position
	pu->value = pu->value ^ pu->lsb; 	                        // bitwise XOR the LSB with the value to place it in the LSB position
}

void setParam_S(unsigned char cmd, Param_sword* ps) {
  
	Wire.beginTransmission(BATTERY_ADDR);
	Wire.write(cmd);
	Wire.endTransmission();
	Wire.requestFrom(BATTERY_ADDR, 2); 	                      // request 2 bytes from battery
	ps->lsb = Wire.read();
	ps->msb = Wire.read();
	ps->value = ps->msb << BYTE;
	ps->value = ps->value ^ ps->lsb; 	                        // bitwise XOR the LSB with the value to place it in the LSB position
 
}

void setParam_B(unsigned char cmd, Param_block* pb) {
 
	Wire.beginTransmission(BATTERY_ADDR);
	Wire.write(cmd);
	Wire.endTransmission();
	Wire.requestFrom(BATTERY_ADDR, 1);	                     // request the first byte which holds the length of the character array
	pb->len = Wire.read();
	Wire.requestFrom(BATTERY_ADDR, pb->len+1);

	for(int i = 0; i <= pb->len+1; i++) {                   // it looks like i = 0 is a newline character!
		if (Wire.available()) {
			pb->value[i] = Wire.read();
		}
	}

}

void displayBlock(Param_block pb) {
	for (int i = 1; i <= pb.len+1; i++){
		tft.print(pb.value[i]);
	}
}

void showPage(int pg, battery data) {
	int x_pos = 145;
	int y_pos = 0;
	int y_inc = 34;
	int x_status = 215;
	int y_status = 44;
	
	// set display colors 
	uint16_t c_label = Color::LightBlue; 
	uint16_t c_param = Color::White;
	uint16_t c_status = Color::Orange;
	uint16_t c_backg = Color::Black;

	
	if (pg == 1) {

			tft.setTextColor(c_status, c_backg); 
			tft.setCursor(x_status, y_status);
			tft.print("Page 1/3");
		
			tft.setCursor(ORIGIN,BATT_BOARDER + BT_END_BOARDER + RECT_BUTTON_OFFSET + 4);

			tft.setTextColor(c_label, c_backg); 
			tft.print("Device Name");
			y_pos = tft.getCursorY();
			tft.setCursor(x_pos, y_pos);
			tft.print(" : ");
			tft.setTextColor(c_param, c_backg); 
			displayBlock(data.DeviceName);
			
			y_pos = tft.getCursorY() + y_inc;
			tft.setCursor(ORIGIN, y_pos);

			tft.setTextColor(c_label, c_backg); 
			tft.print("Serial Num");
			y_pos = tft.getCursorY();
			tft.setCursor(x_pos, y_pos);
			tft.print(" : ");
			tft.setTextColor(c_param, c_backg); 
			tft.print(data.SerialNumber.value);

			y_pos = tft.getCursorY() + y_inc;
			tft.setCursor(ORIGIN, y_pos);

			tft.setTextColor(c_label, c_backg); 
			tft.print("Current");
			y_pos = tft.getCursorY();
			tft.setCursor(x_pos, y_pos);
			tft.print(" : ");
			tft.setTextColor(c_param, c_backg); 
			if (abs(data.Current.value) < 500) {                    // small current values, display in mA
				if (data.Current.value == 0) {
					tft.setTextColor(c_param, c_backg); 	              // no load
					tft.print(abs(data.Current.value)); 
				} else if (data.Current.value < 0) {   		            // current drain (under load)
					tft.setTextColor(Color::Red, c_backg); 
					tft.print(abs(data.Current.value)); 
				} else {								  	                          // battery charging
					tft.setTextColor(Color::Green, c_backg); 
					tft.print(abs(data.Current.value)); 
				}
				tft.print(" mA    ");
			} else {							                                  // Large current values, display in A
				tft.print(((float)data.Current.value)/1000); 
				tft.print(" A");
			}

			y_pos = tft.getCursorY() + y_inc;
			tft.setCursor(ORIGIN, y_pos);

			tft.setTextColor(c_label, c_backg); 
			tft.print("Voltage");
			y_pos = tft.getCursorY();
			tft.setCursor(x_pos, y_pos);
			tft.print(" : ");
			tft.setTextColor(c_param, c_backg); 
			tft.print(((float)data.Voltage.value)/1000);
			tft.print(" V");

			y_pos = tft.getCursorY() + y_inc;
			tft.setCursor(ORIGIN, y_pos);

			tft.setTextColor(c_label, c_backg); 
            tft.print("Temp");
			y_pos = tft.getCursorY();
			tft.setCursor(x_pos, y_pos);
			tft.print(" : ");
			tft.setTextColor(c_param, c_backg); 
			tft.print(((float)data.Temperature.value / 10)-273.15);
			tft.print(" C");

			y_pos = tft.getCursorY() + y_inc;
			tft.setCursor(ORIGIN, y_pos);

			tft.setTextColor(c_label, c_backg); 
			tft.print("Design Cap");
			y_pos = tft.getCursorY();
			tft.setCursor(x_pos, y_pos);
			tft.print(" : ");
			tft.setTextColor(c_param, c_backg); 
			tft.print(data.DesignCapacity.value);
			tft.print(" mAh");

			y_pos = tft.getCursorY() + y_inc;
			tft.setCursor(ORIGIN, y_pos);

			tft.setTextColor(c_label, c_backg); 
            tft.print("Rel SoC");
			y_pos = tft.getCursorY();
			tft.setCursor(x_pos, y_pos);
			tft.print(" : ");
			tft.setTextColor(c_param, c_backg); 
            tft.print(data.RelativeStateOfCharge.value);
			tft.print(" %"); 

			y_pos = tft.getCursorY() + y_inc;
			tft.setCursor(ORIGIN, y_pos);

			tft.setTextColor(c_label, c_backg); 
            tft.print("Full Chg Cap");
			y_pos = tft.getCursorY();
			tft.setCursor(x_pos, y_pos);
			tft.print(" : ");
			tft.setTextColor(c_param, c_backg); 
            tft.print(data.FullChargeCapacity.value);
            tft.print(" mAh");

			y_pos = tft.getCursorY() + y_inc;
			tft.setCursor(ORIGIN, y_pos);

			tft.setTextColor(c_label, c_backg); 
            tft.print("Man Access");
			y_pos = tft.getCursorY();
			tft.setCursor(x_pos, y_pos);
			tft.print(" : ");
			tft.setTextColor(c_param, c_backg); 
            tft.print(data.ManufacturerAccess.value);
            tft.print(" ");

			y_pos = tft.getCursorY() + y_inc;
			tft.setCursor(ORIGIN, y_pos);

			tft.setTextColor(c_label, c_backg); 
			tft.print("Rem Cap Alm");
			y_pos = tft.getCursorY();
			tft.setCursor(x_pos, y_pos);
			tft.print(" : ");
			tft.setTextColor(c_param, c_backg); 
			tft.print(data.RemainingCapacityAlarm.value);
			tft.print(" mAh");

			y_pos = tft.getCursorY() + y_inc;
			tft.setCursor(ORIGIN, y_pos);

			tft.setTextColor(c_label, c_backg); 
			tft.print("Rem Time Alm");
			y_pos = tft.getCursorY();
			tft.setCursor(x_pos, y_pos);
			tft.print(" : ");
			tft.setTextColor(c_param, c_backg); 
            tft.print(data.RemainingTimeAlarm.value);
            tft.print(" mins");
      		

		} else if (pg == 2) {

			tft.setTextColor(c_status, c_backg); 
			tft.setCursor(x_status, y_status);
			tft.print("Page 2/3");

			tft.setCursor(ORIGIN,BATT_BOARDER + BT_END_BOARDER + RECT_BUTTON_OFFSET);

			tft.setTextColor(c_label, c_backg); 
            tft.print("Batt Mode");
			y_pos = tft.getCursorY();
			tft.setCursor(x_pos, y_pos);
			tft.print(" : ");
			tft.setTextColor(c_param, c_backg); 
            tft.print(data.BatteryMode.value);
            tft.print(" ");

			y_pos = tft.getCursorY() + y_inc;
			tft.setCursor(ORIGIN, y_pos);

			tft.setTextColor(c_label, c_backg); 
			tft.print("AtRate");
			y_pos = tft.getCursorY();
			tft.setCursor(x_pos, y_pos);
			tft.print(" : ");
			tft.setTextColor(c_param, c_backg); 
			tft.print(data.AtRate.value);
			tft.print(" ");

			y_pos = tft.getCursorY() + y_inc;
			tft.setCursor(ORIGIN, y_pos);

			tft.setTextColor(c_label, c_backg); 
			tft.print("AtRate TTF");
			y_pos = tft.getCursorY();
			tft.setCursor(x_pos, y_pos);
			tft.print(" : ");
			tft.setTextColor(c_param, c_backg); 
			tft.print(data.AtRateTimeToFull.value);
			tft.print(" ");
			
			y_pos = tft.getCursorY() + y_inc;
			tft.setCursor(ORIGIN, y_pos);

			tft.setTextColor(c_label, c_backg); 
			tft.print("AtRate TTE");
			y_pos = tft.getCursorY();
			tft.setCursor(x_pos, y_pos);
			tft.print(" : ");
			tft.setTextColor(c_param, c_backg); 
			tft.print(data.AtRateTimeToEmpty.value);
			tft.print(" ");

			y_pos = tft.getCursorY() + y_inc;
			tft.setCursor(ORIGIN, y_pos);

			tft.setTextColor(c_label, c_backg); 
      		tft.print("AtRate OK");
			y_pos = tft.getCursorY();
			tft.setCursor(x_pos, y_pos);
			tft.print(" : ");
			tft.setTextColor(c_param, c_backg); 
      		tft.print(data.AtRateOK.value);
      		tft.print(" ");

			y_pos = tft.getCursorY() + y_inc;
			tft.setCursor(ORIGIN, y_pos);

			tft.setTextColor(c_label, c_backg); 
			tft.print("Avg Current");
			y_pos = tft.getCursorY();
			tft.setCursor(x_pos, y_pos);
			tft.print(" : ");
			tft.setTextColor(c_param, c_backg); 
      		tft.print(data.AverageCurrent.value);
      		tft.print(" ");

			y_pos = tft.getCursorY() + y_inc;
			tft.setCursor(ORIGIN, y_pos);

			tft.setTextColor(c_label, c_backg); 
			tft.print("Max Error");
			y_pos = tft.getCursorY();
			tft.setCursor(x_pos, y_pos);
			tft.print(" : ");
			tft.setTextColor(c_param, c_backg); 
      		tft.print(data.MaxError.value);
      		tft.print(" ");
			
			y_pos = tft.getCursorY() + y_inc;
			tft.setCursor(ORIGIN, y_pos);

			tft.setTextColor(c_label, c_backg); 
			tft.print("Abs SoC");
			y_pos = tft.getCursorY();
			tft.setCursor(x_pos, y_pos);
			tft.print(" : ");
			tft.setTextColor(c_param, c_backg); 
			tft.print(data.AbsoluteStateOfCharge.value);
			tft.print(" %"); 

			y_pos = tft.getCursorY() + y_inc;
			tft.setCursor(ORIGIN, y_pos);

			tft.setTextColor(c_label, c_backg); 
			tft.print("Rem Cap");
			y_pos = tft.getCursorY();
			tft.setCursor(x_pos, y_pos);
			tft.print(" : ");
			tft.setTextColor(c_param, c_backg); 
      		tft.print(data.RemainingCapacity.value);
      		tft.print(" ");

			y_pos = tft.getCursorY() + y_inc;
			tft.setCursor(ORIGIN, y_pos);

			tft.setTextColor(c_label, c_backg); 
			tft.print("Run TTE");
			y_pos = tft.getCursorY();
			tft.setCursor(x_pos, y_pos);
			tft.print(" : ");
			tft.setTextColor(c_param, c_backg); 
      		tft.print(data.RunTimeToEmpty.value);
      		tft.print(" ");

			y_pos = tft.getCursorY() + y_inc;
			tft.setCursor(ORIGIN, y_pos);

			tft.setTextColor(c_label, c_backg); 
      		tft.print("Avg TTE");
			y_pos = tft.getCursorY();
			tft.setCursor(x_pos, y_pos);
			tft.print(" : ");
			tft.setTextColor(c_param, c_backg); 
      		tft.print(data.AverageTimeToEmpty.value);
      		tft.print(" ");


		} else {

			tft.setTextColor(c_status, c_backg); 
			tft.setCursor(x_status, y_status);
			tft.print("Page 3/3");
			
			tft.setCursor(ORIGIN,BATT_BOARDER + BT_END_BOARDER + RECT_BUTTON_OFFSET);

			tft.setTextColor(c_label, c_backg); 
			tft.print("Avg TTF");
			y_pos = tft.getCursorY();
			tft.setCursor(x_pos, y_pos);
			tft.print(" : ");
			tft.setTextColor(c_param, c_backg); 
      		tft.print(data.AverageTimeToFull.value);
      		tft.print(" ");

			y_pos = tft.getCursorY() + y_inc;
			tft.setCursor(ORIGIN, y_pos);

			tft.setTextColor(c_label, c_backg); 
      		tft.print("Chrg Current");
			y_pos = tft.getCursorY();
			tft.setCursor(x_pos, y_pos);
			tft.print(" : ");
			tft.setTextColor(c_param, c_backg); 
      		tft.print(data.ChargingCurrent.value);
      		tft.print(" ");

			y_pos = tft.getCursorY() + y_inc;
			tft.setCursor(ORIGIN, y_pos);

			tft.setTextColor(c_label, c_backg); 
      		tft.print("Chrg Voltage");
			y_pos = tft.getCursorY();
			tft.setCursor(x_pos, y_pos);
			tft.print(" : ");
			tft.setTextColor(c_param, c_backg); 
      		tft.print(data.ChargingVoltage.value);
      		tft.print(" ");

			y_pos = tft.getCursorY() + y_inc;
			tft.setCursor(ORIGIN, y_pos);

			tft.setTextColor(c_label, c_backg); 
      		tft.print("Batt Status");
			y_pos = tft.getCursorY();
			tft.setCursor(x_pos, y_pos);
			tft.print(" : ");
			tft.setTextColor(c_param, c_backg); 
      		tft.print(data.BatteryStatus.value);
      		tft.print(" ");

			y_pos = tft.getCursorY() + y_inc;
			tft.setCursor(ORIGIN, y_pos);

			tft.setTextColor(c_label, c_backg); 
      		tft.print("Cycle Count");
			y_pos = tft.getCursorY();
			tft.setCursor(x_pos, y_pos);
			tft.print(" : ");
			tft.setTextColor(c_param, c_backg); 
      		tft.print(data.CycleCount.value);
      		tft.print(" ");

			y_pos = tft.getCursorY() + y_inc;
			tft.setCursor(ORIGIN, y_pos);

			tft.setTextColor(c_label, c_backg); 
      		tft.print("Design Volt");
			y_pos = tft.getCursorY();
			tft.setCursor(x_pos, y_pos);
			tft.print(" : ");
			tft.setTextColor(c_param, c_backg); 
      		tft.print(data.DesignVoltage.value);
      		tft.print(" ");

			y_pos = tft.getCursorY() + y_inc;
			tft.setCursor(ORIGIN, y_pos);

			tft.setTextColor(c_label, c_backg); 
      		tft.print("Spec Info");
			y_pos = tft.getCursorY();
			tft.setCursor(x_pos, y_pos);
			tft.print(" : ");
			tft.setTextColor(c_param, c_backg); 
      		tft.print(data.SpecificationInfo.value);
      		tft.print(" ");

			y_pos = tft.getCursorY() + y_inc;
			tft.setCursor(ORIGIN, y_pos);

			tft.setTextColor(c_label, c_backg); 
      		tft.print("Man Date");
			y_pos = tft.getCursorY();
			tft.setCursor(x_pos, y_pos);
			tft.print(" : ");
			tft.setTextColor(c_param, c_backg); 
      		tft.print(data.ManufacturerDate.value);
      		tft.print(" ");

			y_pos = tft.getCursorY() + y_inc;
			tft.setCursor(ORIGIN, y_pos);

			tft.setTextColor(c_label, c_backg); 
      		tft.print("Man Name");
			y_pos = tft.getCursorY();
			tft.setCursor(x_pos, y_pos);
			tft.print(" : ");
			tft.setTextColor(c_param, c_backg); 
      		displayBlock(data.ManufacturerName);
      		tft.print(" ");
            
			y_pos = tft.getCursorY() + y_inc;
			tft.setCursor(ORIGIN, y_pos);

			tft.setTextColor(c_label, c_backg); 
			tft.print("Device Chem");
			y_pos = tft.getCursorY();
			tft.setCursor(x_pos, y_pos);
			tft.print(" : ");
			tft.setTextColor(c_param, c_backg); 
			displayBlock(data.DeviceChemistry);
			tft.print(" ");
			
			y_pos = tft.getCursorY() + y_inc;
			tft.setCursor(ORIGIN, y_pos);

			tft.setTextColor(c_label, c_backg); 
			tft.print("Man Data");
			y_pos = tft.getCursorY();
			tft.setCursor(x_pos, y_pos);
			tft.print(" : ");
			tft.setTextColor(c_param, c_backg); 
      //displayBlock(data.ManufacturerData);
      		tft.print("N/A");
      		
		}
}
