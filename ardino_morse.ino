/*************************************************************************************************************************
 * 
 * "ALL CAPS SHOUT OUT WITH LOTS OF EXCLAMATION MARKS!!!!!!!!!!!!"
 * IF YOU ARE READING THIS AND YOU HAVE INTERNET CODING SKILLS...
 * PLEASE HELP WRITE SOME PROGRAM OR APP TO HELP READ EMAILS AND WEBSITES IN MORSE CODE!!!
 * IF YOU KNOW SOMEONE WHO MAY KNOW HOW TO DO THIS, PLEASE PASS THIS REQUEST TO THEM!!!
 * WHAT A THRILL TO GET BREAKING NEWS UPDATES IN MORSE CODE.
 * (apologies for all the yelling)
 * 
 * 
 * This Arduino sketch reads text files from a MicroSD card while it sounds and displays the file in morse code.
 * When the end of one file is reached, the next file will be read.
 * At the end of the last file, it will cycle to the first file.
 * Each time the button is pressed, it will advance thru the files on the MicroSD card.
 * 
 * The characters will scroll across the top line of the LCD module.
 * The dots and dashes will scroll across the second line of the LCD module.
 * 
 * The Morse speed (WPM) and pitch (in hertz) of tone are set values in this sketch.
 *          ....There are ways to change these values on the fly....
 *          A small modification can be done to change WPM and pitch tone while the program is running 
 *          by installing a potentiometer on one of the Analog pins and adding a few lines of code.
 *          However, that is beyond the scope of the simplicity of this sketch.
 *          
 * 
 * This program should work fine without the I2C LCD module as simple 'no display, only sound' system. 
 * 
 * The Serial.print lines help to monitor the output on a computer screen.
 * 
 * I can not take credit for the open source nature of this sketch.
 * 
 * Bucky Muttel
 * Send feedback thru www.Muttel.com 
 * 
 **********************************************************************************************************************/

/******************************************************************************************
 *                      ARDUINO NANO                                                      *
 *                    ________________                                                    *
 *                   |    |      |    |                                                   *
 * MicroSD SCK pin---|D13 |      | D12|---MicroSD MISO pin                                *
 *                   |3V3 |______| D11|---MicroSD MOSI pin                                *
 *                   |REF    /\    D10|---MicroSD CS pin                                  *
 *                   |A0    /  \    D9|                                                   *
 *                   |A1   /    \   D8|                                                   *
 *                   |A2   \    /   D7|                                                   *
 *                   |A3    \  /    D6|---toggle button (the button grounds this pin)     *
 * I2C LCD SDA pin---|A4     \/     D5|                                                   *
 * I2C LCD SLC pin---|A5  ________  D4|                                                   *
 *                   |A6 [________] D3|---Speaker (to sound Morse) thru optional Vol Knob *
 *                   |A7  . . . .   D2|                                                   *
 *  +5V to modules---|5V  * * * *  GND|---GROUND to modules, button, speaker etc...       *
 *                   |RST          RST|                                                   *
 *       GROUND in---|GND  o o o   RX0|                                                   *
 *    +5V power in---|VIN  o o o   TX1|                                                   *
 *                   |________________|                                                   *
 *                                                                                        *
 *****************************************************************************************/

/****************************************************************************
 *  MicroSD Read program adapted from an example code in the public domain. *
 *  created   Nov 2010                                                      *
 *  by David A. Mellis                                                      *
 *  modified 9 Apr 2012                                                     *
 *  by Tom Igoe                                                             *
 *                                                                          *
 *                                                                          *
 *                                                                          *
 *   MicroSD Card Module           ______________________________           *
 *                                |              _______________ |          *
 *   Arduino Nano pin D10    grey-|CS           |               ||          *
 *   Arduino Nano pin D13    blue-|SCK          |Micro          ||          *
 *   Arduino Nano pin D11  yellow-|MOSI         |SD            / |          *
 *   Arduino Nano pin D12  orange-|MISO         |Card          \ |          *
 *   Arduino Nano +5V         red-|VCC          |Adapter        ||          *
 *   Arduino Nano GND       green-|GND          |_______________||          *
 *                                |______________________________|          *
 *                                                                          *
 ***************************************************************************/
#include <SPI.h>
#include <SD.h>
File root ;
File entry ;
int count = 1;
unsigned long EndOfFile;

