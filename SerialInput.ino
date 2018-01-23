#include<Servo.h>

struct channel {
  unsigned int port: 4;
  unsigned int value: 8; //PWM value 0-fwd 127-stop 255-bkd
};
struct channelValues {
  struct channel left1Thrust;
  struct channel left2Thrust;
  struct channel right1Thrust;
  struct channel right2Thrust;
  struct channel vert1Thrust;
  struct channel vert2Thrust;
} values;
Servo camServo;//hope I don't need pwm. -You don't
int camPos = 90;//since the program will let these values go slightly below zero, we must either not let these go below zero or make these normal ints.
Servo clawServo;
int clawPos = 90;
Servo clawUpDown;
unsigned int accelFactor = 3;//this is geared towards keyboard control. But is also used 4 rotation

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.setTimeout(10);
  // pinMode(2, OUTPUT);
  camServo.attach(2);
  camServo.write(90);
  clawServo.attach(4);
  clawServo.write(90);
  clawUpDown.attach(7);
  values.left1Thrust.port = 3;
  values.left2Thrust.port = 5;
  values.right1Thrust.port = 6;
  values.left2Thrust.port = 9;
  values.vert1Thrust.port = 10;
  values.vert2Thrust.port = 11;
}

void loop() {
  // put your main code here, to run repeatedly:
  while (Serial.available() > 0) {
    String ln = Serial.readString();
    //Serial.println(ln);
    char cmd = ln.charAt(0);
    int pwr = ln.substring(1, ln.length() - 1).toInt();
    switch (cmd) {
      case 'f'://ROV forward&backward
        //PWM value 0-fwd 127-stop 255-bkd
        values.left1Thrust.value = pwr;
        values.right1Thrust.value = pwr;
        break;
      case 'l'://ROV left&right
        //PWM value 0-fwd 127-stop 255-bkd
        values.left2Thrust.value = pwr;
        values.right2Thrust.value = pwr;
        break;
      case 'z'://rotate counter-clockwise
        values.left1Thrust.value += accelFactor;
        values.right1Thrust.value -= accelFactor;
        values.left2Thrust.value += accelFactor;
        values.right2Thrust.value -= accelFactor;
        break;
      case 'r': //reset ROV to stop
        values.left1Thrust.value = 127;
        values.right1Thrust.value = 127;
        values.left2Thrust.value = 127;
        values.right2Thrust.value = 127;
        values.vert1Thrust.value = 127;
        values.vert2Thrust.value = 127;
        break;
      case 'x'://rotate clockwise
        values.left1Thrust.value -= accelFactor;
        values.right1Thrust.value += accelFactor;
        values.left2Thrust.value -= accelFactor;
        values.right2Thrust.value += accelFactor;
        break;
      case 'u'://ROV Up or down
        values.vert1Thrust.value = pwr;
        values.vert2Thrust.value = pwr;
        break;

      case 'c'://camera up. Command from remote client of the connected USB computer
        //Serial.println(camPos);
        if(camPos<180){
          camServo.write(camPos+pwr);
          camPos= camPos+pwr;
        }
        break;
      case 'v'://Cam down
        if(camPos>0){
        camServo.write(camPos-pwr);
        camPos-=pwr;
        }
        break;
      case ','://claw open
        if(clawPos<180){
        clawServo.write(clawPos+5);
        clawPos+=5;
        }
        break;
      case '.'://Claw close
        if(clawPos>0){
        camServo.write(clawPos-5);
        clawPos-=5;
        }
        break;
       case 'a'://acoustic release
          digitalWrite(12, HIGH);
          break;
       case 's':
          digitalWrite(12, LOW);
          break;
        
    }
    //TODO: need to check for values over 255 and under 0;
    analogWrite(values.left1Thrust.port, values.left1Thrust.value);
    analogWrite(values.left2Thrust.port, values.left2Thrust.value);
    analogWrite(values.right2Thrust.port, values.right2Thrust.value);
    analogWrite(values.right1Thrust.port, values.right1Thrust.value);
    analogWrite(values.vert1Thrust.port, values.vert1Thrust.value);
    analogWrite(values.vert2Thrust.port, values.vert2Thrust.value);
  }
}
