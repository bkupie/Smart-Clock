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
#include <DHT.h>
#include <limits.h>
#include "Sodaq_DS3231.h"



//Constants
#define BUZZERPIN 6       // pin for the buzzer 
#define DHTPIN 2          // pin for the temp/humidity sensor 
#define RECV_PIN 4	      // receving pin for the remote
#define DHTTYPE DHT22     // DHT 22  (AM2302)

//Constants for LCD 
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
 
DHT dht(DHTPIN, DHTTYPE); // initialize DHT sensor for normal 16mhz Arduino
IRrecv irrecv(RECV_PIN);  // initalize reciever 
decode_results results;   // storage for the remote results 

//Variables
char weekDay[][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
uint32_t old_ts;
unsigned long previousMillis = 0; // store previous millis value
long onTime = 10000;   //determines how long the screen should be turned on for    
float hum;  		  // store humidity value
float temp; 		  // store temperature value 
int incomingByte = 0; // used when getting input 
bool screenOn;		  // keeps the state of the LCD backlight, on or off 

typedef struct myAlarm{
int hour;
int minute;
int second;
bool on;
}myAlarm;

struct myAlarm alarm;


//Hex values for remote
unsigned long btn_minus = 0xFFE01F; unsigned long btn_plus = 0xFFA857;		unsigned long btn_eq = 0xFF906F;
unsigned long btn_0 = 0xFF30CF;     unsigned long btn_100_plus = 0xFF9867;	unsigned long btn_200_plus = 0xFFB04F;
unsigned long btn_1 = 0xFF30CF;		unsigned long btn_2 = 0xFF18E7;			unsigned long btn_3 = 0xFF7A85;
unsigned long btn_4 = 0xFF10EF;		unsigned long btn_5 = 0xFF38C7;			unsigned long btn_6 = 0xFF5AA5;
unsigned long btn_7 = 0xFF42BD;		unsigned long btn_8 = 0xFF4AB5;			unsigned long btn_9 = 0xFF52AD;

LiquidCrystal_I2C lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);


void screenTimer()	// turn the screen on for a limited amount of time 
{
  unsigned long currentMillis = millis();  // get current time 
  if(screenOn && currentMillis - previousMillis >= onTime)
  {   
	  // if our screen is on, and timer is expired, turn screen off 
      lcd.clear();						// clear the screen 
	  lcd.setBacklight(LOW);			// turn backlight off  
	  previousMillis = currentMillis;   // reset the timer for future use 
	  screenOn = false; 				// change state to being off 
  }
	
}

void alarmSet(int hr,int min,int sec)
{
	alarm.hour = hr;
	alarm.minute = min;
	alarm.second = sec;
}
void alarmOn()  { alarm.on = true;  }
void alarmOff() { alarm.on = false; }

void alarmDisplay(){
	  lcd.clear();
	  lcd.setBacklight(HIGH); // just incase the screen is turned off, which it should be 
	  lcd.setCursor(0,0);lcd.print("+-----------------+");
	  lcd.setCursor(0,1);lcd.print("|     WAKE UP     |");
	  lcd.setCursor(0,2);lcd.print("|   IT'S  "); lcd.print(alarm.hour,DEC); lcd.print(":");lcd.print(alarm.minute, DEC);lcd.print("   |");
	  lcd.setCursor(0,3);lcd.print("+-----------------+");
}

void alarmSound(){
	analogWrite(BUZZERPIN,50);delay(1000);analogWrite(BUZZERPIN,0);
	delay(1000);analogWrite(BUZZERPIN,50);delay(1000);analogWrite(BUZZERPIN,0);
	delay(1000);analogWrite(BUZZERPIN,50);delay(1000);analogWrite(BUZZERPIN,0);
	delay(1000);
}


void alarmGoOff()
{
	alarmDisplay();	
	alarmSound();
	lcd.setBacklight(LOW);		
}

//writing any non-existent time-data may interfere with normal operation of the RTC.
//Take care of week-day also.
//year, month, date, hour, min, sec and week-day(starts from 0 and goes to 6)
void timeSet(int y, int m,int d,int h,int min, int s, int wd)
{
	DateTime newTime(y,m,d,h,min,s,wd);  // make variable to store the time 
	rtc.setDateTime(newTime); 							// adjust date-time as defined 'newTime' above 
}