/**********************************************************************************************************************
 * I2C LCD DISPLAY                                     ___________________________________________________________    *
 *                                                    |                                                           |   *
 *   Arduino Nano ground  green-----------------------|GND                                                        |   *
 *   Arduino Nano +5V       red-----------------------|+VCC      LIQUID CRYSTAL DISPLAY                           |   *
 *   Arduino Nano pin A4 orange-----------------------|SDA                                                        |   *
 *   Arduino Nano pin A5   grey-----------------------|SCL                                                        |   *
 *                                                    |___________________________________________________________|   *
 *                                                                                                                    *
 *                                                                                                                    *
 *  The I2C LCD Module only requires two data pins as opposed to the standard LCD module                              *
 *********************************************************************************************************************/
//NOTE <LiquidCrystal_I2C.h> MAY NOT WORK WITH SOME SKETCHES...probably with some sketches that require <Wire.h> library but I'm not so sure.
//#include <Wire.h>//this is not needed for the LiquidCrystal_I2C library any more since its upgrade version 1.1.1 (earlier versions of <LiquidCrystal_I2C.h> require <Wire.h> library)
/***********************************************************************************************************************************
 * 
 * MOST IMPORTANT TO ADD <LiquidCrystal_I2C> TO LIBRARY FOR THIS SKETCH TO COMPILE PROPERLY
 * to add <LiquidCrystal_I2C.h> to library, follow these steps from Sketch menu...
 * 
 *        Sketch>
 *               Include Library>
 *                              Manage Libraries... (wait for it to load, then search LiquidCrystal_I2C) 
 *                                                  (scroll down to find LiquidCrystal I2C by Marco Schwartz Version 1.1.1) 
 *                                                        (install)
 * 
 **************************************************************************************************************************************/
#include <LiquidCrystal_I2C.h>            //NOTE <LiquidCrystal_I2C.h> MAY NOT WORK WITH SOME SKETCHES...probably with some sketches that require <Wire.h> library but I'm not so sure.
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2) ;  //16 character, two line module
char TpLnArray[17]= "SD_Morse_Vr1.001" ;  // initial Top line of the LCD 
char BtLnArray[17]= "                " ;  // initial Bottom line of the LCD                                  

/**************************************************************************************************
 * The Morse sounding portion of this sketch was adapted from a Mark VandeWettering K6HX sketch
 * 
 * https://brainwagon.org/2009/11/14/another-try-at-an-arduino-based-morse-beacon/comment-page-1/
 *
 * Simple Arduino Morse Beacon
 * Written by Mark VandeWettering K6HX
 * Email: k6hx@arrl.net
 * 
 * This code is so trivial that I'm releasing it completely without 
 * restrictions.  If you find it useful, it would be nice if you dropped
 * me an email, maybe plugged my blog @ https://brainwagon.org or included
 * a brief acknowledgement in whatever derivative you create, but that's
 * just a courtesy.  Feel free to do whatever.
 *
 * Josh asked me how the morse code table works. It’s a little bit clever 
 * (a very little bit) but I guess it does require some explanation. Morse 
 * code characters are all length six or less, and each element is either 
 * a dot or a dash, so it would seem that we can store the pattern in six 
 * bits. Let’s say that dits are zero and dahs are one. Lets store them so 
 * the first element gets stored in the least significant bit, and the next 
 * in the second most, and so on. The only trick is knowing when there are 
 * no elements left, because otherwise we can’t tell (for example) K (-.-) 
 * from C (-.-.) To do that, we store a single extra one after all the 
 * other elements are taken care of. Then, when we are looping, we do the 
 * following. If the pattern is equal to one, we are done (that’s our 
 * guard bit). If not, we look at the least significant digit. If it is 
 * a zero, we have a dit, if we have a one, it’s a dah. We then get rid of 
 * that element (by dividing by two, or shifting right if that floats your 
 * boat) and repeat. Voila. Each character takes only a single byte to 
 * store its pattern, and decoding is just done in a few instructions.
 * 
 * the line while (p != 1) means while p in not equal to 1
 * the line if (p & 1) means if the LSB (least significant bit...the right most bit) is equal to 1
 *  
 * for (int i=0; i<N_MORSE; i++) {                          
 *   if (morsetab[i].c == c) {                              
 *     unsigned char p = morsetab[i].pat ;                 
 *     while (p != 1) {                                    //this line while (p != 1) means while p is not equal to 1
 *         if (p & 1) DitOrDah (DASHLEN,'_') ;             //this line if (p & 1) means if the LSB (least significant bit...the right most bit) is equal to 1
 *         else       DitOrDah (DOTLEN,'.') ;              //this line else means if the LSB (least significant bit...the right most bit) is not equal to 1 
 *         p = p/2 ;                                       //this line p = p/2 divides p by 2 effectively shifting all the bits one to the right
 *     }                                                   
 *     TimeDelay(2*DOTLEN) ;                               
 *     return ;              
 *   }
 * }
 * 
 *     from the t_mtab array below {'Q', 27}, //B00011011 
 * 
 *     character     decimal pattern      binary pattern
 *        "Q"              27         =      00011011
 *        
 *          00011011          00011011          00011011          00011011          00011011
 *                 ^                ^                ^                ^                ^
 *        first element    second element    third element       fourth element    end marker
 *            1=dash            1=dash           0=dot                1=dash        the "1" furthest to the left
 *     
 *     
 *     each character has a one byte number to represent its morse code pattern 
 *     an example for the character 'Q' (morse: dah,dah,dit,dah) is shown below:
 *     within the for loop, when c == 'Q', unsigned char p == 27 .......so in binary form, p==00011011 
 *     p==00011011 first while loop: the right most digit is '1' (dah) then all bits shift to right (the right most digit '1' is removed)
 *     p==00001101 second while loop: the right most digit is '1' (dah) then all bits shift to right (the right most digit '1' is removed)
 *     p==00000110 third while loop: the right most digit is '0' (dit) then all bits shift to right (the right most digit '0' is removed)
 *     p==00000011 fourth while loop:  the right most digit is '1' (dah) then all bits shift to right (the right most digit '1' is removed)
 *     p==00000001 this number now is equal to one and the process is complete 
 *     
 **************************************************************************************************/
 
