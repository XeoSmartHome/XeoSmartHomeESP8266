#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncDNSServer.h>
#include <ESPAsyncWebServer.h>
#include <AsyncWebSocket.h>
#include <AsyncMqttClient.h>
#include <FastLED.h>
#include <CronAlarms.h> 
#include <FS.h>
#include <ArduinoJson.h>
#define _TASK_STD_FUNCTION 
#include <TaskScheduler.h>
#include <CronAlarms.h>

#define XEOSMARTHOME_SERVER "xeosmarthome.com"
#define ACTION_NAME_MAX_LENGTH 32
#define BUTTON_SHORT_PRESS_MIN 50
#define BUTTON_SHORT_PRESS_MAX 500
#define BUTTON_LONG_PRESS 5000

#define SUCCESS 1
#define FAIL 0
#define TASK_FOREVER -1

#define NTP_TIMEOUT 1500

#define DNS_PORT 53


namespace XeoSmartHomeInternals {
	// callbacks
	typedef std::function<void()> OnButtonPressCallback;
	typedef std::function<void(JsonArray& parameters)> OnActionCallback;
	typedef std::function<void(const char * cron, JsonArray& parameters)> OnTimedActionCallback;
	
	typedef struct Action {
		char name[ACTION_NAME_MAX_LENGTH];
		OnActionCallback callback;
	};

	typedef struct TimedAction {
		char name[ACTION_NAME_MAX_LENGTH];
		OnTimedActionCallback callback;
	};
	
	struct Settings {
		bool dhcp = true;
		IPAddress local_ip;
		IPAddress gateway;
		IPAddress subnet_mask;
		char device_name[WL_SSID_MAX_LENGTH] = "XeoSmartHome Device";
	};

	// constants
	const int WEB_SERVER_PORT = 80;
	const char * WEBSOCKET_SERVER_URL = "/ws";
	const byte LED_PIN = D8;

	// NTP
	const char * ntpServer = "pool.ntp.org";
	int8_t timeZone = 2;

};


namespace XeoSmartHomeColorCodes {
	const CRGB SETTINGS_COLORS[] = {CRGB::Blue, CRGB::Black};
	const uint16 SETTINGS_INTERVAL = 500;

	const CRGB WIFI_NOT_CONNECTED[] = {CRGB::Red, CRGB::Black, CRGB::Red, CRGB::Black, CRGB::Red, CRGB::Black, CRGB::Black, CRGB::Black};
	const uint16 WITI_NOT_CONNECTED_INTERVAL = 250;
};


IPAddress stringToIpAdress(const char *string) {
	unsigned short a, b, c, d;
	sscanf(string, "%hu.%hu.%hu.%hu", &a, &b, &c, &d);
	return IPAddress(a, b, c, d);
}


IPAddress stringToIpAdress(String string) {
	return stringToIpAdress(string.c_str());
}


class XeoSmartHomeDevice {
	public:
		/*
		Crete a XeoSmartHome device
		*/	
		XeoSmartHomeDevice();
		~XeoSmartHomeDevice();

		/*
		* Set device name
		* @param name: device name
		*/
		void setName(const char * name);

		/*
		* Set device serial code
		* @param serial: device serial code
		*/
		void setSerial(const char * serial); 

		/*
		* Enable/disable debug output
		* @param debug: if true enable debug outpuut, else disable debug output
		*/
		void setDebug(bool debug); 
		
		/*
		* Initialize device, must be called in arduino setup()
		*/
		void init();

		/*
		* loop must be called in arduino loop()
		*/
		void loop();

		/* 
		* Set enent handler for button short press
		* @param callback: function that will be executed when button is pressed
		*/
		void setOnButtonPressHandler(XeoSmartHomeInternals::OnButtonPressCallback callback);

		/*
		* Set an action callback for an action name
		* Actions are functions that will be executed imediately after them are received
		* @param action_name: action uri that will be received from the cloud
		* @param callback: callback function that is paired with action_name
		*/
		void addActionHandler(const char * action_name, XeoSmartHomeInternals::OnActionCallback callback);

		/*
		* Set a timed action callback
		* Timed actions are function that will be executed the time specified in their cron
		* @param action_name: action uri that will be received from the cloud
		* @param callback: callback function that is paired with action_name
		*/
		void addTimedActionHamdler(const char * action_name, XeoSmartHomeInternals::OnActionCallback callback);

