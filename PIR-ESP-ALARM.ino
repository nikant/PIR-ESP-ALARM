#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include "espWiFi2eeprom.h"


#define pirsensor 13 // pin D7
#define onoffswitch 14 // pin D5
#define ONBOARDLED 2

#define delaytime 25 // initial delay time in seconds


// replace with your channel's IFTTT MAKER key
String IftttMakerKey = "xxxxxxxxxxxxxxxxxxxxxxxxxx"; // IFTTT account
const char *hostIftttMaker = "maker.ifttt.com";
String IftttMakerString = "";

int pirValue = 0;
int switchValue = 1;

boolean switchmessage = false;

// Update these with values suitable for your network.
IPAddress nkip(192, 168, 1, 2); //Node static IP
IPAddress nkgateway(192, 168, 1, 1);
IPAddress nksubnet(255, 255, 255, 0);
IPAddress dns1(8, 8, 8, 8);
IPAddress dns2(8, 8, 4, 4);


void setup() {
  WiFi.persistent(false);
  WiFi.setOutputPower(20.5);
  // uncomment the following if you set a static IP in the begining
  //WiFi.config(nkip, nkgateway, nksubnet, dns1, dns2);

  pinMode(ONBOARDLED, OUTPUT);
  pinMode(pirsensor, INPUT); // declare sensor as input
  pinMode(onoffswitch, INPUT_PULLUP); // declare sensor as input
  digitalWrite(ONBOARDLED, HIGH); // switch off the onboard led

  Serial.begin(57600);
  Serial.println("");

  // initial delay
  delay(delaytime * 1000);


  espNKWiFiconnect();
  //Serial.print("IP address: ");
  //Serial.println(WiFi.localIP());

  SendIftttMaker("piralarmtrigger", IftttMakerKey, "PIRALRM__POWERON");

  stopWiFiAndSleep();


}

void loop() {
  getPirValue();
}


void getPirValue(void)
{
  switchValue = digitalRead(onoffswitch);
  if (switchValue == 0) {
    if (switchmessage == false) {
      startWiFiAndWake();
      SendIftttMaker("piralarmtrigger", IftttMakerKey, "PIRALRM__ACTIVE");
      switchmessage = true;
      stopWiFiAndSleep();
      delay(5000);
    }
    pirValue = digitalRead(pirsensor);
    if (pirValue == 1) {
      startWiFiAndWake();
      SendIftttMaker("piralarmtrigger", IftttMakerKey, "PIRALRM__MOTION__!!!!");

      pirValue = 0;
      stopWiFiAndSleep();
      // delay for PIR sensor (reset from HIGH to LOW, no need for too many alarms)
      // send2web has a 4 second delay + a delay of reconnecting to WiFi, if WiFi connects slow lower the delay value here
      delay(15000); 
    }
  } else {
    switchmessage = false;
  }

}

void stopWiFiAndSleep() {
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  delay(1);
}

void startWiFiAndWake() {
  WiFi.forceSleepWake();
  delay( 1 );
  espNKWiFiconnect();
}

void send2web(String thehost, String urlstring) {
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(thehost.c_str(), httpPort)) {
    Serial.println(F("connection failed"));
    return;
  }

  // We now create a URI for the request
  //String url = urlstring;

  // This will send the request to the server
  client.print(String("GET ") + urlstring + " HTTP/1.1\r\n" +
               "Host: " + thehost + "\r\n" +
               "Connection: close\r\n\r\n");
  delay(4000);
  client.flush();
  client.stop();
  delay(100);
}

void SendIftttMaker(String inIftttEvent, String inIftttMakerKey, String instatus) {
  IftttMakerString = "/trigger/" + inIftttEvent + "/with/key/" + inIftttMakerKey + "?value1=" + instatus;
  send2web(hostIftttMaker, IftttMakerString);
}

