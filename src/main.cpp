#include <Arduino.h>
#include  "XeoSmartHomeDevice.h"


XeoSmartHomeDevice MyDevice;

/*Scheduler taskScheduler;*/


int c = 0;
void buttonPressed(){
	Serial.print("button pressed: ");
	Serial.println(c++);
}


void irrigate(JsonArray& parameters){
	Serial.println("Irrigation triggered");
}

void timed_feed(const char * cron, JsonArray& parameters){
	Serial.println(cron);
}

void setup() {
	Serial.begin(115200);
	delay(500);
	WiFi.begin("Paltinis2", "Paltinis12345");
	
	MyDevice.setName("claudiu's device");
	MyDevice.setSerial("7575757575");
	MyDevice.setOnButtonPressHandler(buttonPressed);

	MyDevice.addActionHandler("irrigate", irrigate);
	MyDevice.addTimedActionHamdler("irrigate", timed_feed);

	MyDevice.setDebug(true);

	MyDevice.init();
	
	/*Task ledTask(200, TASK_FOREVER, buttonPressed);
	taskScheduler.addTask(ledTask);
	ledTask.enable();*/
	
	Cron.create("* * * * * *", [](){
		Serial.print("getFreeHeap(): ");
		Serial.println(ESP.getFreeHeap());
		Serial.print("getFreeHeap(): ");
		time_t now = time(nullptr);
 		 Serial.println(ctime(&now));
		}, false);

}

void loop() {
	MyDevice.loop();
}
