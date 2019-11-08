/*
 * Includes
 */
#include <PubSubClient.h>
#include <DHT.h>
#include <ESP8266WiFi.h>

/*
 * Definitions  
 */
#define OUTPUT_1 2 // RELAY, but because I swapped it in my prototype it's actually 0
#define INPUT_1 0 // Sensor, but because I swapped it in my prototype it's actually 2
#define DHTTYPE DHT11   // DHT 11  (AM2302)
#define REPORT_INTERVAL 5 // in sec

/* 
 * WiFi Settings 
 */
const char *ssid =  "SSID";
const char *pass =  "PSK";
byte ip[]     = { 10, 0, 0, 150 }; // IP for this device

/*
 * MQTT Settings
 */
//char var = 0;
char* topic;

byte broker[] = { 10, 0, 0, 3 }; // IP Address of your MQTT Server

/*
 * global variables - device channels
 */
float H;//  = dht.readHumidity();
float T;// = dht.readTemperature();
float F;// = dht.readTemperature(true);
float wPH;// = 7.0;
float wT;// = 24.0;
float hi;// = dht.computeHeatIndex(F, H);
char RelayState = 0;
/*
 * global variables - for calculus
 */  
float oldH ;
float oldT ;
/*
 * Strings
 */
String clientName; // Might be thought of as the "Thing" in openHAB.
/*
 * Define DHT
 */
DHT dht(INPUT_1, DHTTYPE, 15);
// Callback function header
void callback(char* topic, byte* payload, unsigned int length);
WiFiServer server(1883);
WiFiClient wificlient;
PubSubClient client(broker, 1883, callback, wificlient);

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  Serial.print("topic is: ");
  Serial.println((String) topic);
  Serial.print("payload: ");
  Serial.println((byte) payload[0]);

  //subscription topic
  if (strcmp(topic,"/command")==0) { 
    if (payload[0] == '0') {
      digitalWrite(OUTPUT_1, LOW);
      Serial.println("Received ");
      Serial.println((byte) payload[0]);
//      client.publish("/esp8266_19/log","Turning Off");
      Serial.println ("Turning OFF");
//      delay(100);
//      client.publish(topic,payload);
//      RelayState = digitalRead(OUTPUT_1);
//      PublishState();
      } else {
        Serial.println("Oops, something went wrong!");
        Serial.println("Received ");
        Serial.println((byte) payload[0]);
      }
      
    if (payload[0] == '1') {
      digitalWrite(OUTPUT_1, HIGH);
      Serial.println("Received ");
      Serial.println((byte) payload[0]);
//      client.publish("/esp8266_19/log","Turning ON");
      Serial.println ("Turning ON");
//      delay(100);
//      client.publish("/esp8266_19/log/RelayState","ON");
//      RelayState = digitalRead(OUTPUT_1);
//      PublishState();
      } else {
        Serial.println("Oops, something went wrong!");
        Serial.println("Received ");
        Serial.println((byte) payload[0]);
      }
  }
}
void setup()
{
  pinMode(OUTPUT_1, OUTPUT);
  digitalWrite(OUTPUT_1, LOW);
  pinMode(INPUT_1, INPUT);
  digitalWrite(INPUT_1, LOW);
  Serial.begin(115200);
  WiFiClient wclient = server.available();
  WiFiServer server = wclient.connected();
  clientNameConstructor();
  InitWiFi();
  InitMQTT();
  InitDHT();
}

void loop()
{
    if (!client.connected()) {
    InitMQTT();
    if(!client.connected())
    {
      InitWiFi();  
    }
  }
  client.loop();
//  PublishState();
} 

float readStates() {
  float mySensVals[] = {H,T,F,wPH,wT,hi};
  H = dht.readHumidity();
  T = dht.readTemperature();
  F = dht.readTemperature(true);
  wPH = 7.0;
  wT = 24.0;
  hi = dht.computeHeatIndex(F, H);
  
  if (isnan(H) || isnan(T) || isnan(F)) {
    client.publish("/esp8266_19/log","Failed to read from DHT sensor!");
    Serial.println("Failed to read from DHT sensor!");
    delay(5000);
    exit;
  }
}

