#include <Wire.h>
#include <RTClib.h>
#include <Adafruit_NeoPixel.h>
#include <avr/power.h>
#include <avr/sleep.h>

#define NeoPin 3
#define CLOCK_INTERRUPT_PIN 2

#define BVal 20
//#define MainColor 51, 153, 255, 0 //50, 150, 250, 0 //0, 0, 0, 255
//#define AlertColor 190, 0, 0, 0
//#define BackColor 0, 0,  0, 0

RTC_DS3231 rtc;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(182, NeoPin, NEO_GRB + NEO_KHZ800);

static uint32_t Color(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

static uint32_t BackColor = Color(0, 0, 0);
static uint32_t MainColor = Color(80, 100, 255); //Color(60, 140, 255);
static uint32_t AlrtColor = Color(190, 0, 0);  //Color(200, 0, 0);
static uint32_t GoodColor = Color(0, 200, 0);
static uint32_t WarnColor = Color(255, 80, 40);
static uint32_t MoonColor = Color(70, 70, 30);
static uint32_t EtoiColor = Color(60, 60, 60);
static uint32_t EtinColor = Color(20, 20, 20);

int Hour,Minute,LastDigit;
bool Demie;

//13H,14V:
//[I][L][T][E][S][T][F][M][I][N][U][I][T]   [00][01][02][03][04][05][06][07][08][09][10][11][12]
//[U][N][E][S][E][P][T][R][O][I][S][I][X]   [25][24][23][22][21][20][19][18][17][16][15][14][13]
//[C][I][N][Q][U][A][T][R][E][D][E][U][X]   [26][27][28][29][30][31][32][33][34][35][36][37][38]
//[H][U][I][T][N][E][U][F][K][O][N][Z][E]   [51][50][49][48][47][46][45][44][43][42][41][40][39]
//[D][I][X][M][I][D][I][H][E][U][R][E][S]   [52][53][54][55][56][57][58][59][60][61][62][63][64]
//[E][T][M][O][I][N][S][T][R][E][N][T][E]   [77][76][75][74][73][72][71][70][69][68][67][66][65]
//[L][E][G][Q][U][A][R][T][V][I][N][G][T]   [78][79][80][81][82][83][84][85][86][87][88][89][90]
//[O][N][Z][E][C][I][N][Q][U][A][N][T][E]   [03][02][01][00][99][98][97][96][95][94][93][92][91]
//[D][E][M][I][E][Q][U][A][R][A][N][T][E]   [04][05][06][07][08][09][10][11][12][13][14][15][16]
//[D][O][U][Z][E][S][E][I][Z][E][D][I][X]   [29][28][27][26][25][24][23][22][21][20][19][18][17]
//[Q][U][A][T][O][R][Z][E][T][R][O][I][S]   [30][31][32][33][34][35][36][37][38][39][40][41][42]
//[Q][U][A][T][R][E][I][Z][E][D][E][U][X]   [55][54][53][52][51][50][49][48][47][46][45][44][43]
//[E][T][L][U][N][E][U][F][C][S][E][P][T]   [56][57][58][59][60][61][62][63][64][65][66][67][68]
//[H][U][I][T][R][T][C][H][O][K][S][I][X]   [81][80][79][78][77][76][75][74][73][72][71][70][69]

byte MoonMatrix[61] = { 32,33,34,42,43,44,59,60,61,62,67,68,
                        69,86,87,88,89,92,93,94,95,96,102,105,
                        106,107,110,111,112,113,114,115,119,120,121,122,
                        123,124,125,126,127,128,132,133,134,135,136,137,
                        138,139,140,146,147,148,149,150,151,152,161,162,163
                      };

byte Etoiles[5] = { 27,38,56,168,180 };

byte Etincelle[17] = { 13,24,26,28,37,39,47,50,55,57,73,143,157,167,169,179,181 };

byte mots[40][2] = {

  {0,1}, // #0 / IL

  {23,25}, // #1 / UNE
  {35,38}, // #2 / DEUX
  {15,19}, // #3 / TROIS
  {29,34}, // #4 / QUATRE
  {26,29}, // #5 / CINQ
  {13,15}, // #6 / SIX
  {19,22}, // #7 / SEPT
  {48,51}, // #8 / HUIT
  {44,47}, // #9 / NEUF
  {52,54}, // #10 / DIX
  {39,42}, // #11 / ONZE

  {55,58}, // #12 / MIDI
  {7,12}, // #13 / MINUIT
  
  {59,64}, // #14 / HEURE
  {3,5}, // #15 / EST

  {76,77}, // #16 / ET
  {71,75}, // #17 / MOINS

  {78,79}, // #18 / LE
  {81,85}, // #19 / QUART

  {156,161}, // #20 / ET UNE
  {143,146}, // #21 / DEUX
  {138,142}, // #22 / TROIS
  {150,155}, // #23 / QUATRE
  {96,99}, // #24 / CINQ
  {169,171}, // #25 / SIX
  {165,168}, // #26 / SEPT
  {178,181}, // #27 / HUIT
  {160,163}, // #28 / NEUF
  //=======
  {117,119}, // #29 / DIX
  {100,103}, // #30 / ONZE
  {125,129}, // #31 / DOUZE
  {147,152}, // #32 / TREIZE
  {130,137}, // #33 / QUATORZE
  // LE QUART (# et #)
  {120,124}, // #34 / SEIZE
  // DIX... (# et #,#,#)
  {86,90}, // #35 / VINGT
  {65,70}, // #36 / TRENTE
  {109,116}, // #37 / QUARANTE
  // MOINS LE QUART (# # #)
  {91,99}, // #38 / CINQUANTE
  // MOINS DIX (# #)
  // MOINS CINQ (# #)
  {104,107} // #39 / DEMI(E)

};

void setup() {

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  pixels.begin();
  pixels.setBrightness(BVal);
  for (int i=0; i<=181; i++) {
    pixels.setPixelColor(i,MainColor);
  }
  pixels.show();

  delay(500);
  
  if(!rtc.begin()) {
    pixels.setPixelColor(177,255,0,0);
    pixels.setPixelColor(176,255,0,0);
    pixels.setPixelColor(175,255,0,0);
    pixels.show();
    while (1) delay(10);
  }

  // if(1) {
  //   rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  //   rtc.adjust(DateTime(rtc.now().year(), rtc.now().month(), rtc.now().day(), rtc.now().hour(), rtc.now().minute(), (rtc.now().second()+9)));
  //   Serial.begin(9600);
  //   Serial.print("il est: ");
  //   Serial.print(rtc.now().hour());
  //   Serial.print(":");
  //   Serial.print(rtc.now().minute());
  //   Serial.print(":");
  //   Serial.println(rtc.now().second());
  // } 

  if(rtc.lostPower()) {
    pixels.setPixelColor(177,WarnColor);
    pixels.setPixelColor(176,WarnColor);
    pixels.setPixelColor(175,WarnColor);
    pixels.show();
    while (1) delay(10);
  }else {
  }

  rtc.disable32K();

  pinMode(CLOCK_INTERRUPT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN), onAlarm, FALLING);

  rtc.clearAlarm(1);
  rtc.clearAlarm(2);

  rtc.writeSqwPinMode(DS3231_OFF);

  if(!rtc.setAlarm1(DateTime(0,0,0,0,0,0), DS3231_A1_Minute)) {/*Keep on minute mode @ 0mins, thus every hour*/
    while (1) delay(10);
    }else {}

  if(!rtc.setAlarm2(DateTime(0,0,0,0,0,0), DS3231_A2_PerMinute)) {/*Keep on minute mode, once every minute*/
    while (1) delay(10);
    }else {}

    pixels.setPixelColor(177,GoodColor);
    pixels.setPixelColor(176,GoodColor);
    pixels.setPixelColor(175,GoodColor);
    pixels.setPixelColor(173,GoodColor);
    pixels.setPixelColor(172,GoodColor);
    pixels.show();

    //ARC-EN-CIEL pour 2.5 secondes

    delay(1500);

    //IL est
    RstHours();
    RstMinutes();
    pixels.setPixelColor(0,MainColor);
    pixels.setPixelColor(1,MainColor);
    pixels.setPixelColor(2,BackColor);
    pixels.setPixelColor(3,MainColor);
    pixels.setPixelColor(4,MainColor);
    pixels.setPixelColor(5,MainColor);

    SetHrs(0);
    SetMin();
    pixels.show();

    delay(500);
}

