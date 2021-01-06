#include <Arduino.h>
#include "XeoSmartHomeDevice.h"
#include <SoftwareSerial.h>


XeoSmartHomeDevice MyDevice;

//SoftwareSerial ArduinoMega(D6, D7);


int c = 0;
void buttonPressed(){
	Serial.print("button pressed: ");
	Serial.println(c++);

	//MyDevice.sendSensorData("temperature", 5.6);

	MyDevice.sendStatusUpdate("valve_1", random8(0, 2));
	MyDevice.sendStatusUpdate("valve_2", random8(0, 2));
	MyDevice.sendStatusUpdate("valve_3", random8(0, 2));
	MyDevice.sendStatusUpdate("valve_4", random8(0, 2));
}

void openWindow1(JsonArray parametes){
	Serial.println("a");
}
/*


void openWindow2(JsonArray parametes){
	ArduinoMega.write("b");
}

void openWindow3(JsonArray parametes){
	ArduinoMega.write("c");
}

void openWindow4(JsonArray parametes){
	ArduinoMega.write("d");
}

void openWindow5(JsonArray parametes){
	ArduinoMega.write("e");
}

void openWindow6(JsonArray parametes){
	ArduinoMega.write("f");
}


void closeWindow1(JsonArray parametes){
	ArduinoMega.write("g");
}

void closeWindow2(JsonArray parametes){
	ArduinoMega.write("h");
}

void closeWindow3(JsonArray parametes){
	ArduinoMega.write("i");
}

void closeWindow4(JsonArray parametes){
	ArduinoMega.write("j");
}

void closeWindow5(JsonArray parametes){
	ArduinoMega.write("k");
}

void closeWindow6(JsonArray parametes){
	ArduinoMega.write("l");
}
*/

void onStopAll(JsonArray paremeters){
	Serial.println("Stop all triggered");

	digitalWrite(D2, !digitalRead(D2));

	MyDevice.sendStatusUpdate("valve_1", random8(0, 2));
	MyDevice.sendStatusUpdate("valve_2", random8(0, 2));
	MyDevice.sendStatusUpdate("valve_3", random8(0, 2));
	MyDevice.sendStatusUpdate("valve_4", random8(0, 2));
}


void setup() {
	Serial.begin(115200);
	
	MyDevice.setName("claudiu's device");
	MyDevice.setSerial("7575757575");
	MyDevice.setOnButtonPressHandler(buttonPressed);

	MyDevice.addActionHandler("open_window_1", openWindow1);
	MyDevice.addActionHandler("stop_all", onStopAll);
	/*MyDevice.addActionHandler("open_window_1", openWindow1);
	MyDevice.addActionHandler("open_window_2", openWindow2);
	MyDevice.addActionHandler("open_window_3", openWindow3);
	MyDevice.addActionHandler("open_window_4", openWindow4);
	MyDevice.addActionHandler("open_window_5", openWindow5);
	MyDevice.addActionHandler("open_window_6", openWindow6);

	MyDevice.addActionHandler("close_window_1", closeWindow1);
	MyDevice.addActionHandler("close_window_2", closeWindow2);
	MyDevice.addActionHandler("close_window_3", closeWindow3);
	MyDevice.addActionHandler("close_window_4", closeWindow4);
	MyDevice.addActionHandler("close_window_5", closeWindow5);
	MyDevice.addActionHandler("close_window_6", closeWindow6);*/

	MyDevice.setDebug(true);

	MyDevice.init();
	
	//ArduinoMega.begin(9600);
	Cron.create("0 * * * * *", [](){
		//Cron.getTriggeredCronId();
		//time_t now = time(nullptr);
 		Serial.println("Send sensor data");
		MyDevice.sendSensorData("temperature", random16(0, 30));
	}, false);

}

void loop() {
	MyDevice.loop();
}

/*Task ledTask(200, TASK_FOREVER, buttonPressed);
	taskScheduler.addTask(ledTask);
	ledTask.enable();*/
	