void timeDisplay() // display the time and date on the LCD screen 
{

  DateTime now = rtc.now(); //get the current date-time
  uint32_t ts = now.getEpoch();
  
  if (old_ts == 0 || old_ts != ts) {
	  old_ts = ts;
	   if (alarm.hour == now.hour() && alarm.minute == now.minute() && alarm.second == now.second() )
	   {alarmGoOff();}
	  if(now.second() == 0) // if we went to the next miute ... 
		  lcd.clear(); 		// clear screen 
		  
	  // print the current date:
	  lcd.setCursor(0,0);lcd.print("Date  : "); lcd.print(now.month(), DEC);lcd.print('/');lcd.print(now.date(), DEC);lcd.print('/');lcd.print(now.year(), DEC);
	  // print the current time:
	  lcd.setCursor(0,1);lcd.print("Time  : "); lcd.print(now.hour(), DEC);lcd.print(':');lcd.print(now.minute(), DEC);lcd.print(':');lcd.print(now.second(), DEC);
	  // print the day of the week:
	  lcd.setCursor(0,2);lcd.print("Day   : "); lcd.print(weekDay[now.dayOfWeek()]);
	  // print the day of the week:
	  lcd.setCursor(0,3);lcd.print("Alarm : "); lcd.print(alarm.hour); lcd.print(":");lcd.print(alarm.minute);lcd.print(':');lcd.print(alarm.second);
  }
  delay(50);
}


void displayTemp()
{ 
    hum = dht.readHumidity();
    temp= dht.readTemperature();
	rtc.convertTemperature();             //convert current temperature into registers
    //Print temp and humidity values to lcd
    lcd.print("Humidity: ");lcd.print(hum);lcd.print(" %");
    lcd.setCursor(0,1);lcd.print("Temp:     ");lcd.print((temp*9/5) + 32);lcd.print(" F");
    lcd.setCursor(0,2);lcd.print("Sensor 1: ");lcd.print(temp);lcd.print(" C"); 
	lcd.setCursor(0,3);lcd.print("Sensor 2: ");lcd.print(rtc.getTemperature());lcd.print(" C");  //read registers and display the temperature
    lcd.setCursor(0,0);         // set the cursor to the previous line on our LCD ( line 1 )
}

void decodeRemote()
{
  
 if(results.value == btn_1) // pressed 1
	{
	lcd.setBacklight(HIGH);	
    screenOn = true;
	previousMillis = millis(); 
	while(screenOn)
	{
		screenTimer();
		timeDisplay();
	}
	}	
 else if(results.value == btn_2) // pressed 2
    {
	lcd.setBacklight(HIGH);	
    screenOn = true;
	previousMillis = millis(); 
	while(screenOn)
	{
		screenTimer();
		displayTemp();
	}
	}	
 else if(results.value == btn_3) // pressed 3
	  {
		  alarmGoOff();
	  }
 else if(results.value == btn_4) // pressed 4
 {
	 
 }
 else if(results.value == btn_5) // pressed 5
 {
 
 }
 else if(results.value == btn_6) // pressed 5
      {
		alarmGoOff();  
      }
 else if(results.value == btn_minus) // pressed - 
	{  
	 screenOn = !screenOn; // turn screen from on-> off or off -> on 
     if(screenOn == true)
	 {
		previousMillis = millis(); 
        lcd.setBacklight(HIGH);
	 }
     else
        lcd.setBacklight(LOW);
  }
  else if(results.value == btn_plus) // pressed + 
     lcd.setBacklight(HIGH);    // for now always set high 
     

}

void getRemoteInput()
{
   if (irrecv.decode(&results)) 
   {
	   lcd.clear();    				   // clear screen 
	   // print out the code . . . then delay to find out the values, DEBUG ONLY 
	   //lcd.print(results.value,HEX); delay(10000);
	   decodeRemote();  // decode output 
	   irrecv.resume(); // Receive the next value
    }
}




void setup()
{ 
  Wire.begin(); rtc.begin();
  dht.begin();                                 // set up temp monitor 
  alarmOn(); alarmSet(6,0,0); 				   // default alarm at 6 am 
  pinMode(BUZZERPIN,OUTPUT);				   // set buzzer as output 
  screenOn = false;							   // when starting, make LED backlight off 
  lcd.begin (20,4,LCD_5x8DOTS);                // set up LCD  
  lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE); // set up backlight pin
  lcd.setBacklight(LOW);  lcd.home ();        // turn off LED screen, and set to home 
  while (!Serial);                             //delay for serial
  irrecv.enableIRIn();                         // Start the receiver
  Serial.begin(9600);                          // Begin serial communcation, used for input 
  analogWrite(BUZZERPIN,50);delay(50);analogWrite(BUZZERPIN,0); // indicate to user that we  are ready 
}

void loop()
{
  getRemoteInput();
  screenTimer();
 // timeDisplay();
}