struct t_mtab { char c, pat; } ;  // The Morse sounding portion of this sketch was adapted from a Mark VandeWettering K6HX sketch
struct t_mtab morsetab[] = {      // These 42 characters are the most common Morse code characters also known as KOCH characters
  {'A', 6},  //B00000110
  {'B', 17}, //B00010001
  {'C', 21}, //B00010101
  {'D', 9},  //B00001001
  {'E', 2},  //B00000010
  {'F', 20}, //B00010100
  {'G', 11}, //B00001011
  {'H', 16}, //B00010000
  {'I', 4},  //B00000100
  {'J', 30}, //B00011110
  {'K', 13}, //B00001101
  {'L', 18}, //B00010010
  {'M', 7},  //B00000111
  {'N', 5},  //BOOOOO1O1
  {'O', 15}, //B00001111
  {'P', 22}, //B00010110
  {'Q', 27}, //B00011011 
  {'R', 10}, //B00001010
  {'S', 8},  //B00001000
  {'T', 3},  //B00000011
  {'U', 12}, //B00001100
  {'V', 24}, //B00011000
  {'W', 14}, //B00001110
  {'X', 25}, //B00011001
  {'Y', 29}, //B00011101
  {'Z', 19}, //B00010011
  {'1', 62}, //B00111110
  {'2', 60}, //BOO111100
  {'3', 56}, //B00111000
  {'4', 48}, //B00110000
  {'5', 32}, //B00100000
  {'6', 33}, //B00100001
  {'7', 35}, //B00100011
  {'8', 39}, //B00100111
  {'9', 47}, //B00101111
  {'0', 63}, //B00111111
  {'.', 106},//B01101010
  {',', 115},//B01110011
  {'?', 76}, //B01001100
  {'=', 49}, //B00110001
  {'/', 41}, //B00101001
  {'@', 86}, //B01010110
} ;

#define N_MORSE  (sizeof(morsetab)/sizeof(morsetab[0]))
    
int SPEED  (20);              // WPM words per minute...change this to meet your taste...or better yet, figure some way to change this value on the fly.
int DOTLEN  (1200/SPEED);     // Duration of DIT
int DASHLEN  (3*(1200/SPEED));// Duration of DAH

#define Sound_Pin 3           // This is the speaker pin
#define Button_Pin 6          // This is the Select button pin
int Hrtz = 700 ;              // Tone of the beeps...change this to meet your taste...or better yet, figure some way to change this value on the fly.
bool SpaceFlg = 0;            // off=0 on=1

