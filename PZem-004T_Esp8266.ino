#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <InfluxDbClient.h>
#include <PZEM004Tv30.h>
#include "config.h"

InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);
Point sensor("PowerStatus");

/* Use software serial for the PZEM
 * Pin 5 Rx (Connects to the Tx pin on the PZEM)
 * Pin 4 Tx (Connects to the Rx pin on the PZEM)
*/
PZEM004Tv30 pzem(5, 4);

// Connecting to the WIFI network
bool Connect_WIFI() {
  if (WiFi.status() == WL_CONNECTED)
    return 0; //Meaningno action needed
    
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  return 1; //Connection or reconnection was needed!
}
WiFiClient espClient;

void ConnectToInflux() {
  
  client.setInsecure();
  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }
}

void setup() {
  Serial.begin(115200);
  Connect_WIFI();
  // Add constant tags - only once
  sensor.addTag("Device", "ESP8266_Main");
  sensor.addTag("SSID", WiFi.SSID());
  sensor.addTag("Power_Phase", "Geral");
  ConnectToInflux();
}

double sens[4]; //Array of 4 Values
void loop() {
  if (Connect_WIFI())
    ConnectToInflux();
  sensor.clearFields();
  sensor.addField("wifi-rssi", WiFi.RSSI());
  float voltage = pzem.voltage();
  if(!isnan(voltage)){
      sensor.addField("voltage", voltage);
  } else {
      Serial.println("Error reading voltage");
  }
  float current = pzem.current();
  if(!isnan(current)){
        sensor.addField("current", current);
    } else {
        Serial.println("Error reading current");
    }
    float power = pzem.power();
    if(!isnan(power)){
        sensor.addField("power", power);
    } else {
        Serial.println("Error reading power");
    }
    float energy = pzem.energy();
    if(!isnan(energy)){
        sensor.addField("energy", energy);
    } else {
        Serial.println("Error reading energy");
    }
    float frequency = pzem.frequency();
    if(!isnan(frequency)){
        sensor.addField("freq", frequency);
    } else {
        Serial.println("Error reading frequency");
    }
    float pf = pzem.pf();
    if(!isnan(pf)){
        sensor.addField("power-factor", pf);
    } else {
        Serial.println("Error reading power factor");
    }
  
  // Write point
  if (!client.writePoint(sensor)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  Serial.println("Sleep 10s");
  delay(10000);
}