		/*
		* Send sensor value to cloud
		* @param sensor: sensor uri
		* @param value: sensor value
		*/
		bool sendSensorData(const char * sensor, float value);

		/* 
		* Send status update to server
		* @param status: status uri
		* @param 
		*/
		bool sendStatusUpdate(const char * status, int value);

	private:
		char _name[WL_SSID_MAX_LENGTH];  // device name
		char _serial[64]; // device serial code
		bool _debug = false; // debug output enabled

		std::vector<XeoSmartHomeInternals::Action> _ActionsVector; // list of device action callbacks
		std::vector<XeoSmartHomeInternals::Action> _TimedActionsVector; // list of device timed action callback

		/*
		* Called when device receive an action request from cloud
		* @param messge: message from server, json
		*/
		void _onAction(const char * message, size_t len);

		/*
		* Called when device receive a schedule update request from server
		* @param messge: message from server, json
		*/
		void _onSceduleUpdate(const char * message);

		// TASK SCHEDULER
		Scheduler _taskScheduler;

		// LED
		CRGB _leds[1];
		Task _ledTask;

		/*
		* Initialize LED
		*/
		void _initLed();

		/*
		* Set LED to given color
		* @param color: color to show
		*/
		void _setLedColor(CRGB color);

		// COLOR CODES
		uint8 _colors_vector_index = 0;
		uint8 _colors_vector_len;
		CRGB _colors_vector[20];
		void _setColorSignal(const CRGB * color_vector, uint16 size, uint16 miliseconds);

		// BUTTON
		std::function<void()> _onButtonPress;
		unsigned int _button_pin = D4;
		unsigned long _button_press_time = 0;
		bool _button_last_state = false;
		bool _long_detected = false;

		/*
		* @return true if button is pressed, else return false
		*/
		bool _buttonIsPressed();

		/*
		* Initialize button
		*/
		void _initButton();

		/*
		* Check for button short and long press
		*/
		void _checkForButtonStateChanges();

		/*
		* Called when button long pressed is detected
		*/
		void _onButtonLongPress();

		// SETTINGS
		XeoSmartHomeInternals :: Settings _settings;

		/*
		* Load device settings from configuration file
		*/
		void _loadSettings();

		/*
		* Save device settings in configuration file
		*/
		void _saveSettings();

		// WIFI
		WiFiEventHandler _WiFiEventStationModeGotIP;
		WiFiEventHandler _WiFiEventStationModeDisconnected;
		Task _wifiTimer; // WiFi disconnect timer

		/*
		* Initialize WiFi
		*/
		void _initWiFi();

		/*
		* Callback for WiFi connected event
		* @param event:
		*/
		void _onWifiConnected(const WiFiEventStationModeGotIP& event);

		/*
		* Callback for WiFi disconnected event
		* @param event:
		*/
		void _onWifiDisconnected(const WiFiEventStationModeDisconnected& even);

		// MQTT
		AsyncMqttClient * _mqttClient; // pointer to MQTT client
		Task _mqttPingTimer; // MQTT ping timer

		void _initMqttClient();
		void _startMqttClient();
		void _stopMqttClient();

		/*
		* MQTT connected callback
		* @param sessionPresent
		*/
		void _onMqttConnected(bool sessionPresent);

		/*
		* MQTT message callback
		* @param topic: mqtt topic
		* @param payload: message to be send
		* @params properties: AsyncMqttClientMessageProperties
		* @params len: message length
		* @params index: message block index
		* @param total: number of message blocks
		*/
		void _onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);

		// CONFIG-MODE
		bool _config_mode = false; // true if config mode in enabled, false if config mode in disabled

		/*
		* Called when config mode is enabled
		*/
		void _startConfigMode();

		/*
		* Called when config mode is disabled
		*/
		void _stopConfigMode();

		// DNS-SERVER
		AsyncDNSServer * _dnsServer;

		void _initDnsServer();
		void _startDnsServer();
		void _stopDnsServer();

		// WEB-SERVER
		AsyncWebServer * _webServer;
		void _initWebServer();
		void _startWebServer();
		void _stopWebServer();

		// WEB-SOCKET-SERVER
		AsyncWebSocket * _webSocketServer;
		void _initWebSocketServer();
		
		/*
		* Called when a web socket event occurs
		* @param server: poiter to server object
		* @param client: pointer to client object
		* @param type: event type
		* @param arg: extra arguments
		* @param data: pointer to message
		* @param len: message length
		*/
		void _onWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len);
		
		/*
		* Called when device receive a message from websokets
		* @param server: poiter to server object
		* @param message: message received
		*/
		void _onWebSocketMessage(AsyncWebSocket* server, AsyncWebSocketClient* client, String message);
		
		/*
		* Start async wifi scan, result will be send to send using websockets to config mode client (user phone, laptop, pc, ...)
		*/
		void _asyncWifiScan();

		// NTP-CLIENT
		void _initNtpClient();
};


