#include <Wire.h>                  // This library allows you to communicate with I2C
#include <Adafruit_GFX.h>          // Adafruit GFX graphics core library
#include <Adafruit_SSD1306.h>      // Driver library for 'monochrome' 128x64 and 128x32 OLEDs
#include "SD.h" //Lib to read SD card
#include "TMRpcm.h" //Lib to play auido
#include "SPI.h" //SPI lib for SD card

//Define the PINS you're goint to use on your Arduino Nano
#define PIN_SCE   7
#define PIN_RESET 6
#define PIN_DC    5
#define PIN_SDIN  4
#define PIN_SCLK  3

#define SD_Chip   1

#define LCD_C     LOW
#define LCD_D     HIGH

#define LCD_X     84
#define LCD_Y     6

//Aesthetic Modifications
int barWidth = 16;
int barHeight = 4;
int ballPerimeter = 4;
TMRpcm music; //Lib object is named "music"

// Define variables
int song_number=0;
boolean debounce1=true;
boolean debounce2=true;
boolean play_pause;

unsigned int bar1X = 0;
unsigned int bar1Y = 0;
unsigned int bar2X = 0;
unsigned int bar2Y = LCD_Y * 8 - barHeight;
int ballX = 0;
int ballY = 0;
boolean isBallUp = false;
boolean isBallRight = true;

byte pixels[LCD_X][LCD_Y];
unsigned long lastRefreshTime;
const int refreshInterval = 150;
byte gameState = 1;
byte ballSpeed = 2;
byte player1WinCount = 0;
byte player2WinCount = 0;
byte hitCount = 0;

#define OLED_RESET 4

Adafruit_SSD1306 display(OLED_RESET);
    
#if (SSD1306_LCDHEIGHT != 64)
  #error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif


void setup() {
   //Serial.begin(9600);
  LcdInitialize();
  restartGame();
}

//Audio Setup
{
music.speakerPin = 10; //Audio out on pin 10
Serial.begin(9600); //Serial Com for debugging 
if (!SD.begin(SD_ChipSelectPin)) {
Serial.println("SD fail");
return;
}

pinMode(8, INPUT_PULLUP); //Button 1 with internal pull up to change track
pinMode(9, INPUT_PULLUP); //Button 2 with internal pull up to play/pause
pinMode(9, INPUT_PULLUP); //Button 2 with internal pull up to fast forward

music.setVolume(5);    //   0 to 7. Set volume level
music.quality(1);        //  Set 1 for 2x oversampling Set 0 for normal
//music.volume(0);        //   1(up) or 0(down) to control volume
//music.play("filename",30); plays a file starting at 30 seconds into the track    
}

void loop() {
    unsigned long now = millis();
  if(now - lastRefreshTime > refreshInterval){
      update();
      refreshScreen();
      lastRefreshTime = now;  
  } 
{ 
  
  if (digitalRead(2)==LOW  && debounce1 == true) //Button 1 Pressed
  {
  song_number++;
  if (song_number==5)
  {song_number=1;}
  debounce1=false;
  Serial.println("KEY PRESSED");
  Serial.print("song_number=");
  Serial.println(song_number);

  if (song_number ==1)
  {music.play("1.wav",10);} //Play song 1 from 10th second 

  if (song_number ==2)
  {music.play("2.wav",33);} //Play song 2 from 33rd second 

  if (song_number ==3)
  {music.play("3.wav");} //Play song 3 from start

  if (song_number ==4)
  {music.play("4.wav",25);} //Play song 4 from 25th second 

  if (digitalRead(3)==LOW  && debounce2 == true) //Button 2 Pressed
  {
  music.pause();  Serial.println("PLAY / PAUSE");
  debounce2=false;
  }

  if (digitalRead(2)==HIGH) //Avoid debounce
  debounce1=true;

  if (digitalRead(3)==HIGH)//Avoid debounce
  debounce2=true;
}
}
}

void restartGame(){
   ballSpeed = 1;
   gameState = 1;
   ballX = random(0, 60);
   ballY = 20;
   isBallUp = false; 
   isBallRight = true;
   hitCount = 0;
}

