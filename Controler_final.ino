#include "DHT_U.h"                                             //DHT sensor Library
#include <LiquidCrystal_I2C.h>                                 //LCD Library 
#include <TimerOne.h>
#define DHTPIN 6                                               //Define sensor pin(pin 6)
#define DHTTYPE DHT11                                          //Type of sensor used declared.

DHT dht(DHTPIN, DHTTYPE);//Create sensor object
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); //Create lcd object using these pins

boolean a = LOW, b = HIGH;
boolean on = HIGH, off = LOW;
boolean boost;
boolean shower;
boolean fullBoost_flag;
boolean read_flag;
boolean lcdOnFlag;
float t = 0;
int h = 0;
float set_point;     //RH set_point
const byte interruptPin2 = 2;
const byte interruptPin3 = 3;
long previousMillis = 0;
long interval = 6000;        //Read sensor each 6 seconds
long backlighttime = 30000;  //lcd backlight on timer in mSeconds
long preMillis = 0;
int  rh_sample_flag = 0;
int  fbs = 0;
byte relay1 = 8;
byte relay2 = 9;
byte relay3 = 10;
byte relay4 = 11;
const byte fire = 12;
long button_relese_time;



void setup(void) {

  //Serial.begin(9600);         // for debugging and monitoring
  lcd.begin(16, 2);             //Start lcd
  lcd.setCursor(0, 0);          //Set in position 0,0
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(interruptPin2, INPUT_PULLUP);
  pinMode(interruptPin3, INPUT_PULLUP);
  pinMode(4, OUTPUT);          // Shower mode LED
  pinMode(5, OUTPUT);          // Normal mode LED
  pinMode(7, OUTPUT);          // Boost mode  LED
  pinMode(relay1, OUTPUT);     // Relay 1 - extracting fan hi/low
  pinMode(relay4, OUTPUT);     // Relay 4 - extracting fan ON/OFF
  pinMode(relay3, OUTPUT);     // Relay 3 - Supplying fan ON/OFF
  pinMode(relay2, OUTPUT);     // Relay 2 - Supplying fan hi/low
  pinMode(fire, INPUT_PULLUP); // fire signal input
  attachInterrupt(digitalPinToInterrupt(interruptPin2), lcd_on, FALLING);
  attachInterrupt(digitalPinToInterrupt(interruptPin3), fullBoostOn, FALLING);
  digitalWrite(5, LOW);
  digitalWrite(4, LOW);
  digitalWrite(7, LOW);
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, HIGH);
  digitalWrite(relay4, HIGH);
  dht.begin();                   //Start DHT11 sensor
  lcd.begin(16, 2);              //Start lcd
  lcd.setCursor(0, 0);           //Set in position 0,0
  lcd.print("T=");
  lcd.setCursor(0, 1);           //Change cursor position
  lcd.print("H=");
  set_point = 60;                //was
  Timer1.initialize(6000000);
  Timer1.attachInterrupt(set_flag);
  read_flag = false;
  fullBoost_flag = false;
  button_relese_time = 0;
  digitalWrite(LED_BUILTIN, LOW);
  shower = false;
  boost = false;

}




void loop() {

  int main1;
  main1 = normal_mode();
  int main2;
  main2 = main_loop();
  int main3;
  main3 = lcd_autoOff();


}






//################################### Functions listed below ###############################################

int normal_mode() {

  if (rh_sample_flag >= 0 && rh_sample_flag < 50 && fullBoost_flag == false && shower == false && digitalRead(fire) != LOW && h < set_point )  {  //50 * 6sec = 300sec => 5 mins

    fbs = 0;
    analogWrite(5, 5);                            //blue led ON at 5% PWM
    digitalWrite(4, LOW);                         // red led off
    digitalWrite(7, LOW);
    digitalWrite(relay3, LOW);
    digitalWrite(relay4, HIGH);
    digitalWrite(relay2, LOW);
    digitalWrite(relay1, HIGH);
    lcd.setCursor(6, 1);
    lcd.print(" Normal ON");

  }
  else if (rh_sample_flag >= 50 && rh_sample_flag < 100 &&  fullBoost_flag == false && shower == false && digitalRead(fire) != LOW && h < set_point) {

    fbs = 0;
    analogWrite(5, 5);                             //blue LED ON at 5% PWM
    digitalWrite(4, LOW);
    digitalWrite(7, LOW);
    digitalWrite(relay3, HIGH);
    digitalWrite(relay4, HIGH);
    digitalWrite(relay2, HIGH);
    digitalWrite(relay1, HIGH);
    lcd.setCursor(6, 1);
    lcd.print("Normal OFF");
    //set_point = 75;

  }
  else if (rh_sample_flag >= 100 && fullBoost_flag == false && shower == false && digitalRead(fire) != LOW) {

    (rh_sample_flag = 0);

  }

}



