#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
#include <Servo.h>
#include <stdio.h>

struct Trim {
  int multiplier;
  int adder;
};
struct Command {
  char name;
  int pwr;
};

 // the media access control (ethernet hardware) address for the shield:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  
EthernetServer server = EthernetServer(1740);

struct Trim lTT,rTT,fTT,bTT,v1TT,v2TT;

Servo clawServo;
int clawPos = 90;

Servo camServo;
int camPos = 90;

Servo vert1T;
Servo vert2T;

Servo lT;
Servo fT;

Servo rT;
Servo bT;
/*
 * 3,5, 6 - reverse these
 * PIN OUTS:
 * PIN | Usage            | Code Reference | Cylinder Wire #      | Jefferson Number
 * 2    Cam Servo           camServo        | 1(out), 4(in)tape   | N/A
 * 3    Left Thruster       lT              | 13                    6
 * 4    Claw Servo          clawServo       | 4(out), 1(in)
 * 5    Front Thruster      fT              | 14                    3
 * 6    Right Thruster      rT              | 8                     4
 * 7    Back Thruster       bT              | 5                     5 
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
  Serial.begin(250000);
  delay(2000);
  /*digitalWrite(A0, HIGH);
  delay(1000);
  digitalWrite(A0, LOW);*/
  clawServo.attach(2);//switched on 4/15
  camServo.attach(4);//switched with clawServo. See chart.
  
  camServo.write(90);
  clawServo.write(0);
  
  vert1T.attach(8);
  vert2T.attach(9);

  lT.attach(3);
  fT.attach(5);
  rT.attach(6);
  bT.attach(7);

  //test w/ servo

  /*lT.writeMicroseconds(1900);
  delay(1300);
  lT.writeMicroseconds(1500);*/

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
    //delay(20);

        //6. Print how long this took.
    long mTime = millis();
    Serial.println(mTime-previousTime);
    previousTime = mTime;
      }
     }
  }
}

void setValues(struct Command cmd){
  /* Not 100% confident in this chart
   *  1100    1100
   *    /     \
   *   1900   1900
   *   1900   1900 
   *    \     /
   *    1100  1100
   * 
   */
  Serial.println("Values set");
 switch (cmd.name) {
      //1100 Full Back | 1500 Stop | 1900 Full Forward
      case 'f'://ROV forward&backward
      Serial.print("Pwr: ");
      Serial.println(cmd.pwr);//corrected inverses
       // lT.writeMicroseconds((lTT.multiplier<0) ? 3000-(cmd.pwr+lTT.adder): cmd.pwr+lTT.adder);//inv
        lT.writeMicroseconds(3000-cmd.pwr+(lTT.adder * (3000-cmd.pwr>1500 ? 1: -1)));
        rT.writeMicroseconds(cmd.pwr+(rTT.adder * (cmd.pwr>1500 ? 1: -1)));
        fT.writeMicroseconds(cmd.pwr+(fTT.adder * (cmd.pwr>1500 ? 1: -1)));
        bT.writeMicroseconds(cmd.pwr+(bTT.adder * (cmd.pwr>1500 ? 1: -1)));
        /*
         *      / \
         *      v  v
         *      
         *      \   /
         *      V   v
         */
        break;
      case 'l'://ROV left&right
        fT.writeMicroseconds(3000-cmd.pwr+(fTT.adder * (3000-cmd.pwr>1500 ? 1: -1)));
        bT.writeMicroseconds(3000-cmd.pwr+(bTT.adder * (3000-cmd.pwr>1500 ? 1: -1)));
        lT.writeMicroseconds(3000-cmd.pwr+(lTT.adder * (3000-cmd.pwr>1500 ? 1: -1)));
        rT.writeMicroseconds(cmd.pwr+(rTT.adder * (cmd.pwr>1500 ? 1: -1)));
        break;
      case 'z'://rotate counter-clockwise
        lT.writeMicroseconds(3000-cmd.pwr+(lTT.adder * (3000-cmd.pwr>1500 ? 1: -1)));
        rT.writeMicroseconds(3000-cmd.pwr+(rTT.adder * (3000-cmd.pwr>1500 ? 1: -1)));
        fT.writeMicroseconds(3000-cmd.pwr+(fTT.adder * (3000-cmd.pwr>1500 ? 1: -1)));
        bT.writeMicroseconds(cmd.pwr+(bTT.adder * (cmd.pwr>1500 ? 1: -1)));
        break;
      case 'r': //reset ROV to stop
        vert1T.writeMicroseconds(1500);
        vert2T.writeMicroseconds(1500);
        fT.writeMicroseconds(1500);
        lT.writeMicroseconds(1500);
        rT.writeMicroseconds(1500);
        bT.writeMicroseconds(1500);
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
      
      case 'x'://roll as of v2.5
        vert1T.writeMicroseconds(1500+cmd.pwr);
        vert2T.writeMicroseconds(1500-cmd.pwr);//cmd.pwr: +/- ~200
        break;
      case 'h'://ROV Up or down
        vert1T.writeMicroseconds(3000-cmd.pwr+(v1TT.adder * (3000-cmd.pwr>1500 ? 1: -1)));
        vert2T.writeMicroseconds(3000-cmd.pwr+(v2TT.adder * (3000-cmd.pwr>1500 ? 1: -1)));
        break;
     case 't'://set trim
        switch(cmd.pwr%10){
            case 1:
              v1TT.adder = cmd.pwr/10;
              break;
            case 2:
              v2TT.adder = cmd.pwr/10;
              break;
            case 3:
              fTT.adder = cmd.pwr/10;
              break;
            case 4:
              rTT.adder = cmd.pwr/10;
              break;
            case 5:
              bTT.adder = cmd.pwr/10;
              break;
            case 6:
              lTT.adder = cmd.pwr/10;
              break;
        }
        break;
     case 'i'://set inverse (multiplier)
         switch(cmd.pwr/1000){
            case 1:
              v1TT.multiplier = cmd.pwr-((cmd.pwr/1000)*1000); //if cmd.pwr = 1355, then (cmd.pwr/1000)*1000 = 1000, not 1355. Logic.
              break;
            case 2:
              v2TT.multiplier = cmd.pwr-((cmd.pwr/1000)*1000);
              break;
            case 3:
              rTT.multiplier = cmd.pwr-((cmd.pwr/1000)*1000);
              break;
            case 4:
              bTT.multiplier = cmd.pwr-((cmd.pwr/1000)*1000);
              break;
            case 5:
              lTT.multiplier = cmd.pwr-((cmd.pwr/1000)*1000);
              break;
            case 6:
              fTT.multiplier = cmd.pwr-((cmd.pwr/1000)*1000);
              break;
        }
        break;
     case ','://claw open
        if(clawPos<180){
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

     case 'c'://cam up
        if(camPos<180){
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
