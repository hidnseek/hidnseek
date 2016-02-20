#Arduino Hardware, source code and libraries
##Hardware : changes and needs for Arduino IDE 1.6.x

How to upload new firmware in Hidnsek ?

First install Arduino IDE (https://www.arduino.cc/en/Main/Software)
If Arduino IDE already installed, leave/exit the software.

Second, find in the installation directory the boards.txt file

* /Users/<USERNAME>/Library/Arduino/preferences.txt (Mac)
* c:\Documents and Settings\<USERNAME>\Application Data\Arduino\preferences.txt (Windows XP)
* c:\Users\<USERNAME>\AppData\Roaming\Arduino\preferences.txt (Windows Vista)
* ~/.arduino/preferences.txt (Linux)

Insert at the end of this file the description of HidnSeek from [boards.txt](Hardware/boards.txt) 

Launch Arduino IDE and go to the board selection and select HidnSeek at 12MHz

[ArduinoIDE](ArduinoIDE.png)

##HidnSeek_v3_28 : Arduino Sketch of the 3.28 release board
##libraries : librairies used to compile the sketch
