
/*
    test on your server with
    $ mosquitto_sub -h 127.0.0.1 -i testSub -t outLicks

*/

/////////////////////////////////////////////////////////////////////////////
//wifi manager     https://github.com/tzapu/WiFiManager
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

/////////////////////////////////////////////////////////////////////////////
//irRemote code https://github.com/markszabo/IRremoteESP8266/wiki#ir-receiving
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
uint16_t RECV_PIN = 14; // An IR detector/demodulator is connected to GPIO pin 14(D5 on a NodeMCU).
IRrecv irrecv(RECV_PIN);
decode_results results;

/////////////////////////////////////////////////////////////////////////////
//mqtt broker https://github.com/knolleary/pubsubclient - http://pubsubclient.knolleary.net/
#include <PubSubClient.h>
WiFiClient espClient;
PubSubClient client(espClient);
const char* mqtt_server = "idiot.io";

/////////////////////////////////////////////////
//// OLED aliexpress ssd1306
// see setup https://www.youtube.com/watch?v=RpK7-Ljnpho
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SSD1306_LCDHEIGHT 64
#define OLED_RESET LED_BUILTIN
Adafruit_SSD1306 display(OLED_RESET);

#include "imageidiot.h"


///////////////////////////////////////////////
// pysicals
#define ledStatus 2
int linecount = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("starting..");

  //OLED
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.drawBitmap(0, 0, idiotio_Logo_bits, idiotio_Logo_width, idiotio_Logo_height, 1);
  display.display();

  display.setTextSize(1);
  display.setTextColor(WHITE, BLACK);
  display.setCursor(0, 0);

  //wifi manger
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");
  Serial.println("connected...");

  //IRrecv
  irrecv.enableIRIn();  // Start the receiver

  //MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(ledStatus, OUTPUT);
  digitalWrite(ledStatus, LOW);

  delay(1500);

  display.fillRect(4, 35, 108, 29, BLACK);
  display.setCursor(4, 35);
  display.print(WiFi.localIP().toString());
  display.display();


}

void loop() {


  //mqtt loop
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  //ir recv callback
  if (irrecv.decode(&results)) {
    digitalWrite(ledStatus, HIGH);

    //mqtt
    String txt = uint64ToString(results.value, 16);
    char charBuf[50];
    txt.toCharArray(charBuf, 16);

    client.publish("outLicks", charBuf );

    Serial.print("sending: ");
    Serial.println(charBuf);

    //IR sig
    //limited to 2 row, then need to jump up 8*2 on y
    display.setCursor(4, 43 + ((linecount % 2) * 8));
    display.print("sending: ");  display.print(charBuf);
    display.display();
    linecount++;

    delay(100);

    irrecv.resume();  // Receive the next value

    digitalWrite(ledStatus, LOW);
    /*
      ++value;
      snprintf (msg, 128, "licked %lld @ %ld", results.value, millis() );

      Serial.print("Publish message: ");
      Serial.println(msg);
      client.publish("outLicks", msg);

      // print() & println() can't handle printing long longs. (uint64_t)
      serialPrintUint64(results.value, HEX);
      Serial.println("");
    */
  }
}

//MQTT functions
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void testdrawchar(void) {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  for (uint8_t i = 0; i < 168; i++) {
    if (i == '\n') continue;
    display.write(i);
    if ((i > 0) && (i % 21 == 0))
      display.println();
  }
  display.display();
  delay(1);
}
