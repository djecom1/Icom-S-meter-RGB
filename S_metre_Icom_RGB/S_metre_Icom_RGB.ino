// IC7300 Testbed for S-meter readout and other functions
// by Luc Decroos - ON7DQ/KF0CR
// modified & adapted by Daniel VE2BAP, 2018-12-01
// modified & adapted by Jerome F4GMU, 2022-10-16

#include <SoftwareSerial.h> // for comms to IC7000
#define BAUD_RATE 19200     // CI-V speed
#define TRX_address (0x88)  // HEX $70 = Icom IC-7000
//#define TRX_address ((byte)00)  // $00: Icom universal address (works for all radios).

// serial connection
// RX = Icom radio to Arduino  : to pin 2 via resistor 4k7
// TX = Arduino to Icom radio  : to pin 7 via diode 1N4148, with pull up 10k to Vcc (5V) on tip of 3.5 mm connector

SoftwareSerial mySerial = SoftwareSerial(2, 7); // (RX, TX)

int readCounter; // counts the number of bytes received from the radio
int sMeterVal1;  // stores the most  significant BCD byte containing signal info.
int sMeterVal2;  // stores the least significant BCD byte containing signal info.
int trxVal1;     // TRX info
int sMeterOut = 11; // External analog S-meter connected to pin 11.

const int sPwr = 8; // Constante pour la broche 8
const int sSwr = 9; // Constante pour la broche 9
const int sAlc = 10; // Constante pour la broche 10
const int sComp = 4; // Constante pour la broche 4
const int sVolt = 12; // Constante pour la broche 12

/*int pwrState;
int swrState;
int alcState;
int compState;
int voltState;*/

// Const RGB
void ledRVBpwm(int pwmRouge, int pwmVert, int pwmBleu);

const int ledRouge = 3; // Constante pour la broche 3
const int ledVert = 5; // Constante pour la broche 5
const int ledBleu = 6; // Constante pour la broche 6

int tension, val;


//---------------------------------------------------------------------------------------------

void setup()
{
  pinMode(13, OUTPUT);  // force LED (pin 13) to turn off.

  pinMode(2, INPUT);  // CI-V serial communication from IC7000
  pinMode(7, OUTPUT); // CI-V serial communication to IC7000
  pinMode(sMeterOut, OUTPUT); // set sMeterPin for output

  pinMode(sPwr, INPUT_PULLUP);  // Switch power (pin 8)
  pinMode(sSwr, INPUT_PULLUP);  // Switch SWR (pin 9)
  pinMode(sAlc, INPUT_PULLUP);  // Switch ALC (pin 10)
  pinMode(sComp, INPUT_PULLUP);  // Switch Comp. (pin 4)
  pinMode(sVolt, INPUT_PULLUP);  // Switch Volt (pin 12)

  pinMode (ledVert, OUTPUT);  //LED verte
  pinMode (ledRouge, OUTPUT); //LED rouge
  pinMode (ledBleu, OUTPUT);  //LED bleu

  analogWrite(sMeterOut, 200); // Test meter
  delay(1000);
  analogWrite(sMeterOut, 100);
  delay(1000);
  analogWrite(sMeterOut, 0);
  delay(1000);

  mySerial.begin(BAUD_RATE);
  mySerial.listen();  // only one port can be made to listen with software serial
  // see reference https://www.arduino.cc/en/Reference/SoftwareSerialListen
  while (mySerial.available()) mySerial.read(); // clean buffer
}

//---------------------------------------------------------------------------------------------

