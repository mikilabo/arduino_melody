#include <TextFinder.h>
#include <Wire.h>             // I2C
#include <wI2cLcdACM1602.h>   // LCD

/****************************************************************
FastConnect.ino
CC3000 FastConnect Test
Shawn Hymel @ SparkFun Electronics
March 1, 2014
https://github.com/sparkfun/SFE_CC3000_Library

Connects to the WiFi network profile stored in non-volatile
memory. Performs a ping test to verify functionality.

NOTE: You must run SmartConfig.ino prior to this sketch in order
to setup a profile.

Hardware Connections:
 
 Uno Pin    CC3000 Board    Function
 
 +5V        VCC or +5V      5V
 GND        GND             GND
 2          INT             Interrupt
 7          EN              WiFi Enable
 10         CS              SPI Chip Select
 11         MOSI            SPI MOSI
 12         MISO            SPI MISO
 13         SCK             SPI Clock

Resources:
Include SPI.h and SFE_CC3000.h

Development environment specifics:
Written in Arduino 1.0.5
Tested with Arduino UNO R3

This code is beerware; if you see me (or any other SparkFun 
employee) at the local, and you've found our code helpful, please
buy us a round!

Distributed as-is; no warranty is given.
****************************************************************/

#include <SPI.h>
#include <SFE_CC3000.h>
#include <SFE_CC3000_Client.h>

// Pins
#define CC3000_INT      2   // Needs to be an interrupt pin (D2/D3)
#define CC3000_EN       7   // Can be any digital pin
#define CC3000_CS       10  // Preferred is pin 10 on Uno

#define melodyPin       17    //buzzer  

#define NODATA     "NODATA"  //keyword for melody
#define PORT        3000
#define MYNAME      "myname=MFT2014-001"

// Constants
unsigned int timeout = 30000;            // Milliseconds
prog_char remote_host_p[] PROGMEM = "mft.mikilabo.net"; 

/*
char ap_ssid[] = "MIKI-iphone";                  // SSID of network
char ap_password[] = "qwert19750526";          // Password of network
unsigned int ap_security = WLAN_SEC_WPA2; // Security of network
*/

// 文字列の配列をフラッシュメモリ上に配置する。
PROGMEM const prog_char *ip_table[] = {
  remote_host_p,
};


// Global Variables
SFE_CC3000 wifi = SFE_CC3000(CC3000_INT, CC3000_EN, CC3000_CS);
SFE_CC3000_Client client = SFE_CC3000_Client(wifi);

#define BUF_SIZE  370


#define MAX_TONE  100
char buf[BUF_SIZE], name_s[16];
int melody_int[MAX_TONE];
byte temp_int[MAX_TONE];
int i;
volatile int melody_state = false;

byte melody_counter = 0;

wI2cLcdACM1602 lcd;           //

void setup() {
  Serial.begin(115200);
  Wire.begin();
  lcd.begin();
  
  //LCD 
  lcd.clear();        // 全クリア
  lcd.noBlink();      // カーソル点滅なし
  lcd.noCursor();     // カーソル表示なし
  lcd.print(F("Setup Started.")); 
  
  
  pinMode(melodyPin, OUTPUT);//buzzer
  pinMode(3, INPUT);//割り込み
  pinMode(13, OUTPUT);//led indicator when singing a note
  
  
  Serial.println(F("Setup 1:"));
  
  // Initialize CC3000 (configure SPI communications)
  if ( wifi.init() ) {
    Serial.println(F("CC3000 initialization complete"));
  } else {
    Serial.println(F("Something went wrong during CC3000 init!"));
  }
  
  // Connect to WiFi network stored in non-volatile memory  
  Serial.println(F("Connecting to network stored in profile..."));
  if ( !wifi.fastConnect(timeout) ) {
    Serial.println(F("Error: Could not connect to network"));
  }
  /*  
  if(!wifi.connect(ap_ssid, ap_security, ap_password, 30000)) {
    Serial.println("Error: Could not connect to AP");
  }
  */
  
  Serial.println(F("Setup Done."));
  lcd.clear();    // カーソル位置指定
  lcd.print(F("Setup Completed.")); 
  delay(1000);
}

//Text Finder
TextFinder finder( client );
int counter = 0;

