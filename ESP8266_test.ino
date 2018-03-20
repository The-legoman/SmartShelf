
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <ESP8266HTTPClient.h>
#include <ArduinoHttpClient.h>


/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "52.70.203.194"//"io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "the_legoman"
#define AIO_KEY         "73195e98dcf848ba921ce35822f2f17b"

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish photocell = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/photocell");
Adafruit_MQTT_Publish onoff = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/onoff");


// Setup a feed called 'onoff' for subscribing to changes to the button
Adafruit_MQTT_Subscribe test1 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/test1", MQTT_QOS_1);
Adafruit_MQTT_Subscribe test2 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/test2", MQTT_QOS_1);
Adafruit_MQTT_Subscribe toggle1 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/Toggle1", MQTT_QOS_1);



//MQTT CONNECT
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 10 seconds...");
       mqtt.disconnect();
       delay(10000);  // wait 10 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}


/*************************** Sketch Code ************************************/

void digitalCallback(char *data, uint16_t len) {
  Serial.print("Hey we're in a onoff callback, the button value is: ");
  Serial.println(data);

     String message = String(data);
      message.trim();
      if (message == "on") {digitalWrite(12, HIGH);}
      if (message == "off") {digitalWrite(12, LOW);} 
}

void analogicaCallback(char *data, uint16_t len) {
  Serial.print("Hey we're in a onoff callback, the button value is: ");
  Serial.println(data);

     String message = String(data);
     message.trim();
     analogWrite(13, message.toInt());
}

void toggleCallback(char *data, uint16_t len) {
  Serial.print("Hey we're in a onoff callback, the button value is: ");
  Serial.println(data);

     String message = String(data);
      message.trim();
      if (message == "on") {digitalWrite(5, HIGH);}
      if (message == "off") {digitalWrite(5, LOW);} 
}


//-------------------VARIABLES GLOBALES--------------------------
int contconexion = 0;

const char *ssid = "JaramilloR";
const char *password = "d@sajuto2018";

unsigned long previousMillis = 0;

char charPulsador [15];
String strPulsador;
String strPulsadorUltimo;

char charPulsador1 [16];
String strPulsador1;
String strPulsadorUltimo1;

char charPulsador2 [17];
String strPulsador2;
String strPulsadorUltimo2;

char charPulsador3 [18];
String strPulsador3;
String strPulsadorUltimo3;
//-------------------------------------------------------------------------

void setup() {

  //prepara GPI13 y 12 como salidas 
  pinMode(13, OUTPUT); // D7 salida analógica
  analogWrite(13, 0); // analogWrite(pin, value);
  pinMode(12, OUTPUT); // D6 salida digital
  digitalWrite(12, LOW);
  pinMode(5, OUTPUT); // D1 salida digital
  digitalWrite(5, LOW);

  // Entradas
  pinMode(14, INPUT); // D5
  pinMode(4, INPUT); // D2
  pinMode(0, INPUT); // D3
  pinMode(2, INPUT); // D4 

  // Inicia Serial
  Serial.begin(115200);
  Serial.println("");

  // Conexión WIFI
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED and contconexion <50) { //Cuenta hasta 50 si no se puede conectar lo cancela
    ++contconexion;
    delay(500);
    Serial.print(".");
  }
  if (contconexion <50) {
      //para usar con ip fija
      IPAddress ip(192,168,1,156); 
      IPAddress gateway(192,168,1,1); 
      IPAddress subnet(255,255,255,0); 
      WiFi.config(ip, gateway, subnet); 
      
      Serial.println("");
      Serial.println("WiFi CONNECTED");
      Serial.println(WiFi.localIP());
  }
  else { 
      Serial.println("");
      Serial.println("Error de conexion");
  }

  test1.setCallback(digitalCallback);
  test2.setCallback(analogicaCallback);
  toggle1.setCallback(toggleCallback);
  mqtt.subscribe(&test1);
  mqtt.subscribe(&test2);
  mqtt.subscribe(&toggle1);

  
  
}

