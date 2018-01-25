# ROV-Control-System

I got a lot of cleaning up to do with the source.

/src/main:
Android App src that connects to the Windows Form Application via a TCP connection. Sends "command" based on accelerometer readings to Forms Application which then sends the command to Arduino via USB. Will also later recieve a camera feed from on-board cameras.

/WindowsFormApplication1
Connects to Arduino via USB and also opens up TCP port 1738 for the aforementioned Android app to connect. Sends movement commands based on mouse and keyboard controls and forwards camera tilt comands to Arduino from Android.

SerialInput.ino

Arduino code for recieving commands over Serial and changing outputs accordingly.
