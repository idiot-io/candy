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
#include <U8g2lib.h>
#include <Wire.h>
U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ D6, /* data=*/ D7, /* reset=*/ U8X8_PIN_NONE);   // Adafruit Feather ESP8266/32u4 Boards + FeatherWing OLED



// pysicals
#define ledStatus 2
int linecount = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("starting..");

  //wifi manger
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");
  Serial.println("connected...");

  //IRrecv
  irrecv.enableIRIn();  // Start the receiver

  //MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);


  u8g2.begin();

  char mystr[40];
  sprintf(mystr, "Millis: %ul", millis());
  u8g2.clearBuffer();          // clear the internal memory
  u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
  u8g2.drawStr(0, 10, mystr); // write something to the internal memory
  u8g2.sendBuffer();          // transfer internal memory to the display
  delay(10);

}

void loop(void) {


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
    u8g2.clearBuffer();          // clear the internal memory
    u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
    u8g2.drawStr(0, 40, charBuf); // write something to the internal memory
    u8g2.sendBuffer();          // transfer internal memory to the display

    /*
      display.setCursor(4, 43 + ((linecount % 2) * 8));
      display.print("sending: ");  display.print(charBuf);
      display.display();
      linecount++;
    */
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

