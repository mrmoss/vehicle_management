//Rover Vehicle Control Firmware
//Created By:   Mike Moss
//Modified On:  08/27/2013

//Serial Sync Protocol
//0    current x
//1    current y
//2    destination x
//3    destination y
//4    enc0a
//5    enc0b
//6    enc1a
//7    enc1b
//8    ir0
//9    ir1
//10   us0
//11   us1
//12   acceleromter x
//13   acceleromter y
//14   acceleromter z
//15   direction

//Serial Sync Header
#include "serial_sync.h"

//Global Variables
byte dir_pins[4]={11,12,13,10};
byte spd_pins[4]={7,8,9,6};
byte int_nums[4]={4,5,2,3};
bool fwd_dirs[4]={LOW,HIGH,HIGH,LOW};
int x=0;
int y=0;
int dx=0;
int dy=0;
int direction=0;
int enc0a_count=1;
int enc0b_count=2;
int enc1a_count=3;
int enc1b_count=4;
byte ir0_pin=A0;
byte ir1_pin=A1;
byte us0_pin=A2;
byte us1_pin=A3;
byte accel_x_pin=A4;
byte accel_y_pin=A5;
byte accel_z_pin=A6;
msl::serial_sync xbee(Serial2,57600);

//Move Rover Function (Negative numbers mean reverse direction)
void move_rover(int left_spd,int right_spd)
{
  //Create Direction and Speed Arrays
  bool directions[4]={fwd_dirs[0],fwd_dirs[1],fwd_dirs[2],fwd_dirs[3]};
  int speeds[4]={left_spd,right_spd,right_spd,left_spd};

  //Left Side Reverse
  if(left_spd<0)
  {
    //Flp Directions
    directions[0]=!directions[0];
    directions[3]=!directions[3];

    //Flip Speeds
    speeds[0]=-speeds[0];
    speeds[3]=-speeds[3];
  }

  //Right Side Reverse
  if(right_spd<0)
  {
    //Flip Directions
    directions[1]=!directions[1];
    directions[2]=!directions[2];

    //Flip Speeds
    speeds[1]=-speeds[1];
    speeds[2]=-speeds[2];
  }

  //Set Direction and Speed
  for(int ii=0;ii<4;++ii)
  {
    digitalWrite(dir_pins[ii],directions[ii]);
    analogWrite(spd_pins[ii],speeds[ii]);
  }
}

//Stop Rover Function (Halts rover movement)
void stop_rover()
{
  move_rover(0,0);
}

//Setup (Happens once)
void setup()
{
  //Setup Motor Pins
  for(int ii=0;ii<4;++ii)
  {
    pinMode(dir_pins[ii],OUTPUT);
    pinMode(spd_pins[ii],OUTPUT);
  }

  //Motors Default to Off
  stop_rover();

  //Setup Radio Communication
  xbee.setup();
}

//Loop (Happens forever)
void loop()
{
  //Update Radio RX
  xbee.update_rx();

  //Update Current Position
  x=xbee.get(0);
  y=xbee.get(1);

  //Update Desired Position
  dx=xbee.get(2);
  dy=xbee.get(3);

  //Update Direction
  direction=xbee.get(15);

  //Update Sensors
  xbee.set(4,enc0a_count);
  xbee.set(5,enc0b_count);
  xbee.set(6,enc1a_count);
  xbee.set(7,enc1b_count);
  xbee.set(8,analogRead(ir0_pin));
  xbee.set(9,analogRead(ir1_pin));
  xbee.set(10,analogRead(us0_pin));
  xbee.set(11,analogRead(us1_pin));
  xbee.set(12,analogRead(accel_x_pin));
  xbee.set(13,analogRead(accel_y_pin));
  xbee.set(14,analogRead(accel_z_pin));

  //TESTING
  move_rover(dx,dy);

  //Update Radio TX
  xbee.update_tx();
}