// PUBLIC:

XeoSmartHomeDevice :: XeoSmartHomeDevice() {
	this->_mqttClient = new AsyncMqttClient();
	this->_dnsServer = new AsyncDNSServer();
	this->_webServer = new AsyncWebServer(XeoSmartHomeInternals::WEB_SERVER_PORT);
	this->_webSocketServer = new AsyncWebSocket(XeoSmartHomeInternals::WEBSOCKET_SERVER_URL);
}


XeoSmartHomeDevice :: ~XeoSmartHomeDevice() {
	delete(this->_mqttClient);
	delete(this->_dnsServer);
	delete(this->_webServer);
	delete(this->_webSocketServer);

}


void XeoSmartHomeDevice :: setName(const char * name) {
	strncpy(this->_name, name, sizeof(this->_name));
}


void XeoSmartHomeDevice :: setSerial(const char * serial) {
	strncpy(this->_serial, serial, sizeof(this->_serial));
}


void XeoSmartHomeDevice :: setDebug(bool debug){
	this->_debug = debug;
}


void XeoSmartHomeDevice :: init() {
	this->_initButton();
	this->_initLed();
	SPIFFS.begin();
	delay(500); // wait for file system to initialize
	this->_loadSettings();
	this->_initWiFi();
	this->_initMqttClient();
	this->_initDnsServer();
	this->_initWebServer();
	this->_initWebSocketServer();
	this->_initNtpClient();
}


void XeoSmartHomeDevice :: loop() {
	this->_checkForButtonStateChanges();
	this->_taskScheduler.execute();
	//this->_ntpClient->update();
	Cron.delay();
}


void XeoSmartHomeDevice :: setOnButtonPressHandler(XeoSmartHomeInternals::OnButtonPressCallback callback) {
	this->_onButtonPress = callback;
}


void XeoSmartHomeDevice :: addActionHandler(const char * action_name, XeoSmartHomeInternals::OnActionCallback callback) {
	XeoSmartHomeInternals::Action action;
	strncpy(action.name, action_name, ACTION_NAME_MAX_LENGTH);
	action.callback = callback;
	this->_ActionsVector.push_back(action);
}

void XeoSmartHomeDevice :: addTimedActionHamdler(const char * action_name, XeoSmartHomeInternals::OnActionCallback callback) {
	XeoSmartHomeInternals::Action timed_action;
	strncpy(timed_action.name, action_name, ACTION_NAME_MAX_LENGTH);
	timed_action.callback = callback;
	this->_TimedActionsVector.push_back(timed_action);
}


bool XeoSmartHomeDevice :: sendSensorData(const char * sensor, float value){
	if(not this->_mqttClient->connected())
		return false;
	
	String topic = "device/" + String(this->_serial) + "/sensor/" + String(sensor);

	this->_mqttClient->publish(topic.c_str(), 2, false, String(value).c_str());

	return true;
}


bool XeoSmartHomeDevice :: sendStatusUpdate(const char * status, int value){
	if(not this->_mqttClient->connected())
		return false;
	
	String topic = "device/" + String(this->_serial) + "/status/" + String(status);

	this->_mqttClient->publish(topic.c_str(), 2, false, String(value).c_str());

	return true;
}
// PRIVATE:

void _decodeJwtMessage(){
	// TODO: implement JWT decode
}


