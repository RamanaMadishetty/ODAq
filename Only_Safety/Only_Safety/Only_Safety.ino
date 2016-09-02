
// include the library code:
#include <LiquidCrystal.h>
#include <EEPROM.h>

#include <Event.h>
#include <Timer.h>


// initialize the library with the numbers of the interface pins
/*
 * The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
 */
 // Serial Port 1 is used for RF Transmitter
 /*
  * TX1 - Pin18
  * RX1 - Pin19
  */
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
 /*
  *           *****Digital Pins 23 to 37 used for sensors reading******
  * Digital Pin23 |   Digital Pin27 |  Digital Pin31 |  Digital Pin25 |  Digital Pin35   |  Digital Pin29 |  Digital Pin33 | Digital Pin37
  * Low-water     |   Stove-sensor  |  Steam-High    |  Stack-High    |  Pressure-Switch |  Blowdown      |  Fill/Fire     | PowerStatus
  */
const int WaterSensor = 23; const int StackTemperature = 25; const int StoveSensor = 27; const int Blowdown = 29; 
const int SteamTemperature = 31; const int FillMode = 33; const int PressureSwitch = 35; const int PowerStatus = 37;

short int gPOWER_STATUS_Flag; // flag to check main POWER supply availibity 
short int gSTOVE_flag;        // flag to check the stove inside or outside
short int gPUMP_flag;         // flag to count the timer for PUMP OFF delay
short int key;                // flag to check the on-board switch
short int gLIMIT_EXCEED_flag;
char abnormal;
unsigned short int counter_flag, modeValue;     // Counter to set the values in key press
Timer t;

int PumpRelay,counterValue;
char timerCount;



/*
 *            *****Digital Pins 39 & 41,43 used to Digital write - Relays ****
 * Digital Pin 39 | Digital Pin 41      | Digital Pin 43
 * HOOTER ON      | Water Feed Pump ON  | Buzzer
 */
const int HOOTER = 39; const int WaterPump = 41; const int Buzzer = 43;

/*
 *           *****Digital Pins 45,47,49 used to Buttons ******
 * Digital Pin 45 | Digital Pin 47 | Digital Pin 49
 * Menu Btton     | Increment      | Decrement
 */
const int MENU_BUTTON = 45; const int INCREMENT_BUTTON = 47; const int DECREMENT_BUTTON = 49; 

/*
 *  Check Switches functions checks the on-board switch buttons
 */
 
void check_switches(){
  if(counter_flag>=2){
    char count[3];
    String dispCount=dtostrf(counterValue,3,0,count);
    lcd.setCursor(12, 0);
    lcd.print(dispCount); // Printing counter value on LCD
  }
  if(counter_flag<2){
    if(counterValue>2){
      counterValue=0;
    }
  }
  if(digitalRead(MENU_BUTTON)==LOW){
     delay(50);
     if(digitalRead(MENU_BUTTON)==LOW){
      key=1; //lcd.clear(); 
     }
  }
  else if(digitalRead(INCREMENT_BUTTON)==LOW){
    if(key!=2){
     delay(50);  
     if(digitalRead(INCREMENT_BUTTON)==LOW){
      key=2; }
    }
   }
    else if(digitalRead(DECREMENT_BUTTON)==LOW){
      if(key!=3){
      delay(50);
      if(digitalRead(DECREMENT_BUTTON)==LOW){
        key=3; }
      }
    }
   else
  { key=0;}
  if(key==1)
  {
    delay(50);
    if(digitalRead(DECREMENT_BUTTON)==LOW)
    {key=4;}
  }
  if(key==1)
  {
    delay(50);
    if(digitalRead(INCREMENT_BUTTON)==LOW)
    {key=5;}
  }
 }

/*
 *  Function: process_switch(), porcess the value for the switch pressed on-board
 */
void process_switch(){
  switch(key){
    case 1: counter_flag++;
            key=0;
            switch(counter_flag){
              case 1: counterValue = modeValue;
                      lcd.clear();
                      lcd.setCursor(0, 0);
                      lcd.print("Mode Settings");
                      break;
              case 2: counterValue = EEPROM.read(1); // storing PUMP off delay time in EEPROM @ location 1
                      lcd.clear();
                      lcd.setCursor(0, 0);
                      lcd.print("Feed Pump ");
                      break;
              case 3: EEPROM.write(1,counterValue); // Write counter value in EEPROM 
                      timerCount = counterValue;
                      counter_flag=0;
                      lcd.setCursor(0, 1);
                      lcd.print("Processing...");
                      delay(1000);
                      lcd.clear();
                      break;
            }
     case 2: delay(50); 
             if(counterValue>=100)counterValue=0;
             else counterValue++;
             break;
     case 3: delay(50);
             if(counterValue<=0)counterValue=100;
             else counterValue--;
             break;
     case 4: counterValue = counterValue-10;
             delay(50);
             break;
     case 5: counterValue = counterValue+10;
             delay(50);
             break;    
  }
  
}

