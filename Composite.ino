#include <SoftwareSerial.h>
#include <Wire.h>
#include <dht.h>
#include <AltSoftSerial.h>


// Pins
#define DHTpin 22 // DHT22 on D2
#define ultrasonicEcho 24 // ultrasonic echo at D5
#define ultrasonicTrigger 26 // ultrasonic trigger at D4
#define moisturePowerLevel 28
#define ledPin 9 // Photoresistor LED at D9 Debugging
#define RE 30
#define DE 32

#define pResistor A1 // Photoresistor at A1
#define moisturePin A0 // MoisturePin at A0

// Constants
#define soilWet 500 // Define max value we consider soil 'wet'
#define soilDry 750 // Define min value we consider soil 'dry'
const byte nitro[] = {0x01,0x03, 0x00, 0x1e, 0x00, 0x01, 0xe4, 0x0c};
const byte phos[] = {0x01,0x03, 0x00, 0x1f, 0x00, 0x01, 0xb5, 0xcc};
const byte pota[] = {0x01,0x03, 0x00, 0x20, 0x00, 0x01, 0x85, 0xc0};

// Variables
dht DHT; // Creats a DHT object
//SoftwareSerial mySerial(2, 3); // RX, TX
byte values[11];
//SoftwareSerial mod(13, 10);
//AltSoftSerial altSerial;
SoftwareSerial mySerial(53, 51);

// array holding the serial data
float data_read[5]; //dist, photo, t, h, mois

/*
 * Setup
 */