//--------------------------LOOP--------------------------------
void loop() {

  MQTT_connect();

  unsigned long currentMillis = millis();
    
  if (currentMillis - previousMillis >= 2000) { //envia la temperatura cada 2 segundos
    previousMillis = currentMillis;
    int analog = analogRead(17);
    float temp = analog*0.322265625;
    Serial.print(F("\nSending temperatura val "));
    Serial.print(temp);
    Serial.print("...");
    if (!photocell.publish(temp)) {
      Serial.println(F("Failed"));
    } else {
      Serial.println(F("OK!"));
    }
  }

  if (digitalRead(14) == 0) {
    strPulsador = "ON";
    HTTPClient http;
    http.begin("https://maker.ifttt.com/trigger/button_pressed/with/key/d9KqeOd0ZUkexUG9Dx1gkNInu9SSmj6pYwmHel6ViBs");
    http.addHeader("Button press", "the button has been pressed");
    http.POST("title=foo&body=bar&userId=1");
    http.writeToStream(&Serial);
    http.end();
  } else {
    strPulsador = "N/A";
  }

  if (strPulsador != strPulsadorUltimo) { //envia el estado del pulsador solamente cuando cambia.

    strPulsadorUltimo = strPulsador;
    strPulsador.toCharArray(charPulsador, 15);
    Serial.print(F("\nSending pulsador val "));
    Serial.print(strPulsador);
    Serial.print("...");
    if (! onoff.publish(charPulsador)) {
      Serial.println(F("Failed"));
    } else {
      Serial.println(F("OK!"));
    }
    delay(1000);
  }


//Button 2

if (digitalRead(4) == 0) {
    strPulsador1 = "ON1";
    HTTPClient http;
    http.begin("https://maker.ifttt.com/trigger/button1_pressed/with/key/d9KqeOd0ZUkexUG9Dx1gkNInu9SSmj6pYwmHel6ViBs");
    http.addHeader("Button press1", "the button1 has been pressed");
    http.POST("title=foo&body=bar&userId=1");
    http.writeToStream(&Serial);
    http.end();
  } else {
    strPulsador1 = "N/A1";
  }

  if (strPulsador1 != strPulsadorUltimo1) { //envia el estado del pulsador solamente cuando cambia.

    strPulsadorUltimo1 = strPulsador1;
    strPulsador1.toCharArray(charPulsador1, 16);
    Serial.print(F("\nSending pulsador val "));
    Serial.print(strPulsador1);
    Serial.print("...");
    if (! onoff.publish(charPulsador1)) {
      Serial.println(F("Failed"));
    } else {
      Serial.println(F("OK!"));
    }
    delay(1000);
  }


//Button 3


if (digitalRead(0) == 0) {
    strPulsador2 = "ON2";
    HTTPClient http;
    http.begin("https://maker.ifttt.com/trigger/button2_pressed/with/key/d9KqeOd0ZUkexUG9Dx1gkNInu9SSmj6pYwmHel6ViBs");
    http.addHeader("Button press2", "the button2 has been pressed");
    http.POST("title=foo&body=bar&userId=1");
    http.writeToStream(&Serial);
    http.end();
  } else {
    strPulsador1 = "N/A2";
  }

  if (strPulsador2 != strPulsadorUltimo2) { //envia el estado del pulsador solamente cuando cambia.

    strPulsadorUltimo2 = strPulsador2;
    strPulsador2.toCharArray(charPulsador2, 17);
    Serial.print(F("\nSending pulsador val "));
    Serial.print(strPulsador2);
    Serial.print("...");
    if (! onoff.publish(charPulsador2)) {
      Serial.println(F("Failed"));
    } else {
      Serial.println(F("OK!"));
    }
    delay(1000);
  }



  //Button 4


if (digitalRead(2) == 0) {
    strPulsador2 = "ON3";
    HTTPClient http;
    http.begin("https://maker.ifttt.com/trigger/button3_pressed/with/key/d9KqeOd0ZUkexUG9Dx1gkNInu9SSmj6pYwmHel6ViBs");
    http.addHeader("Button press3", "the button3 has been pressed");
    http.POST("title=foo&body=bar&userId=1");
    http.writeToStream(&Serial);
    http.end();
  } else {
    strPulsador1 = "N/A3";
  }

  if (strPulsador3 != strPulsadorUltimo3) { //envia el estado del pulsador solamente cuando cambia.

    strPulsadorUltimo3 = strPulsador3;
    strPulsador3.toCharArray(charPulsador3, 17);
    Serial.print(F("\nSending pulsador val "));
    Serial.print(strPulsador3);
    Serial.print("...");
    if (! onoff.publish(charPulsador3)) {
      Serial.println(F("Failed"));
    } else {
      Serial.println(F("OK!"));
    }
    delay(1000);
  }
  // this is our 'wait for incoming subscription packets and callback em' busy subloop
  // try to spend your time here:
  mqtt.processPackets(500);
  }
  
  
  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  
  // if(! mqtt.ping()) {
  //   mqtt.disconnect();
  // }


// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.