void XeoSmartHomeDevice :: _onAction(const char * message, size_t len){
	if(this->_debug)
		Serial.println("OnAction()");

	DynamicJsonDocument doc(4096);
	deserializeJson(doc, message, len);

	const char * action_name = doc["name"];
	JsonArray action_parameters = doc["parameters"];

	for (XeoSmartHomeInternals::Action action : this->_ActionsVector){
		if(strcmp(action.name, action_name) == 0){
			action.callback(action_parameters);
		}
	}
}


void XeoSmartHomeDevice :: _onSceduleUpdate(const char * message){
	if(this->_debug)
		Serial.println("OnScheduleUpdate()");

	DynamicJsonDocument doc(4096);
	deserializeJson(doc, message);

	const char * action_name = doc["name"];
	JsonArray action_parameters = doc["parameters"];

	for(XeoSmartHomeInternals::Action timed_action : this->_TimedActionsVector){
		if(strcmp(timed_action.name, action_name) == 0){
			Serial.print(action_name);
			Serial.println(" - timed action");
			//timed_action.callback(action_parameters);
		}
	}
}

//<BUTTON>

void XeoSmartHomeDevice :: _initButton(){
	pinMode(this->_button_pin, INPUT_PULLUP);
}


bool XeoSmartHomeDevice :: _buttonIsPressed() {
	return not digitalRead(this->_button_pin);
}


void XeoSmartHomeDevice :: _checkForButtonStateChanges(){
	bool current_state = this->_buttonIsPressed();
	if(current_state){ // Button is pressed
		if(this->_button_last_state){
			if(millis() - this->_button_press_time > BUTTON_LONG_PRESS and not this->_long_detected){
				this->_long_detected = true;
				this->_onButtonLongPress();
			}
		} else {
			this->_button_press_time = millis();
		}
	} else
	if(this->_button_last_state){ // Button is not pressed
		this->_long_detected = false;
		unsigned long pressed_time = millis() - this->_button_press_time;
		if(BUTTON_SHORT_PRESS_MIN < pressed_time and pressed_time < BUTTON_SHORT_PRESS_MAX){
			if(not this->_config_mode)
				this->_onButtonPress();
		}
	}
	_button_last_state = current_state;
}


void XeoSmartHomeDevice :: _onButtonLongPress(){
	if(this->_debug)
		Serial.println("Long button pressed detected");

	this->_config_mode = not this->_config_mode;

	if(this->_config_mode){
		this->_startConfigMode();
	} else {
		this->_stopConfigMode();
	}
	
}

//</BUTTON>
//<LED>

void XeoSmartHomeDevice :: _initLed(){
	FastLED.addLeds<NEOPIXEL, D8>(this->_leds, 1);
	
	this->_taskScheduler.addTask(_ledTask);
	this->_ledTask.setOnDisable([this](){
		this->_setLedColor(CRGB::Black);
	});
}


void XeoSmartHomeDevice :: _setLedColor(CRGB color){
	this->_leds[0] = color;
	FastLED.show();
}


void XeoSmartHomeDevice :: _setColorSignal(const CRGB * color_vector, uint16 size, uint16 miliseconds){
	this->_colors_vector_index = 0;
	this->_colors_vector_len = size / sizeof(CRGB);
	this->_ledTask.setInterval(miliseconds);
	this->_ledTask.setIterations(TASK_FOREVER);
	
	for(uint8 i = 0; i < this->_colors_vector_len; i++){
		this->_colors_vector[i] = color_vector[i];
	}
	
	this->_ledTask.setCallback([this](){
		this->_setLedColor(this->_colors_vector[this->_colors_vector_index++]);
		this->_colors_vector_index = this->_colors_vector_index % this->_colors_vector_len;
	});

	this->_ledTask.enable();
}

//</LED>
//<SETTINGS>

void XeoSmartHomeDevice :: _loadSettings() {
	if ( ! SPIFFS.exists("settings.txt")) {
		this->_saveSettings();
	} 

	File settings_file = SPIFFS.open("/settings.txt", "r");
	settings_file.readBytesUntil('\n', this->_name, sizeof(this->_name));

	this->_settings.dhcp = settings_file.readStringUntil('\n').toInt();
	this->_settings.local_ip = stringToIpAdress(settings_file.readStringUntil('\n'));
	this->_settings.gateway = stringToIpAdress(settings_file.readStringUntil('\n'));
	this->_settings.subnet_mask = stringToIpAdress(settings_file.readStringUntil('\n'));
}


