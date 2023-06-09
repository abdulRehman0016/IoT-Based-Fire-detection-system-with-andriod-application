/* We have 4 sensors and LCD
MQ-7, MQ-135, DHT22, Flame Sensor
Buzzer and LCD
A wifi-Module Esp8266 For thingSpeaks
*/


#include <dht.h>
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>  

String statusChWriteKey = "4VMRH0IGIKA8MT2M";

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

SoftwareSerial EspSerial(6, 7); // Rx,  Tx
#define HARDWARE_RESET 9
#define speed8266 9600

#define dataPin 8

// Variables to be used with timers
long writeTimingSeconds = 5; // ==> Define Sample time in seconds to send data
long startWriteTiming = 0;
long elapsedWriteTime = 0;

int spare = 0;
boolean error;

dht DHT;
const int sensorMin = 0;     // sensor minimum
const int sensorMax = 1024;  // sensor maximum
int flamevalue;
const int buzzer = 10;

void setup() {
  
  Serial.begin(9600);
   pinMode(buzzer, OUTPUT);
   digitalWrite(buzzer, LOW);
   lcd.begin(20, 4);

  Serial.begin(speed8266);
  
  EspSerial.begin(9600);       // Comunicacao com Modulo WiFi
  pinMode(HARDWARE_RESET,OUTPUT); 
  digitalWrite(HARDWARE_RESET, HIGH);
 


  EspHardwareReset(); //Reset do Modulo WiFi
  startWriteTiming = millis(); // starting the "program clock"
   
}
void loop() 
{
  start: // label
    error = 0;

    elapsedWriteTime = millis() - startWriteTiming;

    if (elapsedWriteTime > (writeTimingSeconds * 500))
    {
        readSensors();
        writeThingSpeak();
        startWriteTiming = millis();
    }

    if (error == 1) // Resend if transmission is not completed
    {
        Serial.println(" <<<< ERROR >>>>");
        delay(2000);
        goto start; // go to label "start"
    }
    Flame();   // Flame with A2
    dhts();      //  Dht22 Pin 8
    MQ135s();     // MQ-135 with A1
    MQ7s();        // MQ-7 with A0
}
void readSensors(){
  int sensorReading   = analogRead(A2);
  int readData = DHT.read22(dataPin);
  float t = DHT.temperature; // Gets the values of the temperature
	float h = DHT.humidity; // Gets the values of the humidity
  int MQ7 = analogRead(A0);
  int MQ135 = analogRead(A1);

}
void Flame(void){
	int sensorReading   = analogRead(A2);
	int range = map(sensorReading,   sensorMin, sensorMax, 0, 3);
  flamevalue = range;
  if (range == 0){
    tone(buzzer, 1000);
  }
  else if(range == 1){
    tone(buzzer, 1000);
  }
  else{
    noTone(buzzer);
  }
  switch (range) {
   case 0:    // A fire closer than 1.5 feet away.
    //Serial.println("**Close Fire**");
    lcd.setCursor(0,0);
    lcd.print("**Close Fire**   ");  
    break;
  case 1:    
  // A fire between 1-3 feet away.
   //Serial.println("**Distant Fire**");
    lcd.setCursor(0,0);
    lcd.print("**Distant Fire**");  
    break;
  case 2:    // No fire detected.
    //Serial.println("**No Fire**");
    lcd.setCursor(0,0);
    lcd.print("**No Fire**          "); 
    break;
  }
  // delay between reads
}
void dhts(void){
  int readData = DHT.read22(dataPin);
  float t = DHT.temperature; // Gets the values of the temperature
	float h = DHT.humidity; // Gets the values of the humidity
  //Printing the results on the serial monitor
	// Serial.print("Temperature= ");
  // Serial.print(t);
  // Serial.write(0xC2);
  // Serial.write(0xB0);
  //Serial.print("C | ");
  // Serial.print("Humidity = ");
	// Serial.print(h);
	// Serial.println("%"); 
  lcd.setCursor(0, 1);
  lcd.print("Temperature= ");
	lcd.print(t);
  lcd.print(F("\xDF"));
  lcd.print("C");
  delay(2000);
  lcd.setCursor(0, 1);
  lcd.print("Humidity = ");
	lcd.print(h);
	lcd.print("%   ");
  delay(2000); // Delays 1.5 secods	 
 }
