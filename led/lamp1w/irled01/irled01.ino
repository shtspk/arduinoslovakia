/**
 * Arduino Uno IR remote 1 W led driver
 * v. 1.0
 * Copyright (C) 2015 Robert Ulbricht
 *
 * IR Receiver + PWM output on pin 5, 6
 * 10 level of light with nonlinear sensitivity
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/ 

#include <IRremote.h>
#include "irtestdefine.h"

const int RECV_PIN = 11;
const int LED_PIN1 = 5; // channel 1
const int LED_PIN2 = 6; // channel 2
const int MS_ADD = 10;

///
/// Translation table
///
struct rbl
{
  RButton b;
  long value;
  const char* name;
};

///
/// Translation table
///
rbl rblv[] =
{
  {rbStandby,vSTANDBY,"STANDBY"},
  {rbBTN1   ,vBTN1   ,"BTN1"},
  {rbBTN2   ,vBTN2   ,"BTN2"},
  {rbBTN3   ,vBTN3   ,"BTN3"},
  {rbBTN4   ,vBTN4   ,"BTN4"},
  {rbBTN5   ,vBTN5   ,"BTN5"},
  {rbBTN6   ,vBTN6   ,"BTN6"},
  {rbBTN7   ,vBTN7   ,"BTN7"},
  {rbBTN8   ,vBTN8   ,"BTN8"},
  {rbBTN9   ,vBTN9   ,"BTN9"},
  {rbBTN0   ,vBTN0   ,"BTN0"},
  {rbPLUS10 ,vPLUS10 ,"PLUS10"},
  {rbMINUS10,vMINUS10,"MINUS10"},
  {rbPLUS1  ,vPLUS1  ,"PLUS1"},
  {rbMINUS1 ,vMINUS1 ,"MINUS1"},
  {rbCH1    ,vCH1    ,"CH1"},
  {rbCH2    ,vCH2    ,"CH2"},
  {rbCH12   ,vCH12   ,"CH12"},
};
int RBLN=sizeof(rblv)/sizeof(rbl);

IRrecv irrecv(RECV_PIN);
decode_results results;
int cl=0;
int clstandby=0;
boolean active1=true;
boolean active2=true;
int coloractive1=0;
int coloractive2=0;
int colorrequest1=0;
int colorrequest2=0;
unsigned long lasttime=0;

// table of exponential values
// generated for values of i from 0 to 255 -> x=round( pow( 2.0, i/32.0) - 1);
const byte table[] PROGMEM = {
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   2,   2,   2,   2,   2,
  2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   3,   3,   3,   3,   3,   3,
  3,   3,   3,   3,   3,   3,   4,   4,   4,   4,   4,   4,   4,   4,   4,   5,
  5,   5,   5,   5,   5,   5,   5,   6,   6,   6,   6,   6,   6,   6,   7,   7,
  7,   7,   7,   8,   8,   8,   8,   8,   9,   9,   9,   9,   9,  10,  10,  10,
 10,  11,  11,  11,  11,  12,  12,  12,  12,  13,  13,  13,  14,  14,  14,  15,
 15,  15,  16,  16,  16,  17,  17,  18,  18,  18,  19,  19,  20,  20,  21,  21,
 22,  22,  23,  23,  24,  24,  25,  25,  26,  26,  27,  28,  28,  29,  30,  30,
 31,  32,  32,  33,  34,  35,  35,  36,  37,  38,  39,  40,  40,  41,  42,  43,
 44,  45,  46,  47,  48,  49,  51,  52,  53,  54,  55,  56,  58,  59,  60,  62,
 63,  64,  66,  67,  69,  70,  72,  73,  75,  77,  78,  80,  82,  84,  86,  88,
 90,  91,  94,  96,  98, 100, 102, 104, 107, 109, 111, 114, 116, 119, 122, 124,
127, 130, 133, 136, 139, 142, 145, 148, 151, 155, 158, 161, 165, 169, 172, 176,
180, 184, 188, 192, 196, 201, 205, 210, 214, 219, 224, 229, 234, 239, 244, 250
};

///
/// From scancode to enum
///
RButton DecodeValue(long v)
{
for(int i=0;i<RBLN;i++)
  if(rblv[i].value==v)
    return rblv[i].b;
return rbUNKNOWN;
}

///
/// From enum to human name
///
const char* DecodeName(RButton b)
{
for(int i=0;i<RBLN;i++)
  if(rblv[i].b==b)
    return rblv[i].name;
return "UNKNOWN";  
}

///
/// Update all channels
///
void updateColors()
{
unsigned long t=millis();
if(t>lasttime)
  {
  // channel 1
  if(colorrequest1>coloractive1)
    {
    coloractive1++;
    colorOne(LED_PIN1,coloractive1);
    }
  if(colorrequest1<coloractive1)
    {
    coloractive1--;
    colorOne(LED_PIN1,coloractive1);
    }

  // channel 2
  if(colorrequest2>coloractive2)
    {
    coloractive2++;
    colorOne(LED_PIN2,coloractive2);
    }
  if(colorrequest2<coloractive2)
    {
    coloractive2--;
    colorOne(LED_PIN2,coloractive2);
    }

  lasttime=t+MS_ADD; 
  }
}

///
/// Set one PWM output
///
void colorOne(int pin, int c)
{
if(c==0)
  digitalWrite(pin,LOW);
else if(c==255)
  digitalWrite(pin,HIGH);
else
  analogWrite(pin,pgm_read_byte(&table[c]));
}

///
/// Set one PWM request
void requestColorOne(int pin, int c)
{
if(pin==1)
  colorrequest1=c;
else
  colorrequest2=c;
}

///
/// Set PWM output
///
void color(int c)
{
Serial.print("color=");
Serial.print(c);
Serial.println("%");

cl=c;
c=map(c,0,100,0,255);

// channel 1
if(active1)
  requestColorOne(1,c);
else  
  requestColorOne(1,0);
  
// channel 2
if(active2)
  requestColorOne(2,c);
else  
  requestColorOne(2,0);
clstandby=0;
}

///
/// Setup function
///
void setup()
{
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
  pinMode(LED_PIN1,OUTPUT);
  pinMode(LED_PIN2,OUTPUT);
  Serial.println("Arduino Uno IR LED driver");
  color(50);
}

///
/// Main loop
///
void loop() {
  updateColors();
  
  if (irrecv.decode(&results)) {
    Serial.print(results.value, HEX);
    RButton b=DecodeValue(results.value);
    Serial.print(" - ");
    Serial.print(DecodeName(b));
    Serial.println("");
    switch(b)
      {
      case rbBTN1:color(10);break;  
      case rbBTN2:color(20);break;  
      case rbBTN3:color(30);break;  
      case rbBTN4:color(40);break;  
      case rbBTN5:color(50);break;  
      case rbBTN6:color(60);break;  
      case rbBTN7:color(70);break;  
      case rbBTN8:color(80);break;  
      case rbBTN9:color(90);break;  
      case rbBTN0:color(100);break;  
      case rbPLUS10:
        cl+=10;
        if(cl>100) cl=100;
        color(cl);
        break;
      case rbMINUS10:
        cl-=10;
        if(cl<0) cl=0;
        color(cl);
        break;
      case rbPLUS1:
        cl+=1;
        if(cl>100) cl=100;
        color(cl);
        break;
      case rbMINUS1:
        cl-=1;
        if(cl<0) cl=0;
        color(cl);
        break;
      case rbStandby:
        if(clstandby==0)
          {
          int tmp=cl;
          if(cl==0)
            color(50);
          else
            {
            color(0);  
            clstandby=tmp;
            }
          }
        else
          {
          color(clstandby);
          }  
        break;
      case rbCH1:
        active1=true;
        active2=false;
        color(cl);
        break;  
      case rbCH2:
        active1=false;
        active2=true;
        color(cl);
        break;  
      case rbCH12:
        active1=true;
        active2=true;
        color(cl);
        break;  
      }
    irrecv.resume(); // Receive the next value
  }
  //delay(100);
}
