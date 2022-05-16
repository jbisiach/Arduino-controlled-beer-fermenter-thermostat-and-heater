#include <OneWire.h>
#include <DallasTemperature.h>
#include <Keypad.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// OneWire bus intialisation 
#define ONE_WIRE_BUS 5
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Keypad intialisation
const byte ROWS = 4; 
const byte COLS = 3; 

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

byte rowPins[ROWS] = {13, 12, 11, 10}; 
byte colPins[COLS] = {9, 8, 7}; 

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);  

//I2C pins declaration
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); 

//Setup for air TMP36
int inputPin = A0;
const byte numReadings = 32; // number of readings for smoothing 
int readings[numReadings]; // readings from the analogue input
byte index = 0; // index of the current reading
unsigned int total = 0; // running total
float airRef = 1.0759; // temp calibration
float tempAir; 

//variables
int tempMin = 0;
int tempMax = 0;
volatile boolean interrupt;

void setup(void){

//Setup for TMP36
  analogReference(INTERNAL);
  Serial.begin(115200);
   for (index = 0; index < numReadings; index++) { 
    readings[index] = analogRead(inputPin);
    total = total + readings[index];
  }
  index = 0; // reset
  delay(5000); // info display time

//Pin declarations and sensor initialisation
  pinMode(2, INPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(6, OUTPUT);
  
  lcd.begin(16,2);
 
  sensors.begin();

  Serial.begin(9600);

//Begin sequence
  lcd.clear();
  lcd.backlight();

  lcd.setCursor(0,0); 
  lcd.print("   Welcome to   ");
  lcd.setCursor(0,1);  
  lcd.print(" BrewHeet (tm)  ");
  delay(3000); 
  lcd.clear();
  
  lcd.setCursor(0,0); 
  lcd.print("Enter min temp: ");
  tempMin = GetNumber();
  lcd.setCursor(0,1);
  lcd.print(tempMin);
  lcd.print("C");
  delay(3000);
  lcd.clear();
  
  lcd.setCursor(0,0); 
  lcd.print("Enter max temp: ");
  tempMax = GetNumber();
  lcd.setCursor(0,1);
  lcd.print(tempMax);
  lcd.print("C");
  delay(3000);
  lcd.clear();

  if (tempMin < tempMax){
    lcd.setCursor(0,0); 
    lcd.print("Min temp: ");
    lcd.print(tempMin);
    lcd.print("C");
    lcd.setCursor(0,1); 
    lcd.print("Max temp: ");
    lcd.print(tempMax);
    lcd.print("C");
    delay(3000);
    lcd.clear();
    lcd.setCursor(0,0); 
    lcd.print("  Go and have   ");
    lcd.setCursor(0,1); 
    lcd.print("     a beer     ");
    delay(3000);
    lcd.clear();
    lcd.noBacklight();
    } 
  else if (tempMin > tempMax){
    lcd.setCursor(0,0); 
    lcd.print(" Error: Min>Max ");
    lcd.setCursor(0,1); 
    lcd.print(" Reset BrewHeet ");
    lcd.print(tempMax);
    delay(3000);
    lcd.clear();
    lcd.noBacklight();
  }
     
  Serial.print("Min temp is: ");
  Serial.println(tempMin);
  Serial.print("Max temp is: ");
  Serial.println(tempMax); 

//Interrupt initialisation
  attachInterrupt(digitalPinToInterrupt(2), statusCheck, FALLING);
}
  
void loop(void){

//Read air temp 
 total = total - readings[index]; // subtract the last reading
  readings[index] = analogRead(inputPin); // one unused reading to clear ghost charge
  readings[index] = analogRead(inputPin); // read from the sensor
  total = total + readings[index]; // add the reading to the total
  index = index + 1; // advance to the next position in the array
  if (index >= numReadings) // if we're at the end of the array
    index = 0; // wrap around to the beginning

  tempAir = total * airRef * 0.1 / numReadings - 50.0;
  
  Serial.print("Air temp is: ");
  Serial.println(tempAir); 

//Read brew temp    
  sensors.requestTemperatures();

  Serial.print("Brew temp is: ");
  Serial.println(sensors.getTempCByIndex(0)); 
  volatile float tempBrew = sensors.getTempCByIndex(0);
  
//(De)Activate heater
  if(tempBrew < tempMin){
    digitalWrite(6, HIGH);
    digitalWrite(3, HIGH);
    digitalWrite(4, LOW);
  } 
  else if(tempBrew >= tempMax){
    digitalWrite(6, LOW);
    digitalWrite(3, LOW);
    digitalWrite(4, HIGH);
}
  delay(5000);

//Interrupt sequence
  if (interrupt == true){
    lcd.clear();
    lcd.backlight();
    lcd.setCursor(0,0); 
    lcd.print("Air temp: ");
    lcd.print(tempAir);
    lcd.print("C");
    lcd.setCursor(0,1);  
    lcd.print("Brew temp:");
    lcd.print(tempBrew);
    lcd.print("C");
    delay(5000); 
    lcd.clear();
    lcd.setCursor(0,0); 
    lcd.print("Min temp: ");
    lcd.print(tempMin);
    lcd.print("C");
    lcd.setCursor(0,1);  
    lcd.print("Max temp: ");
    lcd.print(tempMax);
    lcd.print("C");
    delay(5000); 
    lcd.clear();
    lcd.noBacklight();
    
    interrupt = false;
    }
}

//Interrupt trigger
void statusCheck(){
  interrupt = true;
  }

//Getting keypad variable
int GetNumber()
{
   int num = 0;
   char key = customKeypad.getKey();
   while(key != '#')
   {
      switch (key)
      {
         case NO_KEY:
            break;

         case '0': case '1': case '2': case '3': case '4':
         case '5': case '6': case '7': case '8': case '9':
            
            num = num * 10 + (key - '0');
            break;

         case '*':
            num = 0;
            
            break;
      }

      key = customKeypad.getKey();
   }

   return num;
}