void loop() {

  if (rtc.alarmFired(1)) {
    rtc.clearAlarm(1);
    //Hour Changed
    RstHours();
    //=====================
    SetHrs(0);
    pixels.show();
  }

  if (rtc.alarmFired(2)) {
    rtc.clearAlarm(2);
    //MinuteChanged
    RstMinutes();
    //=======================
    SetMin();
  }

  pixels.show();

  delay(100);

  sleep_mode();

}

void onAlarm() {
  sleep_disable();
}

void RstMinutes() {
  for(int i = 65; i <= 181; i++){
      pixels.setPixelColor(i,BackColor);
    }
  if (rtc.now().hour()>=0&&rtc.now().hour()<2) {
    PaintMoon();
  }
}

void RstHours() {
  for(int i = 6; i <= 64; i++){
    pixels.setPixelColor(i,BackColor);
  }
  if (rtc.now().hour()>=0&&rtc.now().hour()<2) {
    PaintMoon();
  }
}

void PaintMoon(){
  for (int i = 0; i<sizeof(MoonMatrix); i++) {
    pixels.setPixelColor(MoonMatrix[i],MoonColor);
  }
  for (int i = 0; i<sizeof(Etoiles); i++) {
    pixels.setPixelColor(Etoiles[i],EtoiColor);
  }
  for (int i = 0; i<sizeof(Etincelle); i++) {
    pixels.setPixelColor(Etincelle[i],EtinColor);
  }
}