//
// main loop
//
void loop() {
  byte isAttached = 0;
  
  lcd.clear();            // 全クリア
  lcd.print(F("Waiting."));     // 文字列表示   
  
  Serial.println(F("XXX loop1 "));
  //get IP addr from Flash Mem
  strcpy_P(buf, (char*)pgm_read_byte(&(ip_table[0])));
  //Serial.print("XXX: ");Serial.println(buf);  

  //connect to server , ip addar included in buf 
  //memset(&client, 0, sizeof(client));
  
  if (client.connect(buf, PORT)) { 
      if(melody_state == false){
        Serial.println(F("connected for melody"));
        client.print(F("GET /melody/?counter="));
        client.print(counter++);
        client.print(F("&"));
        client.print(F("stop=0"));
        client.print(F("&"));
        client.print(F(MYNAME));
        client.println(F(" HTTP/1.0"));
        client.println();
      }else{
        Serial.println(F("connected for stop"));
        client.print(F("GET /melody/?counter="));
        client.print(counter++);
        client.print(F("&"));
        client.print(F("stop=1"));
        client.print(F("&"));
        client.print(F(MYNAME));
        client.println(F(" HTTP/1.0"));
        client.println();
        
        melody_state = false;
      }
      //Serial.println(buf);
      //client.println("GET /melody HTTP/1.0");
      //client.println(buf);
            
    }else{
      Serial.println(F("connection failed"));
      
      lcd.clear();    // カーソル位置指定
      lcd.print(F("Connection"));
      lcd.setCursor(0,1);
      lcd.print(F("Failed, Retry."));
      delay(3000);
      goto end;
    }
  
  Serial.println(F("XXX loop2"));
  
/** 
  //connection check for DEBUG
  while(1){
  // If there are incoming bytes, print them
  if ( client.available() ) {
    char c = client.read();
    Serial.print(c);
  }
  }
**/
 
 
  //Serial.println("XXX1");
  if (client.connected()) {
    //Serial.println("XXX11");
    //delay(1000);
    
    finder.getString("<name>", "</name>", name_s, sizeof(name_s));
    Serial.println(F("name done: ")); //Serial.print(name); Serial.println("]");
    if(strcmp(name_s, NODATA) == 0 ){
       //no melody at this moment
       Serial.println(F("no data, disconnected."));
       client.stop();
      
       //exit loop
       goto end;
    }
    Serial.print(F("name: "));
    Serial.println(name_s);
      
    finder.getString("<tone>", "</tone>", buf, BUF_SIZE);
    Serial.println(F("tone done: ")); //Serial.println(buf);
    get_csv_num(buf, melody_int, NULL, 0);
    
    finder.getString("<tempo>", "</tempo>", buf, BUF_SIZE);
    get_csv_num(buf, NULL, temp_int, 1);
    Serial.println(F("temo done: ")); //Serial.println(buf);

    finder.getString("<name_s1>", "</name_s1>", name_s, 16);
    Serial.print(F("name_s1 done: ")); //Serial.println(buf);
    Serial.println(name_s);
    lcd.clear();            // 全クリア
    lcd.print(name_s);     // 文字列表示
    
    finder.getString("<name_s2>", "</name_s2>", name_s, 16);
    Serial.print(F("name_s2 done: ")); //Serial.println(buf);
    Serial.println(name_s);
    lcd.setCursor(0, 1);    // カーソル位置指定
    lcd.print(name_s);     // 文字列表示   
    
    client.stop();
    Serial.println(F("disconnected."));
  } 
    
 
  
  //Serial.println("hoge1");
  for(i = 0; i < MAX_TONE; i++){
    
    //during the melody 
    if( isAttached == 0 ){
      attachInterrupt(1, pressed, RISING);
      isAttached = 1;
    }
    
    if( melody_state == true ){
      //button pushed!!      
      break;
    }
    if( melody_int[i] == -1){
      i = 0; // indefinite loop
      
      melody_counter++;
      if( melody_counter == 5){
        melody_counter = 0;
        break;
      }
    }
        
    int noteDuration = 1000 / temp_int[i];
        
    // to calculate the note duration, take one second
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    buzz(melodyPin, melody_int[i], noteDuration);
        
    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
  
    // stop the tone playing:
    buzz(melodyPin, 0, noteDuration);
    
    
  }
  detachInterrupt(1);

end:
  ;
}


static int
get_csv_num(char *buf, int num[], byte num2[], int flg){
  char    *av[MAX_TONE];
  
  //CSV Parse 
  parse_data( buf , av, MAX_TONE);
  
  for(i=0; i<MAX_TONE; i++){
    //Serial.print(i); Serial.print(" :"); 
    if( flg == 0 ){
      num[i] = atoi(av[i]);
      //Serial.println(num[i]);
    }else{
      num2[i] = atoi(av[i]);
      //Serial.println(num2[i]);
    }
    
    if(atoi(av[i]) == -1){
       break;
    }
  }
  //Serial.println("get_csv_num done");
  
  return 0;
}

/**
 * csv parser function
 */
static int
parse_data(
    char        *buf,
    char        **av,
    int     max_av
)
{
    int i = 0;
    int counter = 0;

    av[0] = (char*)buf;
    while(1){

        //改行コードがきたら終わり
        if( buf[i] == '\n' || buf[i] =='\r' || buf[i] =='\0'){
            buf[i] = '\0';
            goto end;
        }

        //カンマがきたら次の行
        if( buf[i] == ','){
            buf[i] = '\0';
            av[counter+1] = (char*)&buf[i+1];
            i++;
            counter++;
            if( counter == max_av){
                //全部分解終わった
                goto end;
            }
            continue;
        }
        i++;   
    }  

end:
    return 0;
}

void buzz(int targetPin, long frequency, long length) {
  digitalWrite(13, HIGH);
  long delayValue = 1000000 / frequency / 2; 
  // calculate the delay value between transitions
  //// 1 second's worth of microseconds, divided by the frequency, then split in half since
  //// there are two phases to each cycle
  long numCycles = frequency * length / 1000; 
  // calculate the number of cycles for proper timing
  //// multiply frequency, which is really cycles per second, by the number of seconds to
  //// get the total number of cycles to produce
  
  /*
  Serial.println(F("---"));
  Serial.print(F("frequency: ")); Serial.println(frequency); 
  Serial.print(F("length: ")); Serial.println(length);
  Serial.print(F("delayValue: ")); Serial.println(delayValue);   
  Serial.print(F("numCycles: ")); Serial.println(numCycles);
  */
  
  for (long i = 0; i < numCycles; i++) { // for the calculated length of time...
    digitalWrite(targetPin, HIGH); // write the buzzer pin high to push out the diaphram
    delayMicroseconds(delayValue); // wait for the calculated delay value
    digitalWrite(targetPin, LOW); // write the buzzer pin low to pull back the diaphram
    delayMicroseconds(delayValue); // wait again or the calculated delay value
  }
  digitalWrite(13, LOW);

}

void pressed () {
  melody_state = true;
  Serial.println(F("pressed!!"));
}