int main_loop() {


  if (digitalRead(fire) == LOW)                      // disable all fans on fire signal
  {
    digitalWrite(5, HIGH);
    digitalWrite(4, HIGH);
    digitalWrite(7, HIGH);
    delay(400);
    digitalWrite(5, LOW);
    digitalWrite(4, LOW);
    digitalWrite(7, LOW);
    digitalWrite(relay1, HIGH);
    digitalWrite(relay2, HIGH);
    digitalWrite(relay3, HIGH);
    digitalWrite(relay4, HIGH);
    lcd.setCursor(6, 1);
    lcd.setBacklight(on);
    lcd.print("Fire!!!!!!");
    shower = false;
    boost = false;


  }


  if (read_flag == true && digitalRead(fire) != LOW) {
    h = dht.readHumidity();                          //read RH
    t = dht.readTemperature();                       //read Temp

    set_point = (-3.5 * t) + 130;                    // Linear relation bewtween temp and set_point

    digitalWrite(LED_BUILTIN, LOW);
    lcd.setCursor(2, 0);
    lcd.print(t);
    lcd.setCursor(6, 0);                              //writes over second decimal point to get rid of it
    lcd.print("C");
    lcd.setCursor(2, 1);
    lcd.print(h);
    lcd.print("%");
    lcd.setCursor(8, 0);
    lcd.print("set=");
    lcd.print(set_point);
    read_flag = false;

  }

  if (h > set_point && fullBoost_flag == false && digitalRead(fire) != LOW) //if humidity is above the set_point and pin a is LOW
  {
    fbs = 0;
    shower = true;
    digitalWrite(4, HIGH);                           //Red led
    digitalWrite(5, LOW);                            //blue led
    digitalWrite(7, LOW);
    digitalWrite(relay1, LOW);                       //extract fan hi speed
    digitalWrite(relay2, LOW);                       //extract fan ON/OFF - LOW = ON
    digitalWrite(relay3, LOW);                       // Supply fan ON/OFF - LOW = ON
    digitalWrite(relay4, HIGH);                      // Supply fan low speed
    //set_point= 60;
    lcd.setCursor(6, 1);
    lcd.print(" Shower ON");

  }
  else if (h < set_point && fullBoost_flag == false && shower == true && digitalRead(fire) != LOW)
  {
    digitalWrite(5, LOW);
    digitalWrite(4, LOW);
    digitalWrite(7, LOW);
    digitalWrite(relay1, HIGH);
    digitalWrite(relay2, HIGH);
    digitalWrite(relay3, HIGH);
    digitalWrite(relay4, HIGH);
    rh_sample_flag = 0;                               //reset sampling flag for the normal mode
    shower = false;

  }

  if (fullBoost_flag == true && fbs <= 300 && digitalRead(fire) != LOW) {

    digitalWrite(4, LOW);
    digitalWrite(5, LOW);
    digitalWrite(7, HIGH);
    digitalWrite(relay1, LOW);
    digitalWrite(relay2, LOW);
    digitalWrite(relay3, LOW);
    digitalWrite(relay4, LOW);
    lcd.setCursor(5, 1);
    lcd.print(" Boost ");
    lcd.print((1800 - (fbs * 6)) / 60);
    lcd.print("min");

  }

  else if (fbs > 300 && digitalRead(fire) != LOW) {

    digitalWrite(4, LOW);
    digitalWrite(5, LOW);
    digitalWrite(7, LOW);
    digitalWrite(relay1, HIGH);
    digitalWrite(relay2, HIGH);
    digitalWrite(relay3, HIGH);
    digitalWrite(relay4, HIGH);
    fbs = 0;
    fullBoost_flag = false;
  }

  if (digitalRead(3) == LOW && fullBoost_flag == true) {  //Boost off when button pressed for longer

    lcd.setBacklight(on);
    button_relese_time = 0;
    while (digitalRead(3) == LOW) {
      button_relese_time ++;
      delay(10);
      if (button_relese_time >= 100) {

        fullBoost_flag = false;
        fbs = 0;
      }
    }
  }
}

//############# other functions ##################

int lcd_autoOff() {

  unsigned long currentbcklight = millis();       //time elapsed
  if (currentbcklight - preMillis > backlighttime) //Comparison between the elapsed time and the time in which the action is to be executed
  {
    preMillis = currentbcklight;                   //Last time is now
    lcd.setBacklight(off);
  }
  else if (lcdOnFlag == true) {
    preMillis = millis();                        //Keeps the interval at the same length
    lcd.setBacklight(on);
    lcdOnFlag = false;
  }

}

int set_flag()
{
  read_flag  = true;
  rh_sample_flag  ++;
  fbs ++;
  digitalWrite(LED_BUILTIN, HIGH);
}

int lcd_on() {
  lcdOnFlag = true;

}
//setting flag for full boost mode

int fullBoostOn()
{
  fullBoost_flag = true;
  fbs = 0;

}