void SetHrs(int offset){
    Hour = ((rtc.now().hour() + offset) == 24) ? 0 : (rtc.now().hour() + offset) ;
    if (Hour == 0) {
      Demie = 0;
      for (int i = mots[13][0]; i <= mots[13][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
      PaintMoon();
    } else if (Hour == 12) {
      Demie = 0;
      for (int i = mots[12][0]; i <= mots[12][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
    } else {              
      Demie = 1;
      Hour = Hour > 12 ? Hour - 12 : Hour ;
        for (int i = mots[14][0]; i <= mots[14][1]; i++) {
          pixels.setPixelColor(i,MainColor);
        }
    }

    if (Hour == 1) {
      for (int i = mots[1][0]; i <= mots[1][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
      pixels.setPixelColor(64,BackColor);
    } else if (Hour == 2) {
      for (int i = mots[2][0]; i <= mots[2][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
    } else if (Hour == 3) {
      for (int i = mots[3][0]; i <= mots[3][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
    } else if (Hour == 4) {
      for (int i = mots[4][0]; i <= mots[4][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
    } else if (Hour == 5) {
      for (int i = mots[5][0]; i <= mots[5][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
    } else if (Hour == 6) {
      for (int i = mots[6][0]; i <= mots[6][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
    } else if (Hour == 7) {
      for (int i = mots[7][0]; i <= mots[7][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
    } else if (Hour == 8) {
      for (int i = mots[8][0]; i <= mots[8][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
    } else if (Hour == 9) {
      for (int i = mots[9][0]; i <= mots[9][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
    } else if (Hour == 10) {
      for (int i = mots[10][0]; i <= mots[10][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
    } else if (Hour == 11) {
      for (int i = mots[11][0]; i <= mots[11][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
    }
}

void SetMin(){
    Minute = rtc.now().minute();
      if (Minute == 45||Minute == 50||Minute == 55) {
        RstHours();
        SetHrs(1);
      } else if (Minute == 46||Minute == 51||Minute == 56){
        RstHours();
        SetHrs(0);
      }
    LastDigit = 0;
    if (Minute == 11) {
      for (int i = mots[30][0]; i <= mots[30][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
    } else if (Minute == 12) {
      for (int i = mots[31][0]; i <= mots[31][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
    } else if (Minute == 13) {
      for (int i = mots[32][0]; i <= mots[32][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
    } else if (Minute == 14) {
      for (int i = mots[33][0]; i <= mots[33][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
    } else if (Minute == 15) {
      for (int i = mots[16][0]; i <= mots[16][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
      for (int i = mots[19][0]; i <= mots[19][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
    } else if (Minute == 16) {
      for (int i = mots[34][0]; i <= mots[34][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
    } else if (Minute == 30) {
      for (int i = mots[16][0]; i <= mots[16][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
      for (int i = mots[39][0]; i <= mots[39][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
      if (Demie) {
        pixels.setPixelColor(108,MainColor);
      }
    } else if (Minute == 45) {
      for (int i = mots[17][0]; i <= mots[17][1]; i++) {
        pixels.setPixelColor(i,AlrtColor);
      }
      for (int i = mots[18][0]; i <= mots[18][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
      for (int i = mots[19][0]; i <= mots[19][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
    } else if (Minute == 50) {
      for (int i = mots[17][0]; i <= mots[17][1]; i++) {
        pixels.setPixelColor(i,AlrtColor);
      }
      for (int i = mots[29][0]; i <= mots[29][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
    } else if (Minute == 55) {
      for (int i = mots[17][0]; i <= mots[17][1]; i++) {
        pixels.setPixelColor(i,AlrtColor);
      }
      for (int i = mots[24][0]; i <= mots[24][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
    } else if(Minute<10){
      LastDigit = Minute;
    } else if(Minute>=10&&Minute<20){
      LastDigit = Minute - 10;
      for (int i = mots[29][0]; i <= mots[29][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
    } else if(Minute>=20&&Minute<30){
      LastDigit = Minute - 20;
      for (int i = mots[35][0]; i <= mots[35][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
    } else if(Minute>=30&&Minute<40){
      LastDigit = Minute - 30;
      for (int i = mots[36][0]; i <= mots[36][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
    } else if(Minute>=40&&Minute<50){
      LastDigit = Minute - 40;
      for (int i = mots[37][0]; i <= mots[37][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
    } else if(Minute>=50&&Minute<60){
      LastDigit = Minute - 50;
      for (int i = mots[38][0]; i <= mots[38][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
    }
    //Last Digit (If necessary)
    switch (LastDigit) {
    case 1:
      for (int i = mots[20][0]; i <= mots[20][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
        pixels.setPixelColor(158,BackColor);
    break;
    case 2:
      for (int i = mots[21][0]; i <= mots[21][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
    break;
    case 3:
      for (int i = mots[22][0]; i <= mots[22][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
    break;
    case 4:
      for (int i = mots[23][0]; i <= mots[23][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
    break;
    case 5:
      for (int i = mots[24][0]; i <= mots[24][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
    break;
    case 6:
      for (int i = mots[25][0]; i <= mots[25][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
    break;
    case 7:
      for (int i = mots[26][0]; i <= mots[26][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
    break;
    case 8:
      for (int i = mots[27][0]; i <= mots[27][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
    break;
    case 9:
      for (int i = mots[28][0]; i <= mots[28][1]; i++) {
        pixels.setPixelColor(i,MainColor);
      }
    break;
    default:
    break;
    }
}