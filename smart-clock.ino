// Smart Clock 
/*
	Copyright 2017 Bartosz Kupiec
	Can be used by anyone, only for non-commerical use.
	
	This code has been made to work with an Arduino Nano using ATmega 328.
*/




//Libraries
#include <IRremote.h>
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>;
#include <limits.h>
#include <Time.h>
#include <TimeLib.h>

//Constants
#define DHTPIN 2          // what pin we're connected to
#define DHTTYPE DHT22     // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor for normal 16mhz Arduino


//Variables
int chk;
float hum;  		  //Stores humidity value
float temp; 		  //Stores temperature value 
int RECV_PIN = 4;	  // receving pin for the remote
int incomingByte = 0; // used when getting input 
bool toDisplay;		  // specifies what should be displayed 
bool screenOn;		  // keeps the state of the LCD backlight, on or off 
IRrecv irrecv(RECV_PIN); // initalize reciever 
decode_results results;  // storage for the remote results 
//Hex values for remote
unsigned long btn_1 = 0xFF30CF;unsigned long btn_2 = 0xFF18E7;unsigned long btn_3 = 0xFF7A85;
unsigned long btn_4 = 0xFF10EF;unsigned long btn_5 = 0xFF38C7;unsigned long btn_6 = 0xFF5AA5;
unsigned long btn_minus = 0xFFE01F;unsigned long btn_plus = 0xFFA857;

int led_red = 11;
int led_green = 10;
int led_blue = 9;

#define I2C_ADDR 0x27
#define BACKLIGHT_PIN 3
#define DHTPIN 2
#define En_pin 2
#define Rw_pin 1
#define Rs_pin 0
#define D4_pin 4
#define D5_pin 5
#define D6_pin 6
#define D7_pin 7

LiquidCrystal_I2C lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);

void setOurTime()
{
 setTime(4,20,00,5,4,2017);
}

void displayTime() // display the time and date on the LCD screen 
{
lcd.clear();
// Print a message to the LCD.
lcd.print("Date ");lcd.print(month());lcd.print("/");lcd.print(day());lcd.print("/");lcd.print(year());
lcd.setCursor(0,1);         // set the cursor to the next line on our LCD ( line 2 )
lcd.print("Time ");lcd.print(hour());lcd.print(":");lcd.print(minute());lcd.print(":");lcd.print(second());
lcd.setCursor(0,0);         // set the cursor to the previous line on our LCD ( line 1 )
delay(250);
  
}


void printTemp()
{ 
    hum = dht.readHumidity();
    temp= dht.readTemperature();
    //Print temp and humidity values to lcd
    lcd.print("Humidity: ");lcd.print(hum);lcd.print(" %");
    lcd.setCursor(0,1);lcd.print("Temp:     ");lcd.print((temp*9/5) + 32);lcd.print(" F");
    lcd.setCursor(10,2);lcd.print(temp);lcd.print(" C"); 
    lcd.setCursor(0,0);         // set the cursor to the previous line on our LCD ( line 1 )
}

void decodeRemote()
{
  
 if(results.value == btn_1) // pressed 1
     toDisplay = !toDisplay;
 else if(results.value == btn_2) // pressed 2
     digitalWrite(led_red, HIGH);
 else if(results.value == btn_3) // pressed 3
      digitalWrite(led_green, HIGH);
 else if(results.value == btn_4) // pressed 4
      digitalWrite(led_blue, HIGH);   
 else if(results.value == btn_5) // pressed 5
      {
      digitalWrite(led_red, LOW);    
      digitalWrite(led_blue, LOW);    
      digitalWrite(led_green, LOW);    
      }
 else if(results.value == btn_minus) // pressed - 
	{  
	 screenOn = !screenOn; // turn screen from on-> off or off -> on 
     if(screenOn == true)
        lcd.setBacklight(HIGH);
     else
        lcd.setBacklight(LOW);
  }
  else if(results.value == btn_plus) // pressed + 
     lcd.setBacklight(HIGH);    // for now always set high 
     

}

void getRemoteInput()
{
   if (irrecv.decode(&results)) {
   lcd.clear();    // clear screen 
   // lcd.print(results.value,HEX); // print out the code . . .
   // delay(4000); 					// then delay to find out the values, DEBUG ONLY 
   decodeRemote(); // decode output 
   irrecv.resume(); // Receive the next value
  }
}

void setup()
{ 
  pinMode(led_red, OUTPUT);pinMode(led_green, OUTPUT);pinMode(led_blue, OUTPUT);
  setOurTime();								   // set initial time 
  toDisplay = false;						   // when first turned on display temp 
  screenOn = true;							   // when starting, make LED backlight on 
  dht.begin();                                 // set up temp monitor 
  lcd.begin (20,4,LCD_5x8DOTS);                // set up LCD  
  lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE); // set up backlight pin
  lcd.setBacklight(HIGH);  lcd.home ();        // turn on LED screen, and set to home 
  while (!Serial);                             //delay for serial
  irrecv.enableIRIn();                         // Start the receiver
  Serial.begin(9600);                          // Begin serial communcation, used for input 
}

void loop()
{
  getRemoteInput();
  if(toDisplay)
  displayTime();
  else
  printTemp();
}