void XeoSmartHomeDevice :: _saveSettings() {
	File settings_file = SPIFFS.open("/settings.txt", "w");
	settings_file.println(this->_name);
	settings_file.println(this->_settings.dhcp ? "1" : "0");
	settings_file.println(this->_settings.local_ip);
	settings_file.println(this->_settings.gateway);
	settings_file.println(this->_settings.subnet_mask);
	settings_file.close();
}

//</SETTINGS>
// <WIFI>

void XeoSmartHomeDevice :: _initWiFi() {
	//WiFi.mode(WIFI_STA);
	WiFi.hostname(this->_name);
	WiFi.softAP(this->_name);
	WiFi.softAPConfig(IPAddress(8,8,8,8), IPAddress(8,8,8,8), IPAddress(255, 255, 255, 0));
	delay(500);
	//WiFi.softAPConfig(accesPointIp, accesPointIp, NET_MASK);

	WiFi.mode(WIFI_STA);

	// TODO: enable static ip configuration
	/*if (!_settings.dhcp) {
		WiFi.config(_settings.local_ip, _settings.gateway, _settings.subnet_mask, _settings.gateway);
	}*/

	this->_taskScheduler.addTask(this->_wifiTimer);

	this->_WiFiEventStationModeGotIP = WiFi.onStationModeGotIP([this](const WiFiEventStationModeGotIP& event){
		this->_onWifiConnected(event);
	});

	this->_WiFiEventStationModeDisconnected = WiFi.onStationModeDisconnected([this](const WiFiEventStationModeDisconnected& event){
		this->_onWifiDisconnected(event);
	});

	WiFi.begin();
}


void XeoSmartHomeDevice :: _onWifiConnected(const WiFiEventStationModeGotIP& event) {
	if(this->_debug){
		Serial.println("WiFi connected");
		Serial.print("IP: ");
		Serial.println(event.ip);
	}
	this->_startMqttClient();
	this->_wifiTimer.disable();
	if(not this->_config_mode){
		this->_ledTask.disable();
	}
}


void XeoSmartHomeDevice ::_onWifiDisconnected(const WiFiEventStationModeDisconnected& event){
	if(this->_debug){
		Serial.println("WiFi disconnected");
		Serial.println("Reason: ");
		Serial.println(event.reason);
	}

	this->_stopMqttClient();

	this->_wifiTimer.setInterval(20 * 1000);
	this->_wifiTimer.setIterations(TASK_FOREVER);
	this->_wifiTimer.setCallback([this](){
		if(not WiFi.isConnected() and not this->_config_mode)
			this->_setColorSignal(XeoSmartHomeColorCodes::WIFI_NOT_CONNECTED, sizeof(XeoSmartHomeColorCodes::WIFI_NOT_CONNECTED), XeoSmartHomeColorCodes::WITI_NOT_CONNECTED_INTERVAL);
	});
	this->_wifiTimer.enable();
}

// </WIFI>
// <MQTT>

void XeoSmartHomeDevice :: _initMqttClient() {
	this->_mqttClient->setServer(XEOSMARTHOME_SERVER, 1883);
	this->_mqttClient->setClientId(this->_serial);
	this->_mqttClient->onMessage([this](char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
		this->_onMqttMessage(topic, payload, properties, len, index, total);
	});
	this->_mqttClient->onConnect([this](bool sessionPresent){
		this->_onMqttConnected(sessionPresent);
	});

	this->_taskScheduler.addTask(this->_mqttPingTimer);
	this->_mqttPingTimer.setInterval(30 * 1000);
	this->_mqttPingTimer.setIterations(TASK_FOREVER);
	this->_mqttPingTimer.setCallback([this](){
		if(this->_debug)
			Serial.println("MQTT sending ping");
		String topic = "device/" + String(this->_serial) + "/ping";
		this->_mqttClient->publish(topic.c_str(), 2, false, "ping");
		time_t now = time(nullptr);
		Serial.print(ctime(&now));
	});
	this->_mqttPingTimer.enable();
}


void XeoSmartHomeDevice :: _startMqttClient(){
	this->_mqttClient->connect();
}


void XeoSmartHomeDevice :: _stopMqttClient(){
	this->_mqttClient->disconnect();
}


