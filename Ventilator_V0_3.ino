/*---------------------------------------------------------------------------------------
* Ventilator motion control
* Date:2020/04/18
* Version:v0.3
* File: Ventilator
* Note: Ventilator motion control
* Author:PIN(shihkw@gmail.com);CANDY(610008@hjgs.cy.edu.tw)
----------------------------------------------------------------------------------------*/

/*--------------------------------------------------------
 * 
 * History:
 * V0.3 Addition Run switch and home sensor
 * 
 ---------------------------------------------------------*/
/*--------------------------------------------------------
 * Hardware arduino uno 
 * 
 * 
 * PIN 2  -----> TB6560 DIR PIN 
 * PIN 3  -----> TB6560 CLK PIN 
 * PIN 4  -----> TB6560 ENABLE PIN 
 * 
 * PIN 5  <----- HOME SENSOR PIN 
 * PIN 6  <----- RUN SWITCH  PIN 
 * 
 * 
 * PIN A0 <----- BPM Potentiometer :BreathsPerSecond 每次數
 * PIN A1 <----- VOL Potentiometer :Percent of Volume容量 每次吸氣的量百分比
 * PIN A2 <----- SPD Potentiometer :SPEED 速度,壓力    
 * 
 * 
 * PIN A4  -----> IIC LCD SDA
 * PIN A5  -----> IIC LCD SCL
 * 
 *--------------------------------------------------------*/
 
/*--------------------------------------------------------
*    Includes
*---------------------------------------------------------*/

 #include <AccelStepper.h>
 #include <LiquidCrystal_I2C.h> 
 #include <Wire.h> 

 
/*--------------------------------------------------------
*     Define
*---------------------------------------------------------*/
// Hardware
 #define dirPin       3    // stepmoto dir pin
 #define stepPin      2    // stepmoto clk pin
 #define enPin        4    // stepmoto     enable pin
 #define runPin       6    // run          enable pin
 #define homePin      5    // home sensor  enable pin
 

// Stepmoto parameter 
 #define dEnHomeSensor          1          // Home sensot 1 enable有裝SENSOR / 0 disable沒裝SENSOR
 #define dHomeStep              10         // 在有HOME SENSOR的情況下回程要多讓馬達多走几步以強制讓馬達啟動HOME動作,做位置校正
 #define motorInterfaceType     1          // stepmoto enable pin (AccelStepper motor type)
                                           // tb6560 use 1

 #define dMaxStep               1000       // 最大步數:最大的吸氣量
 #define dRateOfInhaleExhale    40         // 吸氣時間比例%  吸氣佔個呼吸週期的時間
                                           // (吸氣時間 / ( 吸氣時間+ 呼氣時間)) * 100
 #define dStepMotoAccel         1000        //  stepmoto 加減速的速度

 
/*--------------------------------------------------------
*     global variables and constants
*---------------------------------------------------------*/
 AccelStepper stepper = AccelStepper(motorInterfaceType, stepPin, dirPin);
 LiquidCrystal_I2C lcd(0x27,16,2);                //設定LCD位置0x27,設定LCD大小為16*2

 


/**
  * @brief Lcd display.
  * @param[RUN]  1:RUN / 0:STOP.
  * @param[BPM]  SHOW BPM.
  * @param[VOL]  SHOW VOL.
  * @param[SPD]  SHOW SPEED.
  * @return None
  * @details This function is show data on lcd.
  */
  
void lcdDisp(int isRun,int BPM,int VOL ,int SPD){
  //lcd.clear();
  lcd.setCursor(8,0); // 設定游標位置在第一行行首
  lcd.print("BPM:  ");   // 每秒次數
  lcd.setCursor(1,1); // 容量 每次吸氣的量百分比
  lcd.print("VOL:  ");
  lcd.setCursor(8,1); // 速度,壓力
  lcd.print("SPD:  ");



  lcd.setCursor(12,0); // 
  lcd.print(BPM);
  lcd.setCursor(5,1); // 
  lcd.print(VOL);
  lcd.setCursor(12,1); // 
  lcd.print(SPD);
  
   
}


/**
  * @brief Step motor control .
  * @param[SETST] motor status  0:Go home then stop.
  *                             1:RUN.
  *               工作狀態
  *                              0:停止工作並回到home
  *                              1:開始工作   *                             
  * @param[BPM]  BPM:BreathsPerSecond 每秒次數
  * @param[VOL]  Percent of Volume容量 每次吸氣的量百分比
  * @param[SPD]  SPEED 速度,壓力
  * @return status 
  * @details This function control Stepmoto.
  */