void loop() {
  
  
  mySerial.flush();

  // start sequence: send "read TRX status" command to radio.
  mySerial.write(0xFE); mySerial.write(0xFE); mySerial.write(TRX_address); mySerial.write(0xE0);
  mySerial.write(0x1C); mySerial.write((byte)00); // Read Transmit ON/OFF , command 1C 00
  mySerial.write(0xFD); // end sequence
  delay(20);

  // now read info from radio
  int nbChar = mySerial.available();

  if (nbChar > 0) {
    for (int readCounter = 0; readCounter < nbChar ; readCounter++) {
      byte byteRead = mySerial.read();

      if (readCounter == 6) {
        trxVal1 = ( (byteRead / 16 * 10) + (byteRead % 16) ); // First byte: convert from BCD to decimal.
      }
    }
  }

  if (trxVal1 == 0) { // Si TX = OFF
    digitalWrite(13, LOW);
    // Lecture/affichage S-meter
    sMeter();
  }
  else if ((trxVal1 != 0) && (digitalRead(sPwr) == LOW)) { // Si TX = ON et Switch Pwr = ON
    digitalWrite(13, HIGH);   // turn the LED on
    // Lecture/affichage Power-meter
    powerMeter();
  }

  else if ((trxVal1 != 0) && (digitalRead(sSwr) == LOW)) { // Si TX = ON et Switch SWR = ON
    digitalWrite(13, HIGH);   // turn the LED on
    // Lecture/affichage Power-meter
    swrMeter();
  }

  else if ((trxVal1 != 0) && (digitalRead(sAlc) == LOW)) { // Si TX = ON et Switch ALC = ON
    digitalWrite(13, HIGH);   // turn the LED on
    // Lecture/affichage ALC-meter
    alcMeter();
  }

  else if ((trxVal1 != 0) && (digitalRead(sComp) == LOW)) { // Si TX = ON et Switch Comp = ON
    digitalWrite(13, HIGH);   // turn the LED on
    // Lecture/affichage Comp-meter
    compMeter();
  }

  else if ((trxVal1 != 0) && (digitalRead(sVolt) == LOW)) { // Si TX = ON et Switch Volt = ON
    digitalWrite(13, HIGH);   // turn the LED on
    // Lecture/affichage Volt-meter
    voltMeter();
  }

  // Routine RGB
  tension = analogRead(A0);

  val = map(tension, 0, 1023, 0, 2 * 255);

  if ( val < 255)
    ledRVBpwm(0, val, 255 - val);

  if ( val >= 255 && val <= 2 * 255)
    ledRVBpwm(val - 255, 255 - (val - 255), 0);
}

void ledRVBpwm(int pwmRouge, int pwmVert, int pwmBleu) { // reçoit valeur 0-255 par couleur

  analogWrite(ledRouge, pwmRouge);
  analogWrite(ledVert, pwmVert);
  analogWrite(ledBleu, pwmBleu);
}

void sMeter() {
    mySerial.flush();

    // start sequence: send "read S meter" command to radio.
    mySerial.write(0xFE); mySerial.write(0xFE); mySerial.write(TRX_address); mySerial.write(0xE0);
    mySerial.write(0x15); mySerial.write(0x02); // Read s-meter , command 15 02
    mySerial.write(0xFD); // end sequence
    delay(20);

    // now read info from radio
    int nbChar = mySerial.available();

    if (nbChar > 0) {
      for (int readCounter = 0; readCounter < nbChar ; readCounter++) {
        byte byteRead = mySerial.read();

        if (readCounter == 6) {
          sMeterVal1 = ( (byteRead / 16 * 10) + (byteRead % 16) ); // First byte: convert from BCD to decimal.
        }

        if (readCounter == 7) {
          sMeterVal2 = ( (byteRead / 16 * 10) + (byteRead % 16) ); // Second byte: convert from BCD to decimal.

          analogWrite(sMeterOut, ((sMeterVal1 * 100) + sMeterVal2)); // Calculate and write the S-meter value on the S-meter output pin.
          delay(20);
        }
      }
    }
  }

void powerMeter() {
    mySerial.flush();

    // start sequence: send "read S meter" command to radio.
    mySerial.write(0xFE); mySerial.write(0xFE); mySerial.write(TRX_address); mySerial.write(0xE0);
    mySerial.write(0x15); mySerial.write(0x11); // Read power-meter , command 15 11
    mySerial.write(0xFD); // end sequence
    delay(20);

    // now read info from radio
    int nbChar = mySerial.available();

    if (nbChar > 0) {
      for (int readCounter = 0; readCounter < nbChar ; readCounter++) {
        byte byteRead = mySerial.read();

        if (readCounter == 6) {
          sMeterVal1 = ( (byteRead / 16 * 10) + (byteRead % 16) ); // First byte: convert from BCD to decimal.
        }

        if (readCounter == 7) {
          sMeterVal2 = ( (byteRead / 16 * 10) + (byteRead % 16) ); // Second byte: convert from BCD to decimal.

          analogWrite(sMeterOut, ((sMeterVal1 * 100) + sMeterVal2)); // Calculate and write the S-meter value on the S-meter output pin.
          delay(20);
        }
      }
    }
  }

