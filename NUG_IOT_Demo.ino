// An amalgamation of ideas and hand-crafted sketchy coding!
// This is built specifically for Wemos Mini D1 with Humidity & Relay Shields + Reed Switch

// Include libraries
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
 
// Define pins & inputs
#define DHTTYPE DHT11   // DHT 11
#define DHTPIN D4 // DHT Input pin 4
#define RELAY_PIN D1 // Relay output pin 1
#define DOOR_PIN D8 // Reed switch input pin 8
 
//****************************
//* Stuff you should change
//****************************

const char* ssid = "kern";                      // SSID
const char* password = "wirefree";              // Password
const char* wifihostname = "wemos";             // Setup client name
const char* mqtt_server = "iot.nug.team";       // MQTT broker hostname
const String mqtt_toplevelprefix = "nug";       // MQTT Topic Prefix

//****************************
//* Stuff you can change
//****************************

const String mqtt_ledTopic = "led";
const String mqtt_relayTopic = "relay";
const String mqtt_humidityTopic = "humidity";
const String mqtt_temperatureTopic = "temperature";
const String mqtt_doorTopic = "door";
const String mqtt_statusTrailer = "status";
const int pubDelay = 30000; // 30 second publishing interval. DHT sensor needs minimum 2 seconds settle time
  
//****************************
//* Stuff you shouldn't change
//****************************

// Initialize the MQTT Client
WiFiClient espClient;
PubSubClient client(espClient);
 
// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);
  
// Initialise Timers auxiliary variables
long doorNow = millis();
long dhtNow = millis();
long doorLast = 0;
long dhtLast = 0;

char* doorState = "UNDEFINED";
char* doorPrevState = "UNDEFINED";

// The setup function sets serial baud rate, starts dht.
// Sets your mqtt broker and sets the callback function
// Normally you set PINs here, but I can only get it working in callback. 
void setup() { 
  // Set door pin to input.
  pinMode(DOOR_PIN, INPUT);
 
  dht.begin();
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback); 
}


// Loop function ensures that the wemos is connected to your broker and handles data collection.
void loop() {

  // Ensure connection to MQTT
  if (!client.connected()) {
    reconnect();
  }

  // Check status of door
  checkDoorState();
   
  if(!client.loop())
    client.connect(wifihostname);
 
  // Take a note of the current time to ensure enough time has passed before reading another measurement
  dhtNow = millis();
  // Publishes new temperature and humidity every 30 seconds
  if (dhtNow - dhtLast > pubDelay) {
    dhtLast = dhtNow;
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    float f = dht.readTemperature(true);
 
    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println("Failed to read from DHT sensor.");
      return;
    }
 
    // Compute temperature values in Celsius
    float hic = dht.computeHeatIndex(t, h, false);
    static char temperatureTemp[7];
    dtostrf(hic, 6, 2, temperatureTemp);
 
    // Format the humidity value
    static char humidityTemp[7];
    dtostrf(h, 6, 2, humidityTemp);
 
    // Publish Temperature and Humidity values
    mqtt_publish(mqtt_temperatureTopic, temperatureTemp);
    Serial.print("Temperature:" );
    Serial.print(temperatureTemp);   
    mqtt_publish(mqtt_humidityTopic, humidityTemp);
    Serial.print(" Humidity:");
    Serial.println(humidityTemp);
  }
}

// This function connects your Wemos to your network
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to SSID ");
  Serial.print(ssid);
  Serial.print(" ");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Success.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Hostname: ");
  Serial.println(WiFi.hostname());
 
}

// Publishes message to structured MQTT Topic based on constants above
void mqtt_publish(String topic, String message) {
  String messagetemp = mqtt_toplevelprefix + "/" + WiFi.hostname() + "/" + topic;
  client.publish(messagetemp.c_str(), message.c_str());
}
 
// Subscribes to structured MQTT Topic based on constants above
void mqtt_subscribe(String topic) {
  String messagetemp = mqtt_toplevelprefix + "/" + WiFi.hostname() + "/" + topic;
  client.subscribe(messagetemp.c_str());
}
 
// Publishes status message
void mqtt_publishstatus(String topic, String message) {
  mqtt_publish(topic + "/" + mqtt_statusTrailer, message);
}

void checkDoorState() {
  //Checks if the door state has changed, and MQTT pub the change
  //Serial.println(doorPrevState);
  doorPrevState = doorState; //get previous state of door
  if (digitalRead(DOOR_PIN) == 1) // get new state of door
    doorState = "closed";
  else {
    doorState = "open"; }
  if (doorPrevState != doorState) { // if the state has changed then publish the change
    mqtt_publishstatus(mqtt_doorTopic, doorState);
    Serial.print("Door is now ");
    Serial.print(doorState);
    Serial.println(".");
  }
  //pub every minute, regardless of a change.
  long doorNow = millis();
  if (doorNow - doorLast > 60000) {
    doorLast = doorNow;
    mqtt_publishstatus(mqtt_doorTopic, doorState);
    //client.publish(door_topic, doorState);
    Serial.print("Door is still ");
    Serial.print(doorState);
    Serial.println(".");
  }
}

// This function is executed when we receive a message on the topics we're subscribed to.
void callback(String topic, byte* message, unsigned int length) {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  
  Serial.print("MQTT Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
 
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
 
  // If a message is received on the topic nug/wifi_hostname/led, change GPIO to match, then publish the status
  if(topic==mqtt_toplevelprefix + "/" + WiFi.hostname() + "/" + mqtt_ledTopic){
      Serial.print("Turning LED ");
      if(messageTemp == "on"){
        //Wemos have a 10K pull-up resistor, so you have to set LOW for LED to come on.
        digitalWrite(LED_BUILTIN, LOW);
        Serial.print("on.");
        mqtt_publishstatus(mqtt_ledTopic, "ON");
      }
      else if(messageTemp == "off"){
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.print("off.");
        mqtt_publishstatus(mqtt_ledTopic, "OFF");
      }
  }
 
  // If a message is received on the topic nug/wifi_hostname/relay, flip the relay, then publish the status
  if(topic==mqtt_toplevelprefix + "/" + WiFi.hostname() + "/" + mqtt_relayTopic) {
    Serial.print("Setting relay to ");
    if(messageTemp == "on"){
      digitalWrite(RELAY_PIN, HIGH);
      Serial.print("on.");
      mqtt_publishstatus(mqtt_relayTopic, "ON");
    }
    else if(messageTemp == "off") {
      digitalWrite(RELAY_PIN, LOW);
      Serial.print("off.");
      mqtt_publishstatus(mqtt_relayTopic, "OFF");
    }
  }
  Serial.println();
}

// This function reconnects your Wemos to your MQTT broker & resubscribes to the topics
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Connecting to MQTT server ");
    Serial.print(mqtt_server);
    if (client.connect(wifihostname)) {
      Serial.println(" ... Success.");  
      mqtt_publish(mqtt_statusTrailer, "ONLINE");
            
      // Subscribe to the topics defined in constants
      mqtt_subscribe(mqtt_relayTopic);
      mqtt_subscribe(mqtt_ledTopic);
      mqtt_subscribe(mqtt_doorTopic);
      
    } else {
      Serial.print(" ... failed. Error code: ");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
 

