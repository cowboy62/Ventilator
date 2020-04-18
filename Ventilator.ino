 #include <AccelStepper.h>
 #include <LiquidCrystal_I2C.h> 
 #include <Wire.h> 

 LiquidCrystal_I2C lcd(0x27,16,2);  //設定LCD位置0x27,設定LCD大小為16*2
 
 #define dirPin 3
 #define stepPin 2
 #define enPin 4
 
 #define motorInterfaceType 1

 #define dMaxStep               2000       // MaxStep  最大步數:最大的吸氣量
 #define dRateOfInhaleExhale    40 

 
 AccelStepper stepper = AccelStepper(1, 2, 3);
 int previous=0;
 int val=0;
void setup() {
  Serial.begin(9600);
    stepper.setMaxSpeed(8000);
    stepper.move(20);
  lcd.init(); //初始化LCD 
  lcd.backlight(); //開啟背光
  lcd.print("Hello, World!"); //顯示Hello, World!
  lcd.clear();
}
void lcdDisp(int isRun,int BPM,int VOL ,int SPD){
  //lcd.clear();
  lcd.setCursor(8,0); // 設定游標位置在第一行行首
  lcd.print("BPM:   ");   // 每秒次數
  lcd.setCursor(1,1); // 容量 每次吸氣的量百分比
  lcd.print("VOL:   ");
  lcd.setCursor(8,1); // 速度,壓力
  lcd.print("SPD:   ");



  lcd.setCursor(12,0); // 設定游標位置
  lcd.print(BPM);
  lcd.setCursor(5,1); // 設定游標位置
  lcd.print(VOL);
  lcd.setCursor(12,1); // 設定游標位置
  lcd.print(SPD);
 // lcd.setCursor(1,0); // 設定游標位置
  //lcd.print(millis());
  
   
}

//                  
int moto_action(int SETST ,int BPM,int VOL,int SPD){
//                setst   工作狀態
//                   0:停止工作並回到home
//                   1:開始工作 
//      BPM:BreathsPerSecond 每秒次數
//      VOL:Percent of Volume容量 每次吸氣的量百分比
//      SPD:SPEED 速度,壓力

  static int st = 0;
  static unsigned long  StartMillis ; //    這次開始的時間
  static unsigned long  LoopMillis  ; //    這次所需的時間
  static unsigned long  MidMillis   ; //    這次吸氣結束時間
  static unsigned long  EndMillis   ; //    這次結束的時間(吸氣+呼氣)
  static int            MotoStep ;           //這次要走的步數:容量:MaxStep / VOl  

  static int            testi;
    
  if (SETST == 0 ) {  //  如果SETST = 0 設定馬達回到HOME
    if (st > 2 ) st = 0 ;
  }
         
  switch(st){
    case 0:   //init
      st = 1;
      break;
    case 1:    // home
      st = 2;
      break;
    case 2:    // idle
     // if (start key in ) st = 3;
      st = 3;
      break;
    case 3:   // start breathing loop 
      MotoStep    = (dMaxStep * VOL) / 100  ;
      stepper.setMaxSpeed(SPD*10);
        StartMillis = millis(); //    這次開始的時間
        LoopMillis  = 60000 /  BPM ;
        EndMillis   = StartMillis + LoopMillis ;    //    這次結束的時間
        MidMillis   = StartMillis + ((LoopMillis * dRateOfInhaleExhale )/100  ) ;  //    這次吸氣結束時間
        st=4;
    case 4:    //floward
      stepper.moveTo(MotoStep);
      if (stepper.distanceToGo() == 0) { st = 5; }
      break;
    case 5:    //waiting for MidMillis
      if (millis() > MidMillis) { st = 6;}
      break;
    case 6:    //back
      //stepper.moveTo(-100);
      stepper.moveTo(0);
      if (stepper.distanceToGo() == 0) { st = 7;}
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
}




void loop() {
  static int showdelay = 0 ;
  
  int sBPM      = map(analogRead(A0),0,1023,5,40);//      BPM:BreathsPerSecond 每次數
  int sVOL      = map(analogRead(A1),0,1023,5,90);//      VOL:Percent of Volume容量 每次吸氣的量百分比
  int sSPD      = map(analogRead(A2),0,1023,1 ,99);//      SPD:SPEED 速度,壓力

  stepper.setAcceleration(600);     //設定加速度

  moto_action(1,sBPM,sVOL,sSPD);


  showdelay ++ ;
  if (showdelay > 500){
    showdelay = 0 ;
    lcdDisp(1,sBPM,sVOL,sSPD);
 //   Serial.println(9999);
 //   Serial.println(val);
 //   Serial.println(stepper.currentPosition ()); 
   
  }
  
}
 