void swrMeter() {
    mySerial.flush();

    // start sequence: send "read S meter" command to radio.
    mySerial.write(0xFE); mySerial.write(0xFE); mySerial.write(TRX_address); mySerial.write(0xE0);
    mySerial.write(0x15); mySerial.write(0x12); // Read SWR-meter , command 15 12
    mySerial.write(0xFD); // end sequence
    delay(20);

    // now read info from radio
    int nbChar = mySerial.available();

    if (nbChar > 0) {
      for (int readCounter = 0; readCounter < nbChar ; readCounter++) {
        byte byteRead = mySerial.read();

        if (readCounter == 6) {
          sMeterVal1 = ( (byteRead / 16 * 10) + (byteRead % 16) ); // First byte: convert from BCD to decimal.
        }

        if (readCounter == 7) {
          sMeterVal2 = ( (byteRead / 16 * 10) + (byteRead % 16) ); // Second byte: convert from BCD to decimal.

          analogWrite(sMeterOut, ((sMeterVal1 * 100) + sMeterVal2)); // Calculate and write the S-meter value on the S-meter output pin.
          delay(20);
        }
      }
    }
  }

void alcMeter() {
    mySerial.flush();

    // start sequence: send "read S meter" command to radio.
    mySerial.write(0xFE); mySerial.write(0xFE); mySerial.write(TRX_address); mySerial.write(0xE0);
    mySerial.write(0x15); mySerial.write(0x13); // Read ALC-meter , command 15 13
    mySerial.write(0xFD); // end sequence
    delay(20);

    // now read info from radio
    int nbChar = mySerial.available();

    if (nbChar > 0) {
      for (int readCounter = 0; readCounter < nbChar ; readCounter++) {
        byte byteRead = mySerial.read();

        if (readCounter == 6) {
          sMeterVal1 = ( (byteRead / 16 * 10) + (byteRead % 16) ); // First byte: convert from BCD to decimal.
        }

        if (readCounter == 7) {
          sMeterVal2 = ( (byteRead / 16 * 10) + (byteRead % 16) ); // Second byte: convert from BCD to decimal.

          analogWrite(sMeterOut, ((sMeterVal1 * 100) + sMeterVal2)); // Calculate and write the S-meter value on the S-meter output pin.
          delay(20);
        }
      }
    }
  }

void compMeter() {
    mySerial.flush();

    // start sequence: send "read S meter" command to radio.
    mySerial.write(0xFE); mySerial.write(0xFE); mySerial.write(TRX_address); mySerial.write(0xE0);
    mySerial.write(0x15); mySerial.write(0x14); // Read COMP-meter , command 15 14
    mySerial.write(0xFD); // end sequence
    delay(20);

    // now read info from radio
    int nbChar = mySerial.available();

    if (nbChar > 0) {
      for (int readCounter = 0; readCounter < nbChar ; readCounter++) {
        byte byteRead = mySerial.read();

        if (readCounter == 6) {
          sMeterVal1 = ( (byteRead / 16 * 10) + (byteRead % 16) ); // First byte: convert from BCD to decimal.
        }

        if (readCounter == 7) {
          sMeterVal2 = ( (byteRead / 16 * 10) + (byteRead % 16) ); // Second byte: convert from BCD to decimal.

          analogWrite(sMeterOut, ((sMeterVal1 * 100) + sMeterVal2)); // Calculate and write the S-meter value on the S-meter output pin.
          delay(20);
        }
      }
    }
  }

void voltMeter() {
    mySerial.flush();

    // start sequence: send "read S meter" command to radio.
    mySerial.write(0xFE); mySerial.write(0xFE); mySerial.write(TRX_address); mySerial.write(0xE0);
    mySerial.write(0x15); mySerial.write(0x15); // Read Volt-meter , command 15 15
    mySerial.write(0xFD); // end sequence
    delay(20);

    // now read info from radio
    int nbChar = mySerial.available();

    if (nbChar > 0) {
      for (int readCounter = 0; readCounter < nbChar ; readCounter++) {
        byte byteRead = mySerial.read();

        if (readCounter == 6) {
          sMeterVal1 = ( (byteRead / 16 * 10) + (byteRead % 16) ); // First byte: convert from BCD to decimal.
        }

        if (readCounter == 7) {
          sMeterVal2 = ( (byteRead / 16 * 10) + (byteRead % 16) ); // Second byte: convert from BCD to decimal.

          analogWrite(sMeterOut, ((sMeterVal1 * 100) + sMeterVal2)); // Calculate and write the S-meter value on the S-meter output pin.
          delay(20);
        }
      }
    }
  }

void blink() {
  digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(200);               // wait for a second
  digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
  delay(200);               // wait for a second
}