void MQ7s(void){
   int MQ7 = analogRead(A0);
   int mq7l = map(MQ7,sensorMin,sensorMax,0,2);
   if (mq7l == 2){
    tone(buzzer, 1000);
  }
  else{
    noTone(buzzer);
  }
   switch(mq7l){
  case 0:    // A fire closer than 1.5 feet away.
   //Serial.println("**NO CO detected**");
    lcd.setCursor(0,2);
    lcd.print("**NO CO detected**");
    break;
  case 1:    // A fire between 1-3 feet away.
    //Serial.println("**CO detected**");
    lcd.setCursor(0,2);
    lcd.print("**CO detected**     "); 
    break;
  case 2:    // No fire detected.
    //Serial.println("**CO detected highly**");
    lcd.setCursor(0,2);
    lcd.print("**CO detected highly");  
    break;
  }
  
}
void MQ135s(void){
  int MQ135 = analogRead(A1);
  int mq135l = map(MQ135,sensorMin,sensorMax,0,3);
   if (mq135l == 1){
    tone(buzzer, 1000);
  }
  else if(mq135l == 2){
   
    tone(buzzer, 1000);
  }
  else if(mq135l == 3){
   
    tone(buzzer, 1000);
  }
  else{
    noTone(buzzer);
  }
  switch(mq135l){
  case 0:   
  //Serial.println("**NO Somke**");
  lcd.setCursor(0,3);
  lcd.print("**NO Smoke**");
  break;
  case 1:   
  //Serial.println("**Smoke!!**");
  lcd.setCursor(0,3);
  lcd.print("**Smoke!!**");
  break;
  case 2:    // No fire detected.
  lcd.setCursor(0,3);
  lcd.print("Smoke detected highly**");
  break;
  case 3:
  //Serial.println("**Smoke detected highly**");
  lcd.setCursor(0,3);
  lcd.print("**Smoke detected very-highly**");
  break;
  }
}

/********* Conexao com TCP com Thingspeak *******/
void writeThingSpeak(void)
{
    startThingSpeakCmd();

    // preparacao da string GET
    String getStr = "GET /update?api_key=";
    getStr += statusChWriteKey;
    getStr += "&field1=";
    getStr += String(flamevalue);
    getStr += "&field2=";
    getStr += String(DHT.temperature);
    getStr += "&field3=";
    getStr += String(DHT.humidity);
    getStr += "&field4=";
    getStr += String(analogRead(A0));
    getStr += "&field5=";
    getStr += String(analogRead(A1));
    getStr += "&field6=";
    getStr += String(spare);
    getStr += "\r\n\r\n";

    sendThingSpeakGetCmd(getStr);
}

/********* Reset ESP *************/
void EspHardwareReset(void)
 {
    Serial.println("Reseting.......");
    digitalWrite(HARDWARE_RESET, LOW);
    delay(500);
    digitalWrite(HARDWARE_RESET, HIGH);
    delay(8000); // Tempo necessário para começar a ler
    Serial.println("RESET");
}

/********* Start communication with ThingSpeak*************/
void startThingSpeakCmd(void)
 {
    EspSerial.flush(); // limpa o buffer antes de começar a gravar

    String cmd = "AT+CIPSTART=\"TCP\",\"";
    cmd += "184.106.153.149"; // Endereco IP de api.thingspeak.com
    cmd += "\",80";
    EspSerial.println(cmd);
    Serial.print("enviado ==> Start cmd: ");
    Serial.println(cmd);

    if (EspSerial.find("Error"))
    {
        Serial.println("AT+CIPSTART error");
        return;
    }
}

/********* send a GET cmd to ThingSpeak *************/
String sendThingSpeakGetCmd(String getStr)
 {
    String cmd = "AT+CIPSEND=";
    cmd += String(getStr.length());
    EspSerial.println(cmd);
    Serial.print("enviado ==> lenght cmd: ");
    Serial.println(cmd);

    if (EspSerial.find((char *)">"))
    {
        EspSerial.print(getStr);
        Serial.print("enviado ==> getStr: ");
        Serial.println(getStr);
        delay(500); // tempo para processar o GET, sem este delay apresenta busy no próximo comando

        String messageBody = "";
        while (EspSerial.available())
        {
            String line = EspSerial.readStringUntil('\n');
            if (line.length() == 1)
            { // actual content starts after empty line (that has length 1)
                messageBody = EspSerial.readStringUntil('\n');
            }
        }
        Serial.print("MessageBody received: ");
        Serial.println(messageBody);
        return messageBody;
    }
    else
    {
        EspSerial.println("AT+CIPCLOSE");                   // alert user
        Serial.println("ESP8266 CIPSEND ERROR: RESENDING"); // Resend...
        spare = spare + 1;
        error = 1;
        return "error";
    }
}