void stopFeedPump(){
  digitalWrite(WaterPump,LOW);
  t.stop(PumpRelay);
}

/*
 * Setup pins and port peripherals
 */
void setup() {
  // set up the LCD's number of columns and rows:  
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.setCursor(2, 0);
  lcd.print("Welcome to");
  lcd.setCursor(4, 1);
  lcd.print("Oorja!!");
  delay(3000);
  lcd.clear();
  lcd.setCursor(1, 0);
  delay(10);
  lcd.print("Initializing..");
  delay(2000);
  Serial.begin(9600);
  Serial1.begin(9600);
  
  // Set up Sensors as a Digital Input
  pinMode(WaterSensor, INPUT);      // sets the digital pin 22 as input
  pinMode(StoveSensor, INPUT);      // sets the digital pin 23 as input
  pinMode(SteamTemperature, INPUT); // sets the digital pin 24 as input
  pinMode(StackTemperature, INPUT); // sets the digital pin 25 as input
  pinMode(PressureSwitch, INPUT);   // sets the digital pin 26 as input
  pinMode(Blowdown, INPUT);         // sets the digital pin 27 as input
  pinMode(FillMode, INPUT);         // sets the digital pin 28 as input
  pinMode(PowerStatus, INPUT);      // sets the digital pin 29 as input

  // Set up Relays as a Digital Output
  pinMode(HOOTER, OUTPUT);    // Sets the digital pin 30 as output
  pinMode(WaterPump, OUTPUT); // Sets the digital pin 31 as output
  pinMode(Buzzer, OUTPUT);    // Sets the digital pin 32 as output

  // Set up on-board Switches as a Digital Input
  pinMode(MENU_BUTTON, INPUT);         // sets the digital pin 45 as input 
  pinMode(INCREMENT_BUTTON, INPUT);    // sets the digital pin 47 as input 
  pinMode(DECREMENT_BUTTON, INPUT);    // sets the digital pin 49 as input 

  lcd.clear();
  lcd.setCursor(0, 0);
  //delay(10);
  lcd.print("Checking");
  lcd.setCursor(0,1);
  lcd.print("Parameters...");
  delay(2000);
  lcd.clear(); 
  // Check Sensors readings - if healthy condition --> Activate Stove and Water Feed Pump
  if((digitalRead(StackTemperature)==HIGH)&&(digitalRead(WaterSensor)==HIGH)&&(digitalRead(PowerStatus)==HIGH)&&(digitalRead(SteamTemperature)==HIGH))
  {
     digitalWrite(WaterPump,HIGH);
     digitalWrite(HOOTER,LOW); 
     Serial.println("Checking Parameters...");
     Serial.println("in Limit - Activate Stove");
     Serial1.print("&ACT&ACT&ACT&ACT&ACT&ACT&ACT&ACT&ACT&ACT&ACT&ACT&ACT&ACT&ACT&ACT&ACT&ACT");
     lcd.setCursor(10, 1);  
     lcd.print("Normal");
  }
  timerCount = EEPROM.read(1);
}