void XeoSmartHomeDevice :: _onMqttConnected(bool sessionPresent){
	if(this->_debug)
		Serial.println("MQTT connected");

	this->_mqttClient->subscribe(("device/" + String(this->_serial) + "/action").c_str(), 2);
	this->_mqttClient->subscribe(("device/" + String(this->_serial) + "/schedule_update").c_str(), 2);
}


void XeoSmartHomeDevice :: _onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {

	if(this->_debug){
		Serial.println("MQTT message received");
		Serial.print("topic: ");
		Serial.println(topic);
		Serial.print("payload: ");
		Serial.write(payload, len);
		Serial.println();
	}

	String _topic = String(topic);

	if(_topic.endsWith("action")){		
		this->_onAction(payload, len);
	} else 
	if (_topic.endsWith("schedule")){
		//this->_onSceduleUpdate(payload);
	}

}

// </MQTT>
// <CONFIG-MODE>

void XeoSmartHomeDevice :: _startConfigMode() {
	if(this->_debug)
		Serial.println("Config mode started");

	this->_setColorSignal(XeoSmartHomeColorCodes::SETTINGS_COLORS, sizeof(XeoSmartHomeColorCodes::SETTINGS_COLORS), XeoSmartHomeColorCodes::SETTINGS_INTERVAL);

	WiFi.mode(WIFI_AP_STA);
	this->_startDnsServer();
	this->_startWebServer();
}


void XeoSmartHomeDevice :: _stopConfigMode() {
	if(this->_debug)
		Serial.println("Config mode stopped");

	WiFi.mode(WIFI_STA);
	this->_stopDnsServer();
	this->_stopWebServer();

	this->_ledTask.disable();
	this->_setLedColor(CRGB::Black);
}

// </CONFIG-MODE>
// <DNS-SERVER>

void XeoSmartHomeDevice :: _initDnsServer() {
	this->_dnsServer->setErrorReplyCode(AsyncDNSReplyCode::NoError);
	this->_dnsServer->setTTL(0);
}


void XeoSmartHomeDevice :: _startDnsServer() {
	this->_dnsServer->start(DNS_PORT, "*", IPAddress(8,8,8,8));
}


void XeoSmartHomeDevice :: _stopDnsServer() {
	this->_dnsServer->stop();
}

// </DNS-SERVER>
// <WEB-SERVER>

void XeoSmartHomeDevice :: _initWebServer() {
	this->_webServer->onNotFound([this](AsyncWebServerRequest* request) {
		request->redirect("/");
	});
	this->_webServer->serveStatic("/", SPIFFS, "/").setDefaultFile("index.html").setCacheControl("max-age=600");
}


void XeoSmartHomeDevice :: _startWebServer() {
	this->_webServer->begin();
}


void XeoSmartHomeDevice :: _stopWebServer() {
	this->_webServer->end();
	SPIFFS.end();
}

// </WEB-SERVER>
// <WEB-SOCKET-SERVER>

void XeoSmartHomeDevice :: _initWebSocketServer() {
	this->_webServer->addHandler(this->_webSocketServer);
	this->_webSocketServer->onEvent([this](AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len){
		this->_onWebSocketEvent(server, client, type, arg, data, len);
	});
}


void XeoSmartHomeDevice :: _onWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len){
	switch (type) {
	case WS_EVT_CONNECT:
		if(this->_debug)
			Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
		client->ping();
		break;

	case WS_EVT_DISCONNECT:
		if(this->_debug)
			Serial.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
		break;

	case WS_EVT_PONG:
		if(this->_debug)
			Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char*)data : "");
		break;

	case WS_EVT_ERROR:
		if(this->_debug)
			Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
		break;

	case WS_EVT_DATA:
		AwsFrameInfo* info = (AwsFrameInfo*)arg;
		String message = "";
		//the whole message is in a single frame and we got all of it's data
		if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
			for (size_t i = 0; i < info->len; i++)
				message += (char)data[i];
				this->_onWebSocketMessage(server, client, message);
		}
		break;
	}
}


