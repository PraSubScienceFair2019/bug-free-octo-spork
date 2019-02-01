    // Based off of user JohnChi's sketch.
    // By Pranav Subramanian, 2018-2019
    #include<Wire.h>
    #include <SD.h>

    File myFile;

    const int MPU_addr=0x68;  
    double AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ; 
    double AC;
    double jitter;
    double oldVal;
    bool calibrating;

    int redPin= 7;
    int greenPin = 8;
    int bluePin = 9;
    
    int fileNum = 1;
    long clocktime;
    
    void setup(){    
      clocktime = millis();
      
      Wire.begin();
      Wire.beginTransmission(MPU_addr);
      Wire.write(0x6B);  
      Wire.write(0);     
      Wire.endTransmission(true);
      
      Serial.begin(9600);
      while (!Serial) {
        ;
      }
      Serial.print(F("Initializing SD card..."));

      if (!SD.begin(4)) {
        Serial.println(F("initialization failed!"));
        while (1);
      }
      Serial.println(F("initialization done."));
    
      
      while(SD.exists("cd" + (String)fileNum + ".txt")){
        fileNum++;
      }

      Serial.print("cd" + (String)fileNum + ".txt");
      
      myFile = SD.open("cd" + (String)fileNum + ".txt", FILE_WRITE);
    
      if (myFile) {
        Serial.print(F("Writing to file..."));
        myFile.print(F("Initialize at "));
        myFile.print(millis()-clocktime);
        myFile.println(F(" millis."));//split into separate lines to reduce memory usage.
        Serial.println(F("done."));
      } else {
        Serial.println(F("error opening test.txt"));
      }
      
      jitter = 0.1;
      oldVal = 16384;

      pinMode(redPin, OUTPUT);
      pinMode(greenPin, OUTPUT);
      pinMode(bluePin, OUTPUT);


      //setup for "Base accelerations", if necessary.
      calibrating = true;
      
      
      
    }
    void loop(){
      //Serial.print("here");
      
      Wire.beginTransmission(MPU_addr);
      Wire.write(0x3B);  
      Wire.endTransmission(false);
      Wire.requestFrom(MPU_addr,14,true);  


      if(calibrating) {
        double baseX = Wire.read()<<8|Wire.read();
        double baseY = Wire.read()<<8|Wire.read();
        double baseZ = Wire.read()<<8|Wire.read();
        int counter = 2;
        while(millis() - clocktime < 10000) {
          //trying not to create too many variables just in case, even though doubles are pretty small compared to strings
          baseX = (baseX*(counter-1) + (double)(Wire.read()<<8|Wire.read()))/counter;//weighted average
          baseY = (baseY*(counter-1) + (double)(Wire.read()<<8|Wire.read()))/counter;
          baseZ = (baseZ*(counter-1) + (double)(Wire.read()<<8|Wire.read()))/counter;
          counter++;
        }
  
        myFile.print(F("BaseX: ")); //did this as opposed to all in one line to avoid invokation of the String class, which allows for more memory available for local variables and less error!
        myFile.print(baseX/16384);
        myFile.print(F(", BaseY: "));
        myFile.print(baseY/16384);
        myFile.print(F(", BaseZ: "));
        myFile.print(baseZ/16384);
        calibrating = false;
      }

      
      AcX=Wire.read()<<8|Wire.read();  
      AcY=Wire.read()<<8|Wire.read();  
      AcZ=Wire.read()<<8|Wire.read();  //these arent converted to g's.
      
      AC = sqrt((AcX/16384)*(AcX/16384) + (AcY/16384)*(AcY/16384) + (AcZ/16384)*(AcZ/16384) - 9.8*9.8);
      if(oldVal + jitter <= AC || oldVal-jitter >= AC) {
        clocktime = millis(); //gotta update the time!
        myFile.print(F("Acceleration = ")); myFile.print(AC); myFile.print(F("g; ACx = ")); myFile.print(AcX); myFile.print(F(", ACy = ")); myFile.print(AcY); myFile.print(F(", ACz = ")); myFile.print(AcZ); myFile.println(F("; ")); myFile.println("at " + (String)clocktime + " ms.");
        Serial.print(F("Acceleration = ")); Serial.print(AC); Serial.print(F("g; at ")); Serial.print(clocktime); Serial.println(F(" ms.")); 
        if (AC+jitter < 1.5){
          setColor(0,0,0);
        }
      }

      switch(int(AC)) {
        case 2: //45-64g
          setColor(74, 191, 0);
          delay(1000);
          myFile.print(F("Mildly/Possibly Concussed. Acceleration was ")); myFile.print(AC); myFile.print(F(". Timestamp of ")); myFile.print(clocktime); myFile.println(F(" ms."));
          break;
        case 3: //65-84g
          setColor(255, 255, 0);
          delay(1000);
          myFile.print(F("Very Likely Concussed. Acceleration was ")); myFile.print(AC); myFile.print(F(". Timestamp of ")); myFile.print(clocktime); myFile.println(F(" ms."));
          break;
        case 4: //85-120g
          setColor(255, 0, 0);
          delay(1000);
          myFile.print(F("Definitely Concussed. Acceleration was ")); myFile.print(AC); myFile.print(F(". Timestamp of ")); myFile.print(clocktime); myFile.println(F(" ms."));
          break;
        case 5: //121-150g
          setColor(140, 0, 255);
          delay(1000);
          myFile.print(F("Definitely Concussed, likely seriously so. Acceleration was ")); myFile.print(AC); myFile.print(F(". Timestamp of ")); myFile.print(clocktime); myFile.println(F(" ms."));
          break;
        default:
          break;
      }
      oldVal = AC;

      myFile.close();
      myFile = SD.open("cd" + (String)fileNum + ".txt", FILE_WRITE);
      
      delay(10); 
      
      if(Serial.available()){//so if you send a command to it it'll stop! Maybe update to something that uses wireless in the future, using a PCB with more input ports
        Serial.print(F("here"));
        exit(0);
      }
    }

    void setColor(int redValue, int greenValue, int blueValue) {
      analogWrite(redPin, redValue);
      analogWrite(greenPin, greenValue);
      analogWrite(bluePin, blueValue);
    }
