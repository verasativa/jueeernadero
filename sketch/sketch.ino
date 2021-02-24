#include "SCD30.h"
#include "Seeed_SHT35.h"
#include "SafeString.h"
#include <arduino-timer.h>

auto timer = timer_create_default();

#if defined(ARDUINO_ARCH_AVR)
    #pragma message("Defined architecture for ARDUINO_ARCH_AVR.")
    #define SERIAL Serial
#elif defined(ARDUINO_ARCH_SAM)
    #pragma message("Defined architecture for ARDUINO_ARCH_SAM.")
    #define SERIAL SerialUSB
#elif defined(ARDUINO_ARCH_SAMD)
    #pragma message("Defined architecture for ARDUINO_ARCH_SAMD.")
    #define SERIAL SerialUSB
#elif defined(ARDUINO_ARCH_STM32F4)
    #pragma message("Defined architecture for ARDUINO_ARCH_STM32F4.")
    #define SERIAL SerialUSB
#else
    #pragma message("Not found any architecture.")
    #define SERIAL Serial
#endif

/*SAMD core*/
#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
    #define SDAPIN  20
    #define SCLPIN  21
    #define RSTPIN  7
#else
    #define SDAPIN  A4
    #define SCLPIN  A5
    #define RSTPIN  2
#endif

SHT35 sensor(SCLPIN);

const size_t maxCmdLength = 6; // make SafeStrings at least large enough to hold longest cmd
// Use SafeStrings for the commands as comparing two SafeStrings is generally faster as the lengths can be compared first.
createSafeString(relayCmdStr, maxCmdLength, "relay");
createSafeString(relayActivateCmdStr, maxCmdLength, "start");
createSafeString(relayStopCmdStr, maxCmdLength, "stop");
createSafeString(relayStatusCmdStr, maxCmdLength, "status");

// input must be large enough to hold longest cmd + 1 delimiter
createSafeString(input, 15 + 1 + 32); // len('relay xx status') => 15
createSafeString(fullCommand, 15 + 1);
createSafeString(mainCommand, 5 + 1); // len('relay') => 5
createSafeString(commandTarget, 2 + 1); // 2 digits targets
createSafeString(subCommand, 6 + 1); // len('status') => 6
int intCommandTarget;


char cmdDelimiters[] = "\n"; // space dot comma CR NL are cmd delimiters
char subCmdDelimiters[] = " "; // space dot comma CR NL are cmd delimiters


void readSensors(){
    float result[3] = {0};

    if (scd30.isAvailable()) {
        scd30.getCarbonDioxideConcentration(result);
        SERIAL.print("internal:");
        SERIAL.print("co2: ");
        SERIAL.print(result[0]);
        SERIAL.print(" ppm");
        SERIAL.print(", ");
        SERIAL.print("temperature = ");
        SERIAL.print(result[1]);
        SERIAL.print(", humidity = ");
        SERIAL.print(result[2]);
        SERIAL.print(" %");
        SERIAL.println(" ");
    } 

    
    u16 value = 0;
    u8 data[6] = {0};
    float temp, hum;
    if (NO_ERROR != sensor.read_meas_data_single_shot(HIGH_REP_WITH_STRCH, &temp, &hum)) {
        SERIAL.println("read temp failed!!");
        SERIAL.println("   ");
        SERIAL.println("   ");
        SERIAL.println("   ");
    } else {
        SERIAL.print("external:");
        SERIAL.print(" temperature = ");
        SERIAL.print(temp);
        SERIAL.print(", humidity = ");
        SERIAL.print(hum);
        SERIAL.print(" % ");
        SERIAL.println("   ");
    }
    int light_measurement = analogRead(A0);
    // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
    int lux = round(light_measurement * (1000 / 1023.0));
    // print out the value you read:
    SERIAL.print("luminance: ");
    SERIAL.print(lux);
    SERIAL.print(" lux");
    SERIAL.println(" ");
  
}

void setup() {
    Wire.begin();
    SERIAL.begin(115200);
    scd30.initialize();
    pinMode(22, OUTPUT);
    digitalWrite(22, HIGH); 
    pinMode(24, OUTPUT);
    digitalWrite(24, HIGH); 
    pinMode(26, OUTPUT);
    digitalWrite(26, HIGH); 
    pinMode(28, OUTPUT);
    digitalWrite(28, HIGH); 
    pinMode(30, OUTPUT);
    digitalWrite(30, HIGH); 
    pinMode(32, OUTPUT);
    digitalWrite(32, HIGH); 
    pinMode(34, OUTPUT);
    digitalWrite(34, HIGH); 
    pinMode(36, OUTPUT);
    digitalWrite(36, HIGH);

    timer.every(2000, readSensors);
}

void loop() {
    timer.tick();

    input.read(Serial);
    if (input.nextToken(fullCommand, cmdDelimiters)) { // process at most one token per loop does not return tokens longer than input.capacity()
      //token.debug("after nextToken => ");
      //Serial.println(fullCommand);
      fullCommand.nextToken(mainCommand, subCmdDelimiters);
      fullCommand.nextToken(commandTarget, subCmdDelimiters);
      if (mainCommand == relayCmdStr){
        commandTarget.toInt(intCommandTarget);
        //Serial.println(intCommandTarget);
        if (fullCommand.trim() == relayActivateCmdStr){
          //Serial.println(F( "starting a fucking relay"));
          digitalWrite(intCommandTarget, LOW); 
        } else if (fullCommand.trim() == relayStopCmdStr){
          digitalWrite(intCommandTarget, HIGH); 
        } else if (fullCommand.trim() == relayStatusCmdStr){
          if (digitalRead(intCommandTarget) == 0){
            Serial.print("relay ");
            Serial.print(commandTarget);
            Serial.print(": On");
            Serial.println();
          } else if (digitalRead(intCommandTarget) == 1){
            Serial.print("relay ");
            Serial.print(commandTarget);
            Serial.print(": Off");
            Serial.println();
          }
//          Serial.println(" mirando status");
//          Serial.println("");
//          Serial.println(digitalRead(intCommandTarget));
//          Serial.println("");
//          Serial.println("");
        }
      }
    } else {
      Serial.print(input);
      input = "";
    }
    delay(1000);
    //
}