/************************************************************ 
 *   SSSSSSSS   EEEEEEEE   TTTTTTTT   UU    UU   PPPPPPPP   *
 *   SS         EE            TT      UU    UU   PP    PP   *
 *   SS         EE            TT      UU    UU   PP    PP   *
 *   SSSSSSSS   EEEEEEEE      TT      UU    UU   PPPPPPPP   *
 *         SS   EE            TT      UU    UU   PP         *
 *         SS   EE            TT      UU    UU   PP         *
 *   SSSSSSSS   EEEEEEEE      TT      UUUUUUUU   PP         *
 ************************************************************/
 void setup() {
  pinMode(Button_Pin, INPUT_PULLUP);
  pinMode(Sound_Pin, OUTPUT);  //Strangly enough, this line seems to be optional
  
  Serial.begin(9600);// Open serial communications and wait for port to open:           // MicroSD Read program adapted from an example code in the public domain.
  while (!Serial);  // wait for serial port to connect. Needed for native USB port only // MicroSD Read program adapted from an example code in the public domain.
  Serial.print("Initializing SD card...");                                              // MicroSD Read program adapted from an example code in the public domain.
  if (!SD.begin(10)) {                                                                  // MicroSD Read program adapted from an example code in the public domain.
    Serial.println("initialization failed!");                                           // MicroSD Read program adapted from an example code in the public domain.
    while (1);                                                                          // MicroSD Read program adapted from an example code in the public domain.
  }                                                                                     // MicroSD Read program adapted from an example code in the public domain.
  Serial.println("initialization done.");                                               // MicroSD Read program adapted from an example code in the public domain.
  root = SD.open("/") ;                                                                 // MicroSD Read program adapted from an example code in the public domain.
  root.rewindDirectory();                                                               // MicroSD Read program adapted from an example code in the public domain.


  lcd.init() ;                                     
  lcd.backlight() ;                                
  PrntLCD(TpLnArray,0,0,0,16) ;                    // PrntLCD(Array,Array start position ,line of LCD ,LCD start position ,Length of characters)
  delay(3000);
}

/***********************************************
 *   LL       OOOOOOOO   OOOOOOOO   PPPPPPPP   *
 *   LL       OO    OO   OO    OO   PP    PP   *
 *   LL       OO    OO   OO    OO   PP    PP   *
 *   LL       OO    OO   OO    OO   PPPPPPPP   *
 *   LL       OO    OO   OO    OO   PP         *
 *   LL       OO    OO   OO    OO   PP         *
 *   LLLLLL   OOOOOOOO   OOOOOOOO   PP         *
 ***********************************************/
void loop() {
  ReadAllFiles();
  
}

/**********************************************************************************************
 *   RRRRRRRR   OOOOOOOO   UU    UU   TTTTTTTT   IIIIIIII   NN     NN   EEEEEEEE   SSSSSSSS   *
 *   RR    RR   OO    OO   UU    UU      TT         II      NNN    NN   EE         SS         *
 *   RR    RR   OO    OO   UU    UU      TT         II      NN NN  NN   EE         SS         *
 *   RRRRRRRR   OO    OO   UU    UU      TT         II      NN  NN NN   EEEEEEEE   SSSSSSSS   *
 *   RR  RR     OO    OO   UU    UU      TT         II      NN   NNNN   EE               SS   *
 *   RR   RR    OO    OO   UU    UU      TT         II      NN    NNN   EE               SS   *
 *   RR    RR   OOOOOOOO   UUUUUUUU      TT      IIIIIIII   NN     NN   EEEEEEEE   SSSSSSSS   *
 **********************************************************************************************/

 /*****************************************************************
 * Liquid Crystal Display
 *****************************************************************/
void PrntLCD(char T[], byte Astart, int line, byte Lstart, byte len) {    // This will display any char array in the top or bottom line of the LCD (top line = 0) (bottom line = 1)
  lcd.setCursor(Lstart,line) ;                                            // Lstart = (LCD start position)
  for (int i=0;i<len;i++) lcd.print(T[i+Astart]) ;                        // Astart = (Array Start position)
}

void ShftAddPrntTop(char c){                 // This does what it is titled
  for (int j=0;j<15;j++)TpLnArray[j]=TpLnArray[j+1] ; 
  TpLnArray[15]=c ;              
  PrntLCD(TpLnArray,0,0,0,16) ;              // PrntLCD(Array,Array start position ,line of LCD ,LCD start position ,Length of characters)
}

void ShftAddPrntBot(char c){                 // This does what it is titled
  for (int l=0;l<15;l++)BtLnArray[l]=BtLnArray[l+1] ;
  BtLnArray[15]=c ;
  PrntLCD(BtLnArray,0,1,0,16) ;              // PrntLCD(Array,Array start position ,line of LCD ,LCD start position ,Length of characters)
}   

