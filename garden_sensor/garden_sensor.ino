// Includes
#include "DHT.h"
#include <RCSwitch.h>
#include "LowPower.h"

// Globals
#define DHTTYPE DHT22
#define DEBUG 1

#define TEMP "1000"
#define HUM "2000"
#define HYGRO "3000"
#define ERRORCODE "9999"
#define SOURCEADDR "100000"
#define PROJECTCODE "444000000"

// Config values
const int dhtPowerPin = 3;
const int dhtPin = 4;

const int hygroPowerPin = 10;

const int pinLed = 13;

const int rfPowerPin = 7;
const int rfPin = 6;


const int timeToSleep = 3600;
const int transmitRepeat = 10;

unsigned long nonce = 10000;

DHT dht(dhtPin, DHTTYPE);
RCSwitch rf = RCSwitch();

void setup() {
  Serial.begin(9600);
  write_log("Init app");
  pinMode(dhtPowerPin, INPUT);
  
  pinMode(pinLed, OUTPUT);
  digitalWrite(pinLed, HIGH);
  delay(3000);
  digitalWrite(pinLed, LOW);
  pinMode(pinLed, INPUT);
  

}

void loop() {
  write_info(">>>Starting transmitter");
  pinMode(rfPowerPin, OUTPUT);
  digitalWrite(rfPowerPin, HIGH);
  rf.enableTransmit(rfPin);
  rf.setRepeatTransmit(transmitRepeat);

  write_info(">>>Temp & Humidity");
  measure_hum_temp();

  write_info(">>>Hygro");
  measure_hygro();

  write_info(">>>Stopping transmitter");
  rf.disableTransmit();
  digitalWrite(rfPowerPin, LOW);
  pinMode(rfPowerPin, INPUT);

  write_info(">>>Go to sleep");
  delay(500);
  sleep_seconds(timeToSleep);
}

void measure_hum_temp() {
  write_log("Powering sensor");
  pinMode(dhtPowerPin, OUTPUT);
  digitalWrite(dhtPowerPin, HIGH);

  write_log("Initializing sensor");
  dht.begin();

  delay(2000);
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) {
    write_log("Failed to read from DHT sensor!");
    send_data(atol(ERRORCODE), atol(TEMP));
    send_data(atol(ERRORCODE), atol(HUM));
  } else {
    write_log("Temp - " + String(temp) + "|| Hum - " + String(hum));
    send_data(int(temp * 10), atol(TEMP));
    send_data(int(hum * 10), atol(HUM));
  }

  write_log("Shutting sensor down");
  digitalWrite(dhtPowerPin, LOW);
  pinMode(dhtPin, INPUT);
  pinMode(dhtPowerPin, INPUT);

}

void measure_hygro() {
  write_log("Powering sensor");
  pinMode(hygroPowerPin, OUTPUT);
  digitalWrite(hygroPowerPin, HIGH);

  delay(500);

  int sensorValue = analogRead(A0);
  write_log("Sensor value - " + String(sensorValue));

  sensorValue = constrain(sensorValue, 300, 1023);
  int hygro = map(sensorValue, 0, 1023, 100, 0);

  write_log("Hygro - " + String(hygro));
  send_data(hygro, atol(HYGRO));

  write_log("Shutting sensor down");
  digitalWrite(hygroPowerPin, LOW);
  pinMode(hygroPowerPin, INPUT);
}

void send_data(long dataToSend, long dataType) {
  /*
      datagram: <PROJECTCODE><SOURCEADDR><NONCE><DATATYPE><VALUE>
      <NONCE> rolling counting from 10000 to 90000
  */
  long dataSum = atol(ERRORCODE);
  if (dataToSend == dataSum) {
    write_log("Got errorcode");
  } else {
    dataSum = dataToSend + dataType;
    write_log("Sending data - " + String(dataToSend) + " for type - " + String(dataType));
  }

  dataSum = dataSum + atol(PROJECTCODE) + atol(SOURCEADDR) + nonce;
  write_log("DataSum incl. Sourcecode: " + String(dataSum));

  rf.send(dataSum, 24);

  nonce = nonce + 10000;
  if (nonce >= 100000) {
    nonce = 10000;
  }
}

void write_info(String msg) {
  Serial.println("[INFO]: " + msg);
}
void write_log(String msg) {
  if (DEBUG == 1) {
    Serial.println("[DEBUG]: " + msg);
  }
}
void sleep_seconds(int seconds)
{
  for (int i = 0; i < seconds; i++) { 
     LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF); 
  }
}
