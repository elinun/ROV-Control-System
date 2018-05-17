/*
 * Whats new:
 *  individual thruster control, for tank controls.
 */
#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
#include <Servo.h>
#include <stdio.h>

struct Trim {
  int multiplier = 1;
  int adder = 0;
};
struct Command {
  char name;
  int pwr;
};

 // the media access control (ethernet hardware) address for the shield:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  
EthernetServer server = EthernetServer(1740);

struct Trim lTT,rTT,v1TT,v2TT;

Servo clawServo;
int clawPos = 0;

Servo camServo;
int camPos = 0;

int camMax = 90;
int clawMax = 75;

int camInertia = 0;//-1 back, 0 stop, 1 fwd
int clawInertia = 0;

Servo vert1T;
Servo vert2T;

Servo lT;
//Servo fT;

Servo rT;
//Servo bT;
/*
 * 3,5, 6 - reverse these
 * PIN OUTS:
 * PIN | Usage            | Code Reference | Cylinder Wire #      | Jefferson Number
 * 2    Cam Servo           camServo        | 1(out), 4(in)tape   | N/A
 * 3/5  Left Thruster*       lT              | 13                    6 / 3
 * 4    Claw Servo          clawServo       | 4(out), 1(in)
 * 5    Front Thruster      fT              | 14                    3                   Off the ROV
 * 6/3  Right Thruster*      rT              | 8                     4 / 6
 * 7    Back Thruster       bT              | 5                     5                   Off the ROV
 * 10   Used by shield      --
 * 11    ||                 --
 * 8    Vertical Thruster   vert1T          | 19                    1  
 * 9    Vertical Thruster   vert2T          | 12                    2
 */
bool acousticOn = false;
long previousTime = millis();
void setup() {
  // put your setup code here, to run once:
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  Serial.begin(19200);

  vert1T.attach(8);
  vert2T.attach(9);

  lT.attach(6);
  //fT.attach(5);
  rT.attach(3);
  //bT.attach(7);
    Serial.println("Joining Network");
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    }
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    for (;;)
      ;

  }
  Serial.println("Network Joined. ");
  

  vert1T.writeMicroseconds(1500);
  vert2T.writeMicroseconds(1500);
  lT.writeMicroseconds(1500);
  rT.writeMicroseconds(1500);

  clawServo.attach(2);//switched on 4/15/18
  camServo.attach(4);//switched with clawServo. Chart Does not reflect changes (I think).
  
  delay(5000);
  /*digitalWrite(A0, HIGH);
  delay(1000);
  digitalWrite(A0, LOW);*/
  
  //camServo.write(camPos);
  //clawServo.write(clawPos);
  
  

  //test w/ servo

  /*lT.writeMicroseconds(1900);
  delay(1300);
  lT.writeMicroseconds(1500);*/


  // start listening for clients
  server.begin();
  printIPAddress();
  
}

void loop() {
  // put your main code here, to run repeatedly:

   EthernetClient client = server.available();
  
  if (client) {
     Serial.println("client here");
     while(client.connected()){
      if(client.available()){
        //1. read and print line
        String ln = client.readStringUntil('\n');
    //server.write(ln);
    Serial.println(ln);
//2. Parse Command
    struct Command cmd;
    //"qf1750 l1400 "
    int lastIndex = 0;
    for(int i = 1; i<ln.length();i++){//probably a better way to do this performance wise
      char c = ln.charAt(i);
      if(c == '\t'){
        lastIndex = i;
        setValues(cmd);
      }else{
        if(i == (lastIndex+1)){
          cmd.name = c;
        }
        else{
          cmd.pwr = ln.substring(i, ln.indexOf('\t', i)).toInt();
          i = ln.indexOf('\t', i)-1;
        }
      }
    }
    //3. Delay for performance.
    delay(9);
    //write claw and cam inertias
    if(camPos>-camInertia&&camPos<(camMax-camInertia)){
      camPos += camInertia;
      camServo.write(camPos);
    }
    if(clawPos>-clawInertia&&clawPos<(clawMax-clawInertia)){
      clawPos += clawInertia;
      clawServo.write(clawPos);
    }
    
        //6. Print how long this took.
    long mTime = millis();
    Serial.println(mTime-previousTime);
    previousTime = mTime;
      }
     }
  }
}

void setValues(struct Command cmd){

  //Serial.println("Values set");
 switch (cmd.name) {
      case 'l':
        lT.writeMicroseconds(cmd.pwr);
        break;
      case 'r':
        rT.writeMicroseconds(cmd.pwr);
        break;
      case 'u'://vert1
        vert1T.writeMicroseconds(cmd.pwr);
        break;
      case 'U'://vert2
        vert2T.writeMicroseconds(cmd.pwr);
        break;
      case 't':
        digitalWrite(A2, HIGH);
        setup();
        break;
      case 's': //reset ROV to stop
        vert1T.writeMicroseconds(1500);
        vert2T.writeMicroseconds(1500);
       // fT.writeMicroseconds(1500);
        lT.writeMicroseconds(1500);
        rT.writeMicroseconds(1500);
       // bT.writeMicroseconds(1500);
        break;
      case 'a':
        Serial.println("Toggling Acoustic Release");
        acousticOn = !acousticOn;
        if(acousticOn){
          digitalWrite(A0, HIGH);
          digitalWrite(A1, HIGH);
        }else{
          digitalWrite(A0, LOW);
          digitalWrite(A1, LOW);
        }
        break;
 
     case 'q'://set claw pos by microsecond
          camServo.writeMicroseconds(cmd.pwr);
          break;
     case 'Q':
          clawServo.write(cmd.pwr);
          break;
     case 'w':
          camServo.writeMicroseconds(cmd.pwr);
          break;
     case 'W':
          clawServo.write(cmd.pwr);
          break;
        
     case ','://claw open
        if(clawPos<clawMax){
        clawServo.write(clawPos+cmd.pwr);
        clawPos+=cmd.pwr;
        }
        break;
      case '.'://Claw close
        if(clawPos>0){
        clawServo.write(clawPos-cmd.pwr);
        clawPos-=cmd.pwr;
        }
        break;
      case 'g'://Go or stop cam
        camInertia = cmd.pwr;
        break;
      case 'G'://Go or stop claw
        clawInertia = cmd.pwr;
        break;
      case 'm'://set cam max pos
          camMax = cmd.pwr;
          break;
      case 'M'://set claw max pos
          clawMax = cmd.pwr;
          break;
      case '<':
      if(clawPos>0){
        clawServo.writeMicroseconds(clawPos-cmd.pwr);
        clawPos-=cmd.pwr;
        }
        break;
      case '>':
       if(clawPos<clawMax){
        clawServo.writeMicroseconds(clawPos+cmd.pwr);
        clawPos+=cmd.pwr;
        }
        break;
     case 'c'://cam up
        if(camPos<camMax){
        camServo.write(camPos+cmd.pwr);
        camPos+=cmd.pwr;
        }
        break;
      case 'v'://cam down
        if(camPos>0){
        camServo.write(camPos-cmd.pwr);
        camPos-=cmd.pwr;
        }
        break;
     case '/':
          if(camPos<camMax){
        camServo.writeMicroseconds(camPos+cmd.pwr);
        camPos+=cmd.pwr;
        }
        break;
    case '?':
      if(camPos>0){
        camServo.writeMicroseconds(camPos-cmd.pwr);
        camPos-=cmd.pwr;
        }
        default:
          Serial.print("Default Reached");
          Serial.println(cmd.name);
          break;
        
    }
}


void printIPAddress(){
  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }

  Serial.println();
}