void refreshScreen(){
  if(gameState == 1){
    for(int y = 0; y < LCD_Y; y++){
        for(int x = 0; x < LCD_X; x++){
           byte pixel = 0x00;
           int realY = y * 8;
           // draw ball if in the frame
           if(x >= ballX && x <= ballX + ballPerimeter -1 && ballY + ballPerimeter > realY && ballY < realY + 8 ){
             byte ballMask = 0x00;
             for(int i = 0; i < realY + 8 - ballY; i++){
               ballMask = ballMask >> 1;
               if(i < ballPerimeter)
                 ballMask = 0x80 | ballMask;
             }
             pixel = pixel | ballMask;
           }
           
           // draw bars if in the frame
           if(x >= bar1X && x <= bar1X + barWidth -1 && bar1Y + barHeight > realY && bar1Y < realY + 8 ){
             byte barMask = 0x00;
             for(int i = 0; i < realY + 8 - bar1Y; i++){
               barMask = barMask >> 1;
               if(i < barHeight)
                 barMask = 0x80 | barMask;
             }
             pixel = pixel | barMask;
           }
           
           if(x >= bar2X && x <= bar2X + barWidth -1 && bar2Y + barHeight > realY && bar2Y < realY + 8 ){
             byte barMask = 0x00;
             for(int i = 0; i < realY + 8 - bar2Y; i++){
               barMask = barMask >> 1;
               if(i < barHeight)
                 barMask = 0x80 | barMask;
             }
             pixel = pixel | barMask;
           }
           LcdWrite(LCD_D, pixel);
         }
    }
  } else if(gameState == 2){
      
  }
}

void update(){
  if(gameState == 1){
     int barMargin = LCD_X - barWidth;
     int pot1 = analogRead(A0); //read potentiometers and set the bar positions
     int pot2 = analogRead(A1);
     bar1X = pot1 / 2 * LCD_X / 512;
     bar2X = pot2 / 2 * LCD_X / 512;
    
     if(bar1X > barMargin) bar1X = barMargin;
     if(bar2X > barMargin) bar2X = barMargin;
     
     //move the ball now
     if(isBallUp)
       ballY -= ballSpeed;
     else
       ballY += ballSpeed;
     if(isBallRight)
       ballX += ballSpeed;
     else
       ballX -= ballSpeed;
     //check collisions  
     if(ballX < 1){
       isBallRight = true;
       ballX = 0;
     }
     else if(ballX > LCD_X - ballPerimeter - 1){
       isBallRight = false;
       ballX = LCD_X - ballPerimeter;
     }
     if(ballY < barHeight){
       if(ballX + ballPerimeter >= bar1X && ballX <= bar1X + barWidth){ // ball bounces from bar1
         isBallUp = false;
         if(ballX + ballPerimeter/2 < bar1X + barWidth/2)
           isBallRight = false;
         else
           isBallRight = true;
         ballY = barHeight;
         if(++hitCount % 10 == 0 && ballSpeed < 5) 
           ballSpeed++;
       }else{ //player2 wins
         gameState = 2;
         player2WinCount++;
       }
     }
     if(ballY + ballPerimeter > LCD_Y * 8 - barHeight){
       if(ballX + ballPerimeter >= bar2X && ballX <= bar2X + barWidth){ //ball bounces from bar2
         isBallUp = true; 
         if(ballX + ballPerimeter/2 < bar2X + barWidth/2)
           isBallRight = false;
         else
           isBallRight = true;
         ballY = LCD_Y * 8 - barHeight - ballPerimeter;
         if(++hitCount % 10 == 0 && ballSpeed < 5) 
           ballSpeed++;
       }else{ // player 1 wins
         gameState = 2;
         player1WinCount++;
       }
     }
  }else if(gameState == 2){
      for(int i =0; i < 4; i++){
        LcdWrite(LCD_C, 0x0D );  // LCD in inverse mode.
        delay(300);
        LcdWrite(LCD_C, 0x0C );  // LCD in inverse mode.
        delay(300);
      }
      restartGame();
  }  
}

void LcdInitialise(void){
  pinMode(PIN_SCE, OUTPUT);
  pinMode(PIN_RESET, OUTPUT);
  pinMode(PIN_DC, OUTPUT);
  pinMode(PIN_SDIN, OUTPUT);
  pinMode(PIN_SCLK, OUTPUT);
  delay(200);
  digitalWrite(PIN_RESET, LOW);
  delay(500);
  digitalWrite(PIN_RESET, HIGH);
  LcdWrite(LCD_C, 0x21 );  // LCD Extended Commands.
  LcdWrite(LCD_C, 0xB1 );  // Set LCD Vop (Contrast). 
  LcdWrite(LCD_C, 0x04 );  // Set Temp coefficent. //0x04
  LcdWrite(LCD_C, 0x14 );  // LCD bias mode 1:48. //0x13
  LcdWrite(LCD_C, 0x0C );  // LCD in normal mode.
  LcdWrite(LCD_C, 0x20 );
  LcdWrite(LCD_C, 0x80 ); //select X Address 0 of the LCD Ram
  LcdWrite(LCD_C, 0x40 ); //select Y Address 0 of the LCD Ram - Reset is not working for some reason, so I had to set these addresses
  LcdWrite(LCD_C, 0x0C );
}

void LcdWrite(byte dc, byte data){
  digitalWrite(PIN_DC, dc);
  digitalWrite(PIN_SCE, LOW);
  shiftOut(PIN_SDIN, PIN_SCLK, MSBFIRST, data);
  digitalWrite(PIN_SCE, HIGH);
}