void XeoSmartHomeDevice :: _onWebSocketMessage(AsyncWebSocket* server, AsyncWebSocketClient* client, String message){
	DynamicJsonDocument request_doc(2048);
	DynamicJsonDocument response_doc(1024);
	deserializeJson(request_doc, message);

	String event = request_doc["event"];

	response_doc["event"] = event;
	String response;

	if (event == "scan_wifi_networks") {
		this->_asyncWifiScan();
		response_doc["status"] = 2; // searching status
	}
	else
	if (event == "set_wifi_credentials") {
		const char *ssid = request_doc["ssid"];
		const char *password = request_doc["password"];

		if (ssid != NULL && password != NULL) {
			this->_settings.dhcp = true;
			this->_saveSettings();
			response_doc["status"] = SUCCESS;
			WiFi.begin(ssid, password);
		} else {
			response_doc["status"] = FAIL;
		}
	}
	else
	if (event == "set_device_name") {
		const char* name = request_doc["name"];

		if (name != NULL) {
			strncpy(this->_name, name, sizeof(this->_name));
			_saveSettings();
			response_doc["status"] = SUCCESS;
		}else
			response_doc["status"] = FAIL;
	}
	else
	if (event == "set_wifi_advanced") {
		String s_local_ip = request_doc["local_ip"];
		String s_gate_way = request_doc["gateway"];
		String s_subnet = request_doc["subnet"];

		if (s_local_ip != NULL && s_gate_way != NULL && s_subnet != NULL) {

			IPAddress local_ip = stringToIpAdress(s_local_ip);
			IPAddress gateway = stringToIpAdress(s_gate_way);
			IPAddress subnet = stringToIpAdress(s_subnet);

			this->_settings.dhcp = false;
			this->_settings.local_ip = local_ip;
			this->_settings.gateway = gateway;
			this->_settings.subnet_mask = subnet;
			_saveSettings();
			WiFi.config(local_ip, gateway, subnet);

			response_doc["status"] = SUCCESS;
		} else {
			response_doc["status"] = FAIL;
		}
	}
	else
	if (event == "reboot_device") {
		ESP.restart();
		// TODO: this sometimes causes a wdt reset and esp8266 crashs.
	}

	serializeJson(response_doc, response);
	client->text(response);
};


void XeoSmartHomeDevice :: _asyncWifiScan() {
	/*
	This function scan for available wifi networks and send response using websockets to the client
	*/
	if (WiFi.scanComplete() != WIFI_SCAN_RUNNING) {
		WiFi.scanNetworksAsync([this](int networks) {
			DynamicJsonDocument doc(2024);
			doc["event"] = "scan_wifi_networks";
			doc["status"] = SUCCESS;

			String ssid;
			uint8_t encryptionType;
			int32_t rssi;
			uint8_t* bssid;
			int32_t channel;
			bool isHidden;

			JsonArray ssidArray = doc.createNestedArray("ssid");
			JsonArray encryptionTypeArray = doc.createNestedArray("encryption_type");
			JsonArray rssiArray = doc.createNestedArray("rssi");
			JsonArray bssidArray = doc.createNestedArray("bssid");
			JsonArray chanelArray = doc.createNestedArray("chanel");
			JsonArray isHidenArray = doc.createNestedArray("is_hiden");

			for (int this_network = 0; this_network < networks; this_network++) {
				WiFi.getNetworkInfo(this_network, ssid, encryptionType, rssi, bssid, channel, isHidden);
				ssidArray.add(ssid);
				/*
				Function returns a number that encodes encryption type as follows:
				* 2 : ENC_TYPE_TKIP - WPA / PSK 
				* 4 : ENC_TYPE_CCMP - WPA2 / PSK 
				* 5 : ENC_TYPE_WEP - WEP 
				* 7 : ENC_TYPE_NONE - open network 
				* 8 : ENC_TYPE_AUTO - WPA / WPA2 / PSK 
				*/
				encryptionTypeArray.add(encryptionType);
				rssiArray.add(rssi);
				//bssidArray.add(bssid); // TODO: transform mac adress from pointer to string
				chanelArray.add(channel);
				isHidenArray.add(isHidden);
			}

			String response;
			//serializeJson(doc, response);
			serializeJson(doc, response);
			//Serial.println(response);

			this->_webSocketServer->textAll(response);
		}, true);
	}
}

// </WEB-SOCKET-SERVER>
// <NTP-CLIENT>

void XeoSmartHomeDevice :: _initNtpClient(){
	configTime(2*60*60, 0, "pool.ntp.org");
}

// </NTP-CLIENT>