String PayloadConstructor() {
//payload = "{\"DeviceName\":";
//  payload += clientName;
//  payload = "{\"DeviceLocation\":";
//  payload += DeviceLocation;
//  payload = "{\"SimulatedLocation\":";
//  payload += SimulatedLocation;
  String   payload = "{\"Humidity\":";
  payload += H;
//  payload += mySensVals[0];
  payload += ",\"GreenhouseTemperature\":";
  payload += T;
  payload += ",\"HeatIndex\":";
  payload += hi;
//  payload += mySensVals[1];;
  payload += ",\"WaterPH\":";
  payload += wPH;
//  payload += mySensVals[2];;
  payload += ",\"WaterTemperature\":";
  payload += wT;
//  payload += mySensVals[3];;
  payload += ",\"SwitchState\":";
  payload += RelayState;
//  payload += mySensVals[4];;
  payload += "}";
   
    if (T != oldT || H != oldH )
  {
    oldT = T;
    oldH = H;
  }
  return payload;
}

//String PublishState(char* topic, byte* payload, unsigned int length) 
//{  
//  if (!client.connected()) {
//    if (client.connect((char*) clientName.c_str())) {
//      client.publish("/esp8266_19/log","Connected to MQTT broker again");
//      Serial.println("Connected to MQTT broker again");
//      Serial.print("topic is: ");
//      Serial.println(topic);
//    }
//    else {
//      client.publish("/esp8266_19/log","MQTT connect failed");
//      Serial.println("MQTT connect failed");
//      client.publish("/esp8266_19/log","will reset and try again");
//      Serial.println("Will reset and try again...");
//      abort();
//    }
//  }
//
//  if (client.connected()) {
//    client.publish("/esp8266_19/log","Sending payload");
//    Serial.print("Sending payload: ");
////    Serial.println(payload);
//
//    if (client.publish(topic, (char*) payload.c_str())) {
//      client.publish("/esp8266_19/log","Publish ok");
//      Serial.println("Publish ok");
//    }
//    else {
//      client.publish("/esp8266_19/log","Publish failed!");
//      Serial.println("Publish failed");
//      abort();
//    }
//  }
//
//    int cnt = REPORT_INTERVAL;
//    while (cnt--)
//    delay(1000);
//}

void InitWiFi()
{
  client.publish("/esp8266_19/log","Starting WiFi connection");
  Serial.println("Starting WiFi connection "); 
  Serial.println();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  client.publish("/esp8266_19/log","WiFi connected");
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("");
  
  server.begin();
  Serial.println("Webserver started");
  client.publish("/esp8266_19/log","Webserver started");
  }

void InitMQTT()
{
  if (client.connect("arduinoClient"))
  {
    client.publish("/radiation/log","MQTT Connected");
    Serial.println("MQTT Connected");
    client.publish("/radiation/log","RadSense online!");
    client.subscribe("/home/out/radiation/#");  // Subscribe to all messages for this device
  }
  
  Serial.print("Connecting to ");
  client.publish("/radiation/log","Connecting to ");

  Serial.print(" as ");
  client.publish("/radiation/log","as");
  
  Serial.println(clientName);
  client.publish("/radiation/log","radiation");
  
  if (client.connect((char*) clientName.c_str()))
  {
    Serial.println("Connected to MQTT broker");
    client.publish("/radiation/log","Connected to MQTT broker");
    
    Serial.print("topic is: ");
    Serial.println(topic);
    client.publish("/radiation/log","topic is: ");
    
    
    if (client.publish("/radiation/log", "hello from Babylawn!"))
    {
      Serial.println("Publish ok");
      client.publish("/radiation/log","Publish ok");
      return;
    }
    else
    {
      Serial.println("Publish failed");
//      client.publish("/radiation/log","Publish failed!");
    }
  }
  else
  {
    Serial.println("MQTT connect failed");
//    client.publish("/radiation/log","MQTT connect failed");
    Serial.println("Will reset and try again...");
//    client.publish("/radiation/log","Will reset and try again...");
    
    return;
  }
}

void InitDHT()
{
dht.begin();
  oldH = -1;
  oldT = -1;
  }

String clientNameConstructor()
{
  
  clientName += "esp8266-";
  uint8_t mac[6];
  WiFi.macAddress(mac);
  clientName += macToStr(mac);
  clientName += "-";
  clientName += String(micros() & 0xff, 16);
  return clientName;
  }

String macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += '-';
      }
  return result;
}
void OperateSwitch() {
        if (RelayState== 1) {
        client.publish("/home/out/esp8266_19","state is 1");
      }
      if (RelayState ==0) {
        client.publish("/home/out/esp8266_19","state is 0");
        }
      Serial.println(RelayState);
  }
