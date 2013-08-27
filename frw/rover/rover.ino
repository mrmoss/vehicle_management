//Rover Vehicle Control Firmware
//Created By:   Mike Moss
//Modified On:  08/27/2013

//Serial Sync Protocol
//0    left speed
//1    right speed
//2    enc0a
//3    enc0b
//4    enc1a
//5    enc1b
//6    ir0
//7    ir1
//8    us0
//9    us1
//10   acceleromter x
//11   acceleromter y
//12   acceleromter z

//Serial Sync Header
#include <SerialSyncArduino.h>

//Global Variables
byte dir_pins[4]={10,11,12,13};
byte spd_pins[4]={6,7,8,9};
byte int_nums[4]={2,3,4,5};
bool fwd_dirs[4]={HIGH,HIGH,HIGH,HIGH};
int enc0a_count=0;
int enc0b_count=0;
int enc1a_count=0;
int enc1b_count=0;
byte ir0_pin=A0;
byte ir1_pin=A1;
byte us0_pin=A2;
byte us1_pin=A3;
byte accel_x_pin=A4;
byte accel_y_pin=A5;
byte accel_z_pin=A6;
SerialSync xbee(Serial2,9600);

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
  //Update Motors
  move_rover(xbee.get(0),xbee.get(1));

  //Update Sensors
  xbee.set(2,analogRead(enc0a_count));
  xbee.set(3,analogRead(enc0b_count));
  xbee.set(4,analogRead(enc1a_count));
  xbee.set(5,analogRead(enc1b_count));
  xbee.set(6,analogRead(ir0_pin));
  xbee.set(7,analogRead(ir1_pin));
  xbee.set(8,analogRead(us0_pin));
  xbee.set(9,analogRead(us1_pin));
  xbee.set(10,analogRead(accel_x_pin));
  xbee.set(11,analogRead(accel_y_pin));
  xbee.set(12,analogRead(accel_z_pin));

  //Update Radio Communication
  xbee.loop();
}