int moto_action(int SETST ,int BPM,int VOL,int SPD){

  static int            st = 0;
  static unsigned long  StartMillis ; //    這次開始的時間
  static unsigned long  LoopMillis  ; //    這次所需的時間
  static unsigned long  MidMillis   ; //    這次吸氣結束時間
  static unsigned long  EndMillis   ; //    這次結束的時間(吸氣+呼氣)
  static int            MotoStep    ; //    這次要走的步數:容量:MaxStep / VOl  

  static int            testi;        //    test
    
  if (SETST == 0 ) {  //  如果SETST = 0 設定馬達回到HOME
    if (st > 2 ) st = 0 ;
  }
         
  MotoStep    = (dMaxStep * VOL) / 100  ;
  stepper.setMaxSpeed(SPD*10);
  switch(st){
    case 0:   //init
      if (digitalRead(runPin) == 0  ) st = 1;       //    RUN SWITCH ON 
      // st = 1;
      break;
    case 1:    // home
      stepper.moveTo((0-dMaxStep) - dHomeStep );
      if ((stepper.distanceToGo()==0)||((dEnHomeSensor==1)&&(digitalRead(homePin)==0))){
        stepper.setCurrentPosition(0);
        st = 2;
      }
      
      break;
    case 2:    // idle
      st = 3;             
      break;
    case 3:   // start breathing loop 
        StartMillis = millis();                     //    這次開始的時間
        LoopMillis  = 60000 /  BPM ;                //    每個週期有几個ms
        EndMillis   = StartMillis + LoopMillis ;    //    這次週期結束的時間
        MidMillis   = StartMillis + ((LoopMillis * dRateOfInhaleExhale )/100  ) ;  //    這次吸氣結束時間
        st=4;
    case 4:    //floward  吸氣
      stepper.moveTo(MotoStep);
      if (stepper.distanceToGo() == 0) { st = 5; }
      break;
    case 5:    //waiting for MidMillis
      if (millis() > MidMillis) { st = 6;}
      break;
    case 6:    //back  呼氣
      if (dEnHomeSensor==1) {
        stepper.moveTo(0-dHomeStep);
      }else {
        stepper.moveTo(0);
      }
      if ((stepper.distanceToGo() == 0)||((dEnHomeSensor==1)&&(digitalRead(homePin)==0)))
      { 
        stepper.setCurrentPosition(0);
        stepper.moveTo(0);
        st = 7;
       }
      break;
    case 7:    //waiting for EndMillis
      if (millis() > EndMillis) { st = 3;}
      break;     
    default:
      st = 3 ;
  }  
  
  stepper.run();
/*  
  testi++;
  if ( testi++ > 800 ) {
      testi = 0 ;
      Serial.print(st);
      Serial.print("m");
      Serial.print(millis());
      Serial.print(" S");
      Serial.print(StartMillis);
      Serial.print(" M");
      Serial.print(MidMillis);
      Serial.print(" E");
      Serial.print(EndMillis);
      Serial.print(" L");
      Serial.println(LoopMillis);
  }
  */
  return st;
}


void setup() {
        // init PIN
   pinMode( dirPin, OUTPUT);
   pinMode(  enPin, OUTPUT);
   pinMode(stepPin, OUTPUT);
   pinMode( runPin, INPUT);
   pinMode(homePin, INPUT);
   digitalWrite(runPin, HIGH); 
   digitalWrite(homePin, HIGH); 


        // init serial port not necessary;Just for debugging
  Serial.begin(9600);

        // init lcd display
  lcd.init(); //初始化LCD 
  lcd.backlight(); //開啟背光
  lcd.clear();
}

void loop() {  
  static int showdelay = 0 ;
  
  int sBPM      = map(analogRead(A0),0,1023,5,40);//      BPM:BreathsPerSecond 每次數
  int sVOL      = map(analogRead(A1),0,1023,5,90);//      VOL:Percent of Volume容量 每次吸氣的量百分比
  int sSPD      = map(analogRead(A2),0,1023,1 ,99);//      SPD:SPEED 速度,壓力
  
  stepper.setAcceleration(dStepMotoAccel);     //設定加速度 

  if (digitalRead(runPin)==0) {
    moto_action(1,sBPM,sVOL,sSPD);               //馬達控制RUN
  }else {
    moto_action(0,sBPM,sVOL,sSPD);               //馬達控制STOP
  }
  showdelay ++ ;
  if (showdelay > 500){
    showdelay = 0 ;
    lcdDisp(1,sBPM,sVOL,sSPD);
 //   Serial.println(stepper.currentPosition ()); //test    
  }  
}
 