void loop() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):

  //lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  //lcd.print(millis() / 1000);
  t.update();
  if(digitalRead(FillMode)==HIGH){
    if((digitalRead(StackTemperature)==LOW)||(digitalRead(WaterSensor)==LOW)||(digitalRead(PowerStatus)==LOW)||(digitalRead(SteamTemperature)==LOW)){
    delay(50);
    if((digitalRead(StackTemperature)==LOW)||(digitalRead(WaterSensor)==LOW)||(digitalRead(PowerStatus)==LOW)||(digitalRead(SteamTemperature)==LOW)){
        Serial1.print("&DEACT&DEACT&DEACT&DEACT&DEACT&DEACT&DEACT&DEACT&DEACT&DEACT&DEACT&DEACT&DEACT");
        digitalWrite(HOOTER,HIGH); 
     //   Serial.print("Timer Count is:");
     //   Serial.println((int)timerCount);
        PumpRelay  = t.after(timerCount*60000,stopFeedPump);
        if(digitalRead(PowerStatus)==LOW){
        Serial.println("NO POWER");
        lcd.clear();
        lcd.setCursor(0, 0);  
        lcd.print("NO POWER");
        } 
        // Once the BOILER Parameter conditions are abnormal, below condition excute only once's
        if(abnormal==0)    
        {
          Serial.println("Abnormal");
          abnormal=1;
          if(digitalRead(StackTemperature)==LOW){
          Serial.println("Stack Temperature High"); 
          lcd.clear();  
          lcd.setCursor(0, 0);
          lcd.print("STACK TEMP HIGH");
          }
          if(digitalRead(WaterSensor)==LOW){
          Serial.println("LOW WATER"); 
          lcd.clear();  
          lcd.setCursor(0, 0);
          lcd.print("LOW WATER");
          }
          if(digitalRead(SteamTemperature)==LOW){
          Serial.println("Steam Temp High"); 
          lcd.clear(); 
          lcd.setCursor(0, 0);
          lcd.print("STEAM TEMP HIGH");
          }
        }
      }
    }    
  // BUZZER ON when no power supply and stove still inside
     if((digitalRead(PowerStatus)==LOW) && (digitalRead(StoveSensor)==LOW))
     {   
       digitalWrite(Buzzer,HIGH); 
       gPOWER_STATUS_Flag=1;
     }
      
  // when No power status and stove is removed --> then BUZZER off
     if((digitalRead(PowerStatus)==LOW) && (digitalRead(StoveSensor)==HIGH))
     {   
       digitalWrite(Buzzer,LOW);
       gPOWER_STATUS_Flag=0;
     }
     
  // Deactivate Hooter when Power available
     if((digitalRead(PowerStatus)==HIGH) && (gPOWER_STATUS_Flag==1)) 
     {   
       digitalWrite(Buzzer,LOW);
       gPOWER_STATUS_Flag=0;
     }

 // Checking all sensors parameters condition - If healthy - system run
     if((digitalRead(StackTemperature)==HIGH)&&(digitalRead(WaterSensor)==HIGH)&&(digitalRead(PowerStatus)==HIGH)&&(digitalRead(SteamTemperature)==HIGH)
         &&(digitalRead(Blowdown)==HIGH)&&(digitalRead(StoveSensor)==HIGH))
     {
      t.stop(PumpRelay);
      abnormal=0;
      digitalWrite(WaterPump,HIGH); 
      digitalWrite(HOOTER,LOW); 
      gLIMIT_EXCEED_flag=0;
      Serial1.print("&ACT&ACT&ACT&ACT&ACT&ACT&ACT&ACT&ACT&ACT&ACT&ACT&ACT&ACT&ACT&ACT&ACT&ACT");
      //lcd.clear(); 
      /*
       * Below *if condition is to scroll the string on LCD
       * when the system is in Normal Condition
       */
      if(counter_flag==0){ 
        lcd.clear();   
        lcd.setCursor(2,0);
        lcd.print("BPC-[NORMAL]");
        lcd.setCursor(10,1);
        lcd.print("Menu >");
      }
     }
     
// checking stove outside
     if(digitalRead(StoveSensor)==LOW)
     {
      delay(50);
      if(digitalRead(StoveSensor)==LOW){
        lcd.clear(); 
        lcd.setCursor(1, 0);
        lcd.print("STOVE REMOVED");
        gSTOVE_flag=1;
      }
     }
// Check blowdown valve open 
    if(digitalRead(Blowdown)==LOW)
    {
      delay(50);
      if(digitalRead(Blowdown)==LOW)
      {
        digitalWrite(HOOTER,HIGH);
        digitalWrite(WaterPump,LOW);
        lcd.clear(); 
        lcd.setCursor(1, 0);
        lcd.print("BLOW DOWN OPEN");
        Serial1.print("&DEACT&DEACT&DEACT&DEACT&DEACT&DEACT&DEACT&DEACT&DEACT&DEACT&DEACT&DEACT&DEACT");
      }
    }
    
// If all sensors are out of limit or Stove remopved outside --> Pump oFF's after a delay
    if((gLIMIT_EXCEED_flag=1)||(gSTOVE_flag=1))
    {
      gPUMP_flag=1;
    }
    check_switches();
    if(key!=0)
    {
      process_switch(); 
    } 
 }
 
 // Fill mode manualy operation -  Continous stove ON and PUMP ON (Used for blowdown)
 if(digitalRead(FillMode)==LOW){
  digitalWrite(HOOTER,LOW); 
  lcd.clear();  
  lcd.setCursor(0, 0);
  lcd.print("FILL MODE!!");
  Serial1.print("&ACT&ACT&ACT&ACT&ACT&ACT&ACT&ACT&ACT&ACT&ACT&ACT&ACT&ACT&ACT&ACT&ACT&ACT");
 }
}