void setup() {
  // initialize serial ports and pinse
  Serial.begin(9600);
//  Serial1.begin(9600);
  Serial3.begin(9600);
  //mod.begin(19200);
//  altSerial.begin(19200);
  mySerial.begin(9600);
  Serial3.print("\n\r\n\r");
//   Set into bypass mode for zigbee
  delay(1000);
  Serial3.print("B");
  delay(1000);
  
  // Set pin modes
  pinMode(ultrasonicTrigger,OUTPUT);
  pinMode(ultrasonicEcho,INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(pResistor, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(moisturePowerLevel, OUTPUT);
  pinMode(RE, OUTPUT);
  pinMode(DE, OUTPUT);
  
  // Set pin levels
  // ensure ultrasonic ranging sensor trigger is initially low
  digitalWrite(ultrasonicTrigger, LOW);
  digitalWrite(moisturePowerLevel, LOW);
  delay(500);
}

/*
 * Loop
 */
void loop() {
  /*
   * Ultrasonic sensor
   */
//  trigger ultrasonic sensor
  digitalWrite(ultrasonicTrigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(ultrasonicTrigger, LOW);
  //read and convert sensor reading to centimeters
  float duration = pulseIn(ultrasonicEcho, HIGH);
  float distance = duration/2*.0343;
  int scaled_dist = distance*100;
  data_read[0] = distance;

  //Debugging
  Serial.print("Distance: ");
  Serial.println(scaled_dist);
  if (distance > 9) Serial.println("refill the water");
  
 /*
  * Photo resistor
  */
  int photoresistorValue = analogRead(pResistor);
  data_read[1] = photoresistorValue;

  //Debugging
  if (photoresistorValue > 500){
    digitalWrite(ledPin, LOW); //Turn led off
    digitalWrite(LED_BUILTIN, LOW);
  }
  else{
    digitalWrite(ledPin, HIGH); //Turn led on
    digitalWrite(LED_BUILTIN, HIGH);
  }
  Serial.print("photoresistorValue: ");
  Serial.println(photoresistorValue);
  
 /*
  *  DHT
  */
  //Uncomment whatever type you're using!
  int dhtData = DHT.read22(DHTpin); // DHT22/AM2302
  float t = float_one_point_round(DHT.temperature); // Gets the values of the temperature
  float h = float_one_point_round(DHT.humidity); // Gets the values of the humidity
  int scaled_t = t*10;
  int scaled_h = h*10;
  data_read[2] = t;
  data_read[3] = h;

  // Debugging
  
  Serial.print("Temperature = ");
  Serial.print(scaled_t);
  Serial.print(" ");
  Serial.print("C | ");
  /*
  Serial.print((t * 9.0) / 5.0 + 32.0);//print the temperature in Fahrenheit
  Serial.print(" ");
  Serial.println("F ");
  */
  Serial.print("Humidity = ");
  Serial.print(scaled_h);
  Serial.println(" % ");

  /*
   *  Dirt Moisture 
   */
  int moisture = readMoisture();
  data_read[5] = t;
  
  //Debugging
  Serial.print("Moisture: ");
  Serial.println(moisture);
  if (moisture < soilWet) {
      Serial.println("Status: Soil is too wet");
  } else if (moisture >= soilWet && moisture < soilDry) {
      Serial.println("Status: Soil moisture is perfect");
  } else {
      Serial.println("Status: Soil is too dry - time to water!");
  }
  
  

  /*
   * NPK sensor
   */
  byte val1,val2,val3;
  val1 = nitrogen();
  delay(250);
  val2 = phosphorous();
  delay(250);
  val3 = potassium();
  delay(250);

  // Print values to the serial monitor
  Serial.print("Nitrogen: ");
  Serial.print(val1);
  
  Serial.println(" mg/kg");
  
  Serial.print("Phosphorous: ");
  Serial.print(val2);
  Serial.println(" mg/kg");
 
  Serial.print("Potassium: ");
  Serial.print(val3);
  Serial.println(" mg/kg");  
 
  
  //Send data array over
  //mySerial.println(data_read);
  //Serial debuging
  //mySerial.println(distance);
  Serial3.println(scaled_dist);
  Serial3.println(photoresistorValue);
  Serial3.println(scaled_t);
  Serial3.println(scaled_h);
  Serial3.println(moisture);
  Serial3.println(val1);
  Serial3.println(val2);
  Serial3.println(val3);
  
  //wait for next loop
  Serial.println("");
  delay(7000);
}

//Functions
float float_one_point_round(float value)
{
        return ((float)((int)(value * 10))) / 10;
}

int readMoisture() {
    digitalWrite(moisturePowerLevel, HIGH);    // Turn the sensor ON
    delay(10);                            // Allow power to settle
    int val = analogRead(moisturePin);    // Read the analog value form sensor
    digitalWrite(moisturePowerLevel, LOW);        // Turn the sensor OFF
    return val;                            // Return analog moisture value
}

byte nitrogen(){
  digitalWrite(DE,HIGH);
  digitalWrite(RE,HIGH);
  delay(10);
  if(mySerial.write(nitro,sizeof(nitro))==8){
    digitalWrite(DE,LOW);
    digitalWrite(RE,LOW);
    for(byte i=0;i<7;i++){
    //Serial.print(mod.read(),HEX);
    values[i] = mySerial.read();
    //Serial.print(values[i],HEX);
    }
    //Serial.println();
  }
  return values[4];
}
 
byte phosphorous(){
  digitalWrite(DE,HIGH);
  digitalWrite(RE,HIGH);
  delay(10);
  if(mySerial.write(phos,sizeof(phos))==8){
    digitalWrite(DE,LOW);
    digitalWrite(RE,LOW);
    for(byte i=0;i<7;i++){
    //Serial.print(mod.read(),HEX);
    values[i] = mySerial.read();
    //Serial.print(values[i],HEX);
    }
    //Serial.println();
  }
  return values[4];
}

byte potassium(){
  digitalWrite(DE,HIGH);
  digitalWrite(RE,HIGH);
  delay(10);
  if(mySerial.write(pota,sizeof(pota))==8){
    digitalWrite(DE,LOW);
    digitalWrite(RE,LOW);
    for(byte i=0;i<7;i++){
    //Serial.print(mod.read(),HEX);
    values[i] = mySerial.read();
    //Serial.print(values[i],HEX);
    }
    //Serial.println();
  }
  return values[4];
}
/*
 * END
 */