/******************************************************************
 * Sound Dit or Dah  //The Morse sounding portion of this sketch was adapted from a Mark VandeWettering K6HX sketch
 ****************************************************************/
void DitOrDah(int duration, char c) {  // char c is the '.' or '_' that will scrool across the bottom LCD
  tone(Sound_Pin,Hrtz) ;
  TimeDelay(duration) ;
  noTone(Sound_Pin) ;
  ShftAddPrntBot(c) ;
  TimeDelay(DOTLEN) ;                  //this is the delay between dots and dashes
}

/****************************************************************
 * SEND MORSE CHARACTER  //The Morse sounding portion of this sketch was adapted from a Mark VandeWettering K6HX sketch
 ***************************************************************/
void send(char c) {
  if ((c == ' ' || c == '\r') && SpaceFlg==0) {          // SpaceFlg prevents extra spaces and delays if there is a large string of spaces or returns in the text.
    SpaceFlg=1;                                          // SpaceFlg also adds spaces between words after returns in the text.
    ShftAddPrntTop(' ') ;                                // Any space or return will show up as a space on the LCD but there will only be ONE space on the LCD no matter how many spaces or returns are strung together in the text.
    TimeDelay(4*DOTLEN) ;                   // This (4*DOTLEN) added to the (3*DOTLEN) (see 14 lines below) turns out to be (7*DOTLEN) delay between words
    return ;
  }
  for (int i=0; i<N_MORSE; i++) {                         // The Morse sounding portion of this sketch was adapted from a Mark VandeWettering K6HX sketch
    if (morsetab[i].c == c) {                             
      SpaceFlg=0;                                         
      unsigned char p = morsetab[i].pat ;                  
      while (p != 1) {                                    // while p in not equal to 1
          if (p & 1) DitOrDah (DASHLEN,'_') ;             // if the LSB (least significant bit...the right most bit) is equal to 1
          else       DitOrDah (DOTLEN,'.') ;              
          p = p/2 ;                                       // This effectively shifts all bits one to the right while removing the right most bit
      }                                                    
      ShftAddPrntTop(c) ;
      ShftAddPrntBot(' ') ;
      TimeDelay(2*DOTLEN) ;                 // This (2*DOTLEN) added to the (DOTLEN) of the DitOrDah() routine totals (3*DOTLEN) delay between characters
      return ;
    }
  }
}

/***************************************************************************************************************
 *  Timed Delay this is a way to check button presses during the delays
 **************************************************************************************************************/
void TimeDelay(int T){
  unsigned long TimerMillis=millis()+T;
  while (millis()<TimerMillis){
    Button();
  }
}

/************************************************************************************************************
 * BUTTON check if button pressed and advance to next file
 ***********************************************************************************************************/
void Button(){
  if (!digitalRead(Button_Pin)){
    noTone(Sound_Pin) ;                // Stop the sound
    entry.seek(EndOfFile);             // Go to end of file, this will advance to next file
    while (!digitalRead(Button_Pin));  // Do nothing while the button is being pressed...just wait till button is released
    delay(50);                         // Debounce
  }
}

/*************************************************************************************************************
 * READ SD CARD                        // MicroSD Read program adapted from an example code in the public domain.
 *************************************************************************************************************/
void ReadAllFiles(){                   // MicroSD Read program adapted from an example code in the public domain.
  entry =  root.openNextFile(); 
  if (! entry) {                       // If no file .... end of the disc....go to beginning
    root.rewindDirectory();
    count=0;
    delay(5000);
  }
  ReadFile();                          // Read the file to morse code 
  count++; 
}

void ReadFile(){                       // MicroSD Read program adapted from an example code in the public domain.
  char c;
  EndOfFile = entry.size();            // EndOfFile is equal to the file size
  if (entry) {
    Serial.print(entry.name());
    Serial.print('\t');
    Serial.print(EndOfFile); 
    Serial.print('\t');
    Serial.println(count);
    entry.seek(0);                     // Go to begining of file
    while (entry.available()) {        // Read from the file until there's nothing else in it:
      c=entry.read();
      Serial.write(c);
      send(toUpperCase(c));            // Added this line to sound morse and if the character is lowercase, it will send as uppercase
    }
    Serial.println();
    entry.close();                     // Close the file so another file can be opened with the root.openNextFile() command
  } 
  else {
    Serial.print("error opening ");    // If the file didn't open, print an error:
    Serial.println(entry.name());
  } 
}
