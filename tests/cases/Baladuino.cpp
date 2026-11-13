# 1 "/tmp/ctags-capture/sketches/Baladuino/Baladuino.ino"
/*
 * The code is released under the GNU General Public License.
 * Developed by Kristian Lauszus, TKJ Electronics 2013
 * This is the algorithm for the Balanduino balancing robot.
 * It can be controlled by either an Android app or a Processing application via Bluetooth.
 * The Android app can be found at the following link: https://github.com/TKJElectronics/BalanduinoAndroidApp
 * The Processing application can be found here: https://github.com/TKJElectronics/BalanduinoProcessingApp
 * It can also be controlled by a PS3, Wii or a Xbox controller
 * For details, see: http://balanduino.net/
 */

/* Use this to enable and disable the different options */







# 21 "/tmp/ctags-capture/sketches/Baladuino/Baladuino.ino" 2
# 22 "/tmp/ctags-capture/sketches/Baladuino/Baladuino.ino" 2
# 23 "/tmp/ctags-capture/sketches/Baladuino/Baladuino.ino" 2


# 26 "/tmp/ctags-capture/sketches/Baladuino/Baladuino.ino" 2


// These are all open source libraries written by Kristian Lauszus, TKJ Electronics
// The USB libraries are located at the following link: https://github.com/felis/USB_Host_Shield_2.0
# 31 "/tmp/ctags-capture/sketches/Baladuino/Baladuino.ino" 2


# 34 "/tmp/ctags-capture/sketches/Baladuino/Baladuino.ino" 2


# 37 "/tmp/ctags-capture/sketches/Baladuino/Baladuino.ino" 2


# 40 "/tmp/ctags-capture/sketches/Baladuino/Baladuino.ino" 2


# 43 "/tmp/ctags-capture/sketches/Baladuino/Baladuino.ino" 2


// Create the Kalman library instance
Kalman kalman; // See https://github.com/TKJElectronics/KalmanFilter for source code



USB Usb; // This will take care of all USB communication



// Implementation for the Android Open Accessory Protocol. Simply connect your phone to get redirected to the Play Store
ADK adk(&Usb, "TKJ Electronics", // Manufacturer Name
              "Balanduino", // Model Name
              "Android App for Balanduino", // Description - user visible string
              "0.5.0", // Version of the Android app
              "https://play.google.com/store/apps/details?id=com.tkjelectronics.balanduino", // URL - web page to visit if no installed apps support the accessory
              "1234"); // Serial Number - this is not used



XBOXRECV Xbox(&Usb); // You have to connect a Xbox wireless receiver to the Arduino to control it with a wireless Xbox controller



USBHub Hub(&Usb); // Some dongles have a hub inside
BTD Btd(&Usb); // This is the main Bluetooth library, it will take care of all the USB and HCI communication with the Bluetooth dongle



SPP SerialBT(&Btd, "Balanduino", "0000"); // The SPP (Serial Port Protocol) emulates a virtual Serial port, which is supported by most computers and mobile phones



PS3BT PS3(&Btd); // The PS3 library supports all three official controllers: the Dualshock 3, Navigation and Move controller



WII Wii(&Btd); // The Wii library can communicate with Wiimotes and the Nunchuck and Motion Plus extension and finally the Wii U Pro Controller
//WII Wii(&Btd,PAIR); // You will have to pair with your Wiimote first by creating the instance like this and the press 1+2 on the Wiimote
// or press sync if you are using a Wii U Pro Controller
// Or you can simply send "CW;" to the robot to start the pairing sequence
// This can also be done using the Android or Processing application


void setup() {
  /* Initialize UART */
  Serial.begin(115200);

  /* Read the PID values, target angle and other saved values in the EEPROM */
  if (!checkInitializationFlags())
    readEEPROMValues(); // Only read the EEPROM values if they have not been restored

  /* Setup encoders */
  pinMode(leftEncoder1, INPUT);
  pinMode(leftEncoder2, INPUT);
  pinMode(rightEncoder1, INPUT);
  pinMode(rightEncoder2, INPUT);
  attachInterrupt(0, leftEncoder, CHANGE);
  attachInterrupt(1, rightEncoder, CHANGE);

  /* Enable the motor drivers */
  pinMode(leftEnable, OUTPUT);
  pinMode(rightEnable, OUTPUT);
  digitalWrite(leftEnable, HIGH);
  digitalWrite(rightEnable, HIGH);

  /* Setup motor pins to output */
  sbi(pwmPortDirection, leftPWM);
  sbi(leftPortDirection, leftA);
  sbi(leftPortDirection, leftB);
  sbi(pwmPortDirection, rightPWM);
  sbi(rightPortDirection, rightA);
  sbi(rightPortDirection, rightB);

  /* Set PWM frequency to 20kHz - see the datasheet http://www.atmel.com/Images/doc8272.pdf page 128-135 */
  // Set up PWM, Phase and Frequency Correct on pin 18 (OC1A) & pin 17 (OC1B) with ICR1 as TOP using Timer1
  TCCR1B = _BV(WGM13) | _BV(CS10); // Set PWM Phase and Frequency Correct with ICR1 as TOP and no prescaling
  ICR1 = PWMVALUE; // ICR1 is the TOP value - this is set so the frequency is equal to 20kHz

  /* Enable PWM on pin 18 (OC1A) & pin 17 (OC1B) */
  // Clear OC1A/OC1B on compare match when up-counting
  // Set OC1A/OC1B on compare match when downcounting
  TCCR1A = _BV(COM1A1) | _BV(COM1B1);
  setPWM(leftPWM, 0); // Turn off PWM on both pins
  setPWM(rightPWM, 0);

  /* Setup buzzer pin */
  pinMode(buzzer, OUTPUT);


  if (Usb.Init() == -1) { // Check if USB Host is working
    Serial.print((reinterpret_cast<const __FlashStringHelper *>(("OSC did not start"))));
    digitalWrite(buzzer, HIGH);
    while (1); // Halt
  }


  /* Attach onInit function */
  // This is used to set the LEDs according to the voltage level and vibrate the controller to indicate the new connection

  PS3.attachOnInit(onInit);


  Wii.attachOnInit(onInit);


  Xbox.attachOnInit(onInit);


  /* Setup IMU */
  Wire.begin();

  while (i2cRead(0x75, i2cBuffer, 1));
  if (i2cBuffer[0] != 0x68) { // Read "WHO_AM_I" register
    Serial.print((reinterpret_cast<const __FlashStringHelper *>(("Error reading sensor"))));
    digitalWrite(buzzer, HIGH);
    while (1); // Halt
  }

  i2cBuffer[0] = 19; // Set the sample rate to 400Hz - 8kHz/(19+1) = 400Hz
  i2cBuffer[1] = 0x00; // Disable FSYNC and set 260 Hz Acc filtering, 256 Hz Gyro filtering, 8 KHz sampling
  i2cBuffer[2] = 0x00; // Set Gyro Full Scale Range to ±250deg/s
  i2cBuffer[3] = 0x00; // Set Accelerometer Full Scale Range to ±2g
  while (i2cWrite(0x19, i2cBuffer, 4, false)); // Write to all four registers at once
  while (i2cWrite(0x6B, 0x09, true)); // PLL with X axis gyroscope reference, disable temperature sensor and disable sleep mode

  delay(100); // Wait for the sensor to get ready

  /* Set Kalman and gyro starting angle */
  while (i2cRead(0x3D, i2cBuffer, 4));
  accY = ((i2cBuffer[0] << 8) | i2cBuffer[1]);
  accZ = ((i2cBuffer[2] << 8) | i2cBuffer[3]);
  // atan2 outputs the value of -π to π (radians) - see http://en.wikipedia.org/wiki/Atan2
  // We then convert it to 0 to 2π and then from radians to degrees
  accAngle = (atan2((double)accY - cfg.accYzero, (double)accZ - cfg.accZzero) + 3.1415926535897932384626433832795) * 57.295779513082320876798154814105;

  kalman.setAngle(accAngle); // Set starting angle
  pitch = accAngle;
  gyroAngle = accAngle;

  /* Find gyro zero value */
  calibrateGyro();

  pinMode(((((1476535296U == 1476527104U) && (13 == 7)) ? 1 : 0) ? 0 : 0) + ((((1476535296U == 1476526080U) && (13 == 9)) ? 1 : 0) ? 1 : 0) + ((((1476535296U == 1476526080U) && (13 == 3)) ? 1 : 0) ? 2 : 0) + ((((1476535296U == 1476526080U) && (13 == 2)) ? 1 : 0) ? 3 : 0) + ((((1476535296U == 1476535296U) && (13 == 8)) ? 1 : 0) ? 4 : 0) + ((((1476535296U == 1476526080U) && (13 == 7)) ? 1 : 0) ? 5 : 0) + ((((1476535296U == 1476529152U) && (13 == 13)) ? 1 : 0) ? 6 : 0) + ((((1476535296U == 1476527104U) && (13 == 4)) ? 1 : 0) ? 7 : 0) + ((((1476535296U == 1476527104U) && (13 == 8)) ? 1 : 0) ? 8 : 0) + ((((1476535296U == 1476527104U) && (13 == 9)) ? 1 : 0) ? 9 : 0) + ((((1476535296U == 1476536320U) && (13 == 1)) ? 1 : 0) ? 10 : 0) + ((((1476535296U == 1476535296U) && (13 == 10)) ? 1 : 0) ? 11 : 0) + ((((1476535296U == 1476535296U) && (13 == 11)) ? 1 : 0) ? 12 : 0) + ((((1476535296U == 1476533248U) && (13 == 6)) ? 1 : 0) ? 13 : 0) + ((((1476535296U == 1476532224U) && (13 == 14)) ? 1 : 0) ? 14 : 0) + ((((1476535296U == 1476528128U) && (13 == 7)) ? 1 : 0) ? 15 : 0) + ((((1476535296U == 1476533248U) && (13 == 13)) ? 1 : 0) ? 16 : 0) + ((((1476535296U == 1476534272U) && (13 == 9)) ? 1 : 0) ? 17 : 0) + ((((1476535296U == 1476529152U) && (13 == 5)) ? 1 : 0) ? 18 : 0) + ((((1476535296U == 1476529152U) && (13 == 6)) ? 1 : 0) ? 19 : 0) + ((((1476535296U == 1476527104U) && (13 == 11)) ? 1 : 0) ? 20 : 0) + ((((1476535296U == 1476533248U) && (13 == 4)) ? 1 : 0) ? 21 : 0) + ((((1476535296U == 1476535296U) && (13 == 12)) ? 1 : 0) ? 22 : 0) + ((((1476535296U == 1476532224U) && (13 == 13)) ? 1 : 0) ? 23 : 0) + ((((1476535296U == 1476532224U) && (13 == 12)) ? 1 : 0) ? 24 : 0) + ((((1476535296U == 1476535296U) && (13 == 0)) ? 1 : 0) ? 25 : 0) + ((((1476535296U == 1476535296U) && (13 == 14)) ? 1 : 0) ? 26 : 0) + ((((1476535296U == 1476535296U) && (13 == 1)) ? 1 : 0) ? 27 : 0) + ((((1476535296U == 1476535296U) && (13 == 15)) ? 1 : 0) ? 28 : 0) + ((((1476535296U == 1476535296U) && (13 == 2)) ? 1 : 0) ? 29 : 0) + ((((1476535296U == 1476536320U) && (13 == 3)) ? 1 : 0) ? 30 : 0) + ((((1476535296U == 1476535296U) && (13 == 3)) ? 1 : 0) ? 31 : 0) + ((((1476535296U == 1476536320U) && (13 == 4)) ? 1 : 0) ? 32 : 0) + ((((1476535296U == 1476535296U) && (13 == 4)) ? 1 : 0) ? 33 : 0) + ((((1476535296U == 1476536320U) && (13 == 5)) ? 1 : 0) ? 34 : 0) + ((((1476535296U == 1476535296U) && (13 == 5)) ? 1 : 0) ? 35 : 0) + ((((1476535296U == 1476536320U) && (13 == 6)) ? 1 : 0) ? 36 : 0) + ((((1476535296U == 1476535296U) && (13 == 6)) ? 1 : 0) ? 37 : 0) + ((((1476535296U == 1476535296U) && (13 == 7)) ? 1 : 0) ? 38 : 0) + ((((1476535296U == 1476534272U) && (13 == 14)) ? 1 : 0) ? 39 : 0) + ((((1476535296U == 1476530176U) && (13 == 6)) ? 1 : 0) ? 40 : 0) + ((((1476535296U == 1476536320U) && (13 == 7)) ? 1 : 0) ? 41 : 0) + ((((1476535296U == 1476534272U) && (13 == 15)) ? 1 : 0) ? 42 : 0) + ((((1476535296U == 1476534272U) && (13 == 10)) ? 1 : 0) ? 43 : 0) + ((((1476535296U == 1476532224U) && (13 == 10)) ? 1 : 0) ? 44 : 0) + ((((1476535296U == 1476534272U) && (13 == 13)) ? 1 : 0) ? 45 : 0) + ((((1476535296U == 1476533248U) && (13 == 15)) ? 1 : 0) ? 46 : 0) + ((((1476535296U == 1476527104U) && (13 == 2)) ? 1 : 0) ? 47 : 0) + ((((1476535296U == 1476536320U) && (13 == 0)) ? 1 : 0) ? 48 : 0) + ((((1476535296U == 1476530176U) && (13 == 4)) ? 1 : 0) ? 49 : 0) + ((((1476535296U == 1476534272U) && (13 == 11)) ? 1 : 0) ? 50 : 0) + ((((1476535296U == 1476530176U) && (13 == 5)) ? 1 : 0) ? 51 : 0) + ((((1476535296U == 1476536320U) && (13 == 2)) ? 1 : 0) ? 52 : 0) + ((((1476535296U == 1476532224U) && (13 == 7)) ? 1 : 0) ? 53 : 0) + ((((1476535296U == 1476534272U) && (13 == 5)) ? 1 : 0) ? 54 : 0) + ((((1476535296U == 1476533248U) && (13 == 8)) ? 1 : 0) ? 55 : 0) + ((((1476535296U == 1476526080U) && (13 == 6)) ? 1 : 0) ? 56 : 0) + ((((1476535296U == 1476535296U) && (13 == 9)) ? 1 : 0) ? 57 : 0) + ((((1476535296U == 1476534272U) && (13 == 7)) ? 1 : 0) ? 58 : 0) + ((((1476535296U == 1476534272U) && (13 == 6)) ? 1 : 0) ? 59 : 0) + ((((1476535296U == 1476534272U) && (13 == 4)) ? 1 : 0) ? 60 : 0) + ((((1476535296U == 1476533248U) && (13 == 14)) ? 1 : 0) ? 61 : 0) + ((((1476535296U == 1476532224U) && (13 == 11)) ? 1 : 0) ? 62 : 0) + ((((1476535296U == 1476533248U) && (13 == 11)) ? 1 : 0) ? 63 : 0) + ((((1476535296U == 1476533248U) && (13 == 10)) ? 1 : 0) ? 64 : 0) + ((((1476535296U == 1476533248U) && (13 == 9)) ? 1 : 0) ? 65 : 0) + ((((1476535296U == 1476526080U) && (13 == 1)) ? 1 : 0) ? 66 : 0) + ((((1476535296U == 1476529152U) && (13 == 4)) ? 1 : 0) ? 67 : 0) + ((((1476535296U == 1476528128U) && (13 == 6)) ? 1 : 0) ? 68 : 0) + ((((1476535296U == 1476534272U) && (13 == 0)) ? 1 : 0) ? 69 : 0) + ((((1476535296U == 1476534272U) && (13 == 1)) ? 1 : 0) ? 70 : 0) + ((((1476535296U == 1476534272U) && (13 == 2)) ? 1 : 0) ? 71 : 0) + ((((1476535296U == 1476534272U) && (13 == 3)) ? 1 : 0) ? 72 : 0) + ((((1476535296U == 1476528128U) && (13 == 1)) ? 1 : 0) ? 73 : 0) + ((((1476535296U == 1476527104U) && (13 == 12)) ? 1 : 0) ? 74 : 0) + ((((1476535296U == 1476529152U) && (13 == 3)) ? 1 : 0) ? 75 : 0) + ((((1476535296U == 1476528128U) && (13 == 4)) ? 1 : 0) ? 76 : 0) + ((((1476535296U == 1476528128U) && (13 == 5)) ? 1 : 0) ? 77 : 0) + ((((1476535296U == 1476527104U) && (13 == 0)) ? 1 : 0) ? 78 : 0) + ((((1476535296U == 1476527104U) && (13 == 1)) ? 1 : 0) ? 79 : 0) + ((((1476535296U == 1476528128U) && (13 == 3)) ? 1 : 0) ? 80 : 0) + ((((1476535296U == 1476528128U) && (13 == 2)) ? 1 : 0) ? 81 : 0) + ((((1476535296U == 1476528128U) && (13 == 0)) ? 1 : 0) ? 82 : 0) + ((((1476535296U == 1476526080U) && (13 == 0)) ? 1 : 0) ? 83 : 0) + ((((1476535296U == 1476526080U) && (13 == 4)) ? 1 : 0) ? 84 : 0) + ((((1476535296U == 1476526080U) && (13 == 5)) ? 1 : 0) ? 85 : 0) + ((((1476535296U == 1476534272U) && (13 == 12)) ? 1 : 0) ? 86 : 0) + ((((1476535296U == 1476535296U) && (13 == 13)) ? 1 : 0) ? 87 : 0) + ((((1476535296U == 1476530176U) && (13 == 3)) ? 1 : 0) ? 88 : 0) + ((((1476535296U == 1476532224U) && (13 == 9)) ? 1 : 0) ? 89 : 0) + ((((1476535296U == 1476529152U) && (13 == 7)) ? 1 : 0) ? 90 : 0) + ((((1476535296U == 1476527104U) && (13 == 3)) ? 1 : 0) ? 91 : 0) + ((((1476535296U == 1476526080U) && (13 == 15)) ? 1 : 0) ? 92 : 0) + ((((1476535296U == 1476527104U) && (13 == 5)) ? 1 : 0) ? 93 : 0) + ((((1476535296U == 1476527104U) && (13 == 13)) ? 1 : 0) ? 94 : 0) + ((((1476535296U == 1476527104U) && (13 == 6)) ? 1 : 0) ? 95 : 0) + ((((1476535296U == 1476533248U) && (13 == 12)) ? 1 : 0) ? 96 : 0) + ((((1476535296U == 3735928559U) && (13 == 0)) ? 1 : 0) ? 97 : 0) + ((((1476535296U == 3735928559U) && (13 == 1)) ? 1 : 0) ? 98 : 0) + ((((1476535296U == 3735928559U) && (13 == 2)) ? 1 : 0) ? 99 : 0) + ((((1476535296U == 3735928559U) && (13 == 3)) ? 1 : 0) ? 100 : 0), OUTPUT); // LED_BUILTIN is defined in pins_arduino.h in the hardware add-on

  /* Beep to indicate that it is now ready */
  digitalWrite(buzzer, HIGH);
  delay(100);
  digitalWrite(buzzer, LOW);

  /* Setup timing */
  kalmanTimer = micros();
  pidTimer = kalmanTimer;
  encoderTimer = kalmanTimer;
  imuTimer = millis();
  reportTimer = imuTimer;
  ledTimer = imuTimer;
  blinkTimer = imuTimer;
}

void loop() {

  if (Wii.wiimoteConnected) // We have to read much more often from the Wiimote to decrease latency
    Usb.Task();


  /* Calculate pitch */
  while (i2cRead(0x3D, i2cBuffer, 8));
  accY = ((i2cBuffer[0] << 8) | i2cBuffer[1]);
  accZ = ((i2cBuffer[2] << 8) | i2cBuffer[3]);
  gyroX = ((i2cBuffer[6] << 8) | i2cBuffer[7]);

  // atan2 outputs the value of -π to π (radians) - see http://en.wikipedia.org/wiki/Atan2
  // We then convert it to 0 to 2π and then from radians to degrees
  accAngle = (atan2((double)accY - cfg.accYzero, (double)accZ - cfg.accZzero) + 3.1415926535897932384626433832795) * 57.295779513082320876798154814105;

  uint32_t timer = micros();
  // This fixes the 0-360 transition problem when the accelerometer angle jumps between 0 and 360 degrees
  if ((accAngle < 90 && pitch > 270) || (accAngle > 270 && pitch < 90)) {
    kalman.setAngle(accAngle);
    pitch = accAngle;
    gyroAngle = accAngle;
  } else {
    gyroRate = ((double)gyroX - gyroXzero) / 131.0; // Convert to deg/s
    double dt = (double)(timer - kalmanTimer) / 1000000.0;
    gyroAngle += gyroRate * dt; // Gyro angle is only used for debugging
    if (gyroAngle < 0 || gyroAngle > 360)
      gyroAngle = pitch; // Reset the gyro angle when it has drifted too much
    pitch = kalman.getAngle(accAngle, gyroRate, dt); // Calculate the angle using a Kalman filter
  }
  kalmanTimer = timer;
  //Serial.print(accAngle);Serial.print('\t');Serial.print(gyroAngle);Serial.print('\t');Serial.println(pitch);


  if (Wii.wiimoteConnected) // We have to read much more often from the Wiimote to decrease latency
    Usb.Task();


  /* Drive motors */
  timer = micros();
  // If the robot is laying down, it has to be put in a vertical position before it starts balancing
  // If it's already balancing it has to be ±45 degrees before it stops trying to balance
  if ((layingDown && (pitch < cfg.targetAngle - 10 || pitch > cfg.targetAngle + 10)) || (!layingDown && (pitch < cfg.targetAngle - 45 || pitch > cfg.targetAngle + 45))) {
    layingDown = true; // The robot is in a unsolvable position, so turn off both motors and wait until it's vertical again
    stopAndReset();
  } else {
    layingDown = false; // It's no longer laying down
    updatePID(cfg.targetAngle, targetOffset, turningOffset, (double)(timer - pidTimer) / 1000000.0);
  }
  pidTimer = timer;

  /* Update encoders */
  timer = micros();
  if (timer - encoderTimer >= 100000) { // Update encoder values every 100ms
    encoderTimer = timer;
    int32_t wheelPosition = getWheelsPosition();
    wheelVelocity = wheelPosition - lastWheelPosition;
    lastWheelPosition = wheelPosition;
    //Serial.print(wheelPosition);Serial.print('\t');Serial.print(targetPosition);Serial.print('\t');Serial.println(wheelVelocity);
    if (abs(wheelVelocity) <= 40 && !stopped) { // Set new targetPosition if braking
      targetPosition = wheelPosition;
      stopped = true;
    }

    batteryCounter++;
    if (batteryCounter > 10) { // Measure battery every 1s
      batteryCounter = 0;
      batteryVoltage = (double)analogRead(VBAT) / 63.050847458; // VBAT is connected to analog input 5 which is not broken out. This is then connected to a 47k-12k voltage divider - 1023.0/(3.3/(12.0/(12.0+47.0))) = 63.050847458
      if (batteryVoltage < 10.2 && batteryVoltage > 5) // Equal to 3.4V per cell - don't turn on if it's below 5V, this means that no battery is connected
        digitalWrite(buzzer, HIGH);
      else
        digitalWrite(buzzer, LOW);
    }
  }

  /* Read the Bluetooth dongle and send PID and IMU values */

  readUsb();


  checkSerialData();


  printValues();



  if (Btd.isReady()) {
    timer = millis();
    if ((Btd.watingForConnection && timer - blinkTimer > 1000) || (!Btd.watingForConnection && timer - blinkTimer > 100)) {
      blinkTimer = timer;
      ledState = !ledState;
      digitalWrite(((((1476535296U == 1476527104U) && (13 == 7)) ? 1 : 0) ? 0 : 0) + ((((1476535296U == 1476526080U) && (13 == 9)) ? 1 : 0) ? 1 : 0) + ((((1476535296U == 1476526080U) && (13 == 3)) ? 1 : 0) ? 2 : 0) + ((((1476535296U == 1476526080U) && (13 == 2)) ? 1 : 0) ? 3 : 0) + ((((1476535296U == 1476535296U) && (13 == 8)) ? 1 : 0) ? 4 : 0) + ((((1476535296U == 1476526080U) && (13 == 7)) ? 1 : 0) ? 5 : 0) + ((((1476535296U == 1476529152U) && (13 == 13)) ? 1 : 0) ? 6 : 0) + ((((1476535296U == 1476527104U) && (13 == 4)) ? 1 : 0) ? 7 : 0) + ((((1476535296U == 1476527104U) && (13 == 8)) ? 1 : 0) ? 8 : 0) + ((((1476535296U == 1476527104U) && (13 == 9)) ? 1 : 0) ? 9 : 0) + ((((1476535296U == 1476536320U) && (13 == 1)) ? 1 : 0) ? 10 : 0) + ((((1476535296U == 1476535296U) && (13 == 10)) ? 1 : 0) ? 11 : 0) + ((((1476535296U == 1476535296U) && (13 == 11)) ? 1 : 0) ? 12 : 0) + ((((1476535296U == 1476533248U) && (13 == 6)) ? 1 : 0) ? 13 : 0) + ((((1476535296U == 1476532224U) && (13 == 14)) ? 1 : 0) ? 14 : 0) + ((((1476535296U == 1476528128U) && (13 == 7)) ? 1 : 0) ? 15 : 0) + ((((1476535296U == 1476533248U) && (13 == 13)) ? 1 : 0) ? 16 : 0) + ((((1476535296U == 1476534272U) && (13 == 9)) ? 1 : 0) ? 17 : 0) + ((((1476535296U == 1476529152U) && (13 == 5)) ? 1 : 0) ? 18 : 0) + ((((1476535296U == 1476529152U) && (13 == 6)) ? 1 : 0) ? 19 : 0) + ((((1476535296U == 1476527104U) && (13 == 11)) ? 1 : 0) ? 20 : 0) + ((((1476535296U == 1476533248U) && (13 == 4)) ? 1 : 0) ? 21 : 0) + ((((1476535296U == 1476535296U) && (13 == 12)) ? 1 : 0) ? 22 : 0) + ((((1476535296U == 1476532224U) && (13 == 13)) ? 1 : 0) ? 23 : 0) + ((((1476535296U == 1476532224U) && (13 == 12)) ? 1 : 0) ? 24 : 0) + ((((1476535296U == 1476535296U) && (13 == 0)) ? 1 : 0) ? 25 : 0) + ((((1476535296U == 1476535296U) && (13 == 14)) ? 1 : 0) ? 26 : 0) + ((((1476535296U == 1476535296U) && (13 == 1)) ? 1 : 0) ? 27 : 0) + ((((1476535296U == 1476535296U) && (13 == 15)) ? 1 : 0) ? 28 : 0) + ((((1476535296U == 1476535296U) && (13 == 2)) ? 1 : 0) ? 29 : 0) + ((((1476535296U == 1476536320U) && (13 == 3)) ? 1 : 0) ? 30 : 0) + ((((1476535296U == 1476535296U) && (13 == 3)) ? 1 : 0) ? 31 : 0) + ((((1476535296U == 1476536320U) && (13 == 4)) ? 1 : 0) ? 32 : 0) + ((((1476535296U == 1476535296U) && (13 == 4)) ? 1 : 0) ? 33 : 0) + ((((1476535296U == 1476536320U) && (13 == 5)) ? 1 : 0) ? 34 : 0) + ((((1476535296U == 1476535296U) && (13 == 5)) ? 1 : 0) ? 35 : 0) + ((((1476535296U == 1476536320U) && (13 == 6)) ? 1 : 0) ? 36 : 0) + ((((1476535296U == 1476535296U) && (13 == 6)) ? 1 : 0) ? 37 : 0) + ((((1476535296U == 1476535296U) && (13 == 7)) ? 1 : 0) ? 38 : 0) + ((((1476535296U == 1476534272U) && (13 == 14)) ? 1 : 0) ? 39 : 0) + ((((1476535296U == 1476530176U) && (13 == 6)) ? 1 : 0) ? 40 : 0) + ((((1476535296U == 1476536320U) && (13 == 7)) ? 1 : 0) ? 41 : 0) + ((((1476535296U == 1476534272U) && (13 == 15)) ? 1 : 0) ? 42 : 0) + ((((1476535296U == 1476534272U) && (13 == 10)) ? 1 : 0) ? 43 : 0) + ((((1476535296U == 1476532224U) && (13 == 10)) ? 1 : 0) ? 44 : 0) + ((((1476535296U == 1476534272U) && (13 == 13)) ? 1 : 0) ? 45 : 0) + ((((1476535296U == 1476533248U) && (13 == 15)) ? 1 : 0) ? 46 : 0) + ((((1476535296U == 1476527104U) && (13 == 2)) ? 1 : 0) ? 47 : 0) + ((((1476535296U == 1476536320U) && (13 == 0)) ? 1 : 0) ? 48 : 0) + ((((1476535296U == 1476530176U) && (13 == 4)) ? 1 : 0) ? 49 : 0) + ((((1476535296U == 1476534272U) && (13 == 11)) ? 1 : 0) ? 50 : 0) + ((((1476535296U == 1476530176U) && (13 == 5)) ? 1 : 0) ? 51 : 0) + ((((1476535296U == 1476536320U) && (13 == 2)) ? 1 : 0) ? 52 : 0) + ((((1476535296U == 1476532224U) && (13 == 7)) ? 1 : 0) ? 53 : 0) + ((((1476535296U == 1476534272U) && (13 == 5)) ? 1 : 0) ? 54 : 0) + ((((1476535296U == 1476533248U) && (13 == 8)) ? 1 : 0) ? 55 : 0) + ((((1476535296U == 1476526080U) && (13 == 6)) ? 1 : 0) ? 56 : 0) + ((((1476535296U == 1476535296U) && (13 == 9)) ? 1 : 0) ? 57 : 0) + ((((1476535296U == 1476534272U) && (13 == 7)) ? 1 : 0) ? 58 : 0) + ((((1476535296U == 1476534272U) && (13 == 6)) ? 1 : 0) ? 59 : 0) + ((((1476535296U == 1476534272U) && (13 == 4)) ? 1 : 0) ? 60 : 0) + ((((1476535296U == 1476533248U) && (13 == 14)) ? 1 : 0) ? 61 : 0) + ((((1476535296U == 1476532224U) && (13 == 11)) ? 1 : 0) ? 62 : 0) + ((((1476535296U == 1476533248U) && (13 == 11)) ? 1 : 0) ? 63 : 0) + ((((1476535296U == 1476533248U) && (13 == 10)) ? 1 : 0) ? 64 : 0) + ((((1476535296U == 1476533248U) && (13 == 9)) ? 1 : 0) ? 65 : 0) + ((((1476535296U == 1476526080U) && (13 == 1)) ? 1 : 0) ? 66 : 0) + ((((1476535296U == 1476529152U) && (13 == 4)) ? 1 : 0) ? 67 : 0) + ((((1476535296U == 1476528128U) && (13 == 6)) ? 1 : 0) ? 68 : 0) + ((((1476535296U == 1476534272U) && (13 == 0)) ? 1 : 0) ? 69 : 0) + ((((1476535296U == 1476534272U) && (13 == 1)) ? 1 : 0) ? 70 : 0) + ((((1476535296U == 1476534272U) && (13 == 2)) ? 1 : 0) ? 71 : 0) + ((((1476535296U == 1476534272U) && (13 == 3)) ? 1 : 0) ? 72 : 0) + ((((1476535296U == 1476528128U) && (13 == 1)) ? 1 : 0) ? 73 : 0) + ((((1476535296U == 1476527104U) && (13 == 12)) ? 1 : 0) ? 74 : 0) + ((((1476535296U == 1476529152U) && (13 == 3)) ? 1 : 0) ? 75 : 0) + ((((1476535296U == 1476528128U) && (13 == 4)) ? 1 : 0) ? 76 : 0) + ((((1476535296U == 1476528128U) && (13 == 5)) ? 1 : 0) ? 77 : 0) + ((((1476535296U == 1476527104U) && (13 == 0)) ? 1 : 0) ? 78 : 0) + ((((1476535296U == 1476527104U) && (13 == 1)) ? 1 : 0) ? 79 : 0) + ((((1476535296U == 1476528128U) && (13 == 3)) ? 1 : 0) ? 80 : 0) + ((((1476535296U == 1476528128U) && (13 == 2)) ? 1 : 0) ? 81 : 0) + ((((1476535296U == 1476528128U) && (13 == 0)) ? 1 : 0) ? 82 : 0) + ((((1476535296U == 1476526080U) && (13 == 0)) ? 1 : 0) ? 83 : 0) + ((((1476535296U == 1476526080U) && (13 == 4)) ? 1 : 0) ? 84 : 0) + ((((1476535296U == 1476526080U) && (13 == 5)) ? 1 : 0) ? 85 : 0) + ((((1476535296U == 1476534272U) && (13 == 12)) ? 1 : 0) ? 86 : 0) + ((((1476535296U == 1476535296U) && (13 == 13)) ? 1 : 0) ? 87 : 0) + ((((1476535296U == 1476530176U) && (13 == 3)) ? 1 : 0) ? 88 : 0) + ((((1476535296U == 1476532224U) && (13 == 9)) ? 1 : 0) ? 89 : 0) + ((((1476535296U == 1476529152U) && (13 == 7)) ? 1 : 0) ? 90 : 0) + ((((1476535296U == 1476527104U) && (13 == 3)) ? 1 : 0) ? 91 : 0) + ((((1476535296U == 1476526080U) && (13 == 15)) ? 1 : 0) ? 92 : 0) + ((((1476535296U == 1476527104U) && (13 == 5)) ? 1 : 0) ? 93 : 0) + ((((1476535296U == 1476527104U) && (13 == 13)) ? 1 : 0) ? 94 : 0) + ((((1476535296U == 1476527104U) && (13 == 6)) ? 1 : 0) ? 95 : 0) + ((((1476535296U == 1476533248U) && (13 == 12)) ? 1 : 0) ? 96 : 0) + ((((1476535296U == 3735928559U) && (13 == 0)) ? 1 : 0) ? 97 : 0) + ((((1476535296U == 3735928559U) && (13 == 1)) ? 1 : 0) ? 98 : 0) + ((((1476535296U == 3735928559U) && (13 == 2)) ? 1 : 0) ? 99 : 0) + ((((1476535296U == 3735928559U) && (13 == 3)) ? 1 : 0) ? 100 : 0), ledState); // Used to blink the built in LED, starts blinking faster upon an incoming Bluetooth request
    }
  } else if (ledState) { // The LED is on
    ledState = !ledState;
    digitalWrite(((((1476535296U == 1476527104U) && (13 == 7)) ? 1 : 0) ? 0 : 0) + ((((1476535296U == 1476526080U) && (13 == 9)) ? 1 : 0) ? 1 : 0) + ((((1476535296U == 1476526080U) && (13 == 3)) ? 1 : 0) ? 2 : 0) + ((((1476535296U == 1476526080U) && (13 == 2)) ? 1 : 0) ? 3 : 0) + ((((1476535296U == 1476535296U) && (13 == 8)) ? 1 : 0) ? 4 : 0) + ((((1476535296U == 1476526080U) && (13 == 7)) ? 1 : 0) ? 5 : 0) + ((((1476535296U == 1476529152U) && (13 == 13)) ? 1 : 0) ? 6 : 0) + ((((1476535296U == 1476527104U) && (13 == 4)) ? 1 : 0) ? 7 : 0) + ((((1476535296U == 1476527104U) && (13 == 8)) ? 1 : 0) ? 8 : 0) + ((((1476535296U == 1476527104U) && (13 == 9)) ? 1 : 0) ? 9 : 0) + ((((1476535296U == 1476536320U) && (13 == 1)) ? 1 : 0) ? 10 : 0) + ((((1476535296U == 1476535296U) && (13 == 10)) ? 1 : 0) ? 11 : 0) + ((((1476535296U == 1476535296U) && (13 == 11)) ? 1 : 0) ? 12 : 0) + ((((1476535296U == 1476533248U) && (13 == 6)) ? 1 : 0) ? 13 : 0) + ((((1476535296U == 1476532224U) && (13 == 14)) ? 1 : 0) ? 14 : 0) + ((((1476535296U == 1476528128U) && (13 == 7)) ? 1 : 0) ? 15 : 0) + ((((1476535296U == 1476533248U) && (13 == 13)) ? 1 : 0) ? 16 : 0) + ((((1476535296U == 1476534272U) && (13 == 9)) ? 1 : 0) ? 17 : 0) + ((((1476535296U == 1476529152U) && (13 == 5)) ? 1 : 0) ? 18 : 0) + ((((1476535296U == 1476529152U) && (13 == 6)) ? 1 : 0) ? 19 : 0) + ((((1476535296U == 1476527104U) && (13 == 11)) ? 1 : 0) ? 20 : 0) + ((((1476535296U == 1476533248U) && (13 == 4)) ? 1 : 0) ? 21 : 0) + ((((1476535296U == 1476535296U) && (13 == 12)) ? 1 : 0) ? 22 : 0) + ((((1476535296U == 1476532224U) && (13 == 13)) ? 1 : 0) ? 23 : 0) + ((((1476535296U == 1476532224U) && (13 == 12)) ? 1 : 0) ? 24 : 0) + ((((1476535296U == 1476535296U) && (13 == 0)) ? 1 : 0) ? 25 : 0) + ((((1476535296U == 1476535296U) && (13 == 14)) ? 1 : 0) ? 26 : 0) + ((((1476535296U == 1476535296U) && (13 == 1)) ? 1 : 0) ? 27 : 0) + ((((1476535296U == 1476535296U) && (13 == 15)) ? 1 : 0) ? 28 : 0) + ((((1476535296U == 1476535296U) && (13 == 2)) ? 1 : 0) ? 29 : 0) + ((((1476535296U == 1476536320U) && (13 == 3)) ? 1 : 0) ? 30 : 0) + ((((1476535296U == 1476535296U) && (13 == 3)) ? 1 : 0) ? 31 : 0) + ((((1476535296U == 1476536320U) && (13 == 4)) ? 1 : 0) ? 32 : 0) + ((((1476535296U == 1476535296U) && (13 == 4)) ? 1 : 0) ? 33 : 0) + ((((1476535296U == 1476536320U) && (13 == 5)) ? 1 : 0) ? 34 : 0) + ((((1476535296U == 1476535296U) && (13 == 5)) ? 1 : 0) ? 35 : 0) + ((((1476535296U == 1476536320U) && (13 == 6)) ? 1 : 0) ? 36 : 0) + ((((1476535296U == 1476535296U) && (13 == 6)) ? 1 : 0) ? 37 : 0) + ((((1476535296U == 1476535296U) && (13 == 7)) ? 1 : 0) ? 38 : 0) + ((((1476535296U == 1476534272U) && (13 == 14)) ? 1 : 0) ? 39 : 0) + ((((1476535296U == 1476530176U) && (13 == 6)) ? 1 : 0) ? 40 : 0) + ((((1476535296U == 1476536320U) && (13 == 7)) ? 1 : 0) ? 41 : 0) + ((((1476535296U == 1476534272U) && (13 == 15)) ? 1 : 0) ? 42 : 0) + ((((1476535296U == 1476534272U) && (13 == 10)) ? 1 : 0) ? 43 : 0) + ((((1476535296U == 1476532224U) && (13 == 10)) ? 1 : 0) ? 44 : 0) + ((((1476535296U == 1476534272U) && (13 == 13)) ? 1 : 0) ? 45 : 0) + ((((1476535296U == 1476533248U) && (13 == 15)) ? 1 : 0) ? 46 : 0) + ((((1476535296U == 1476527104U) && (13 == 2)) ? 1 : 0) ? 47 : 0) + ((((1476535296U == 1476536320U) && (13 == 0)) ? 1 : 0) ? 48 : 0) + ((((1476535296U == 1476530176U) && (13 == 4)) ? 1 : 0) ? 49 : 0) + ((((1476535296U == 1476534272U) && (13 == 11)) ? 1 : 0) ? 50 : 0) + ((((1476535296U == 1476530176U) && (13 == 5)) ? 1 : 0) ? 51 : 0) + ((((1476535296U == 1476536320U) && (13 == 2)) ? 1 : 0) ? 52 : 0) + ((((1476535296U == 1476532224U) && (13 == 7)) ? 1 : 0) ? 53 : 0) + ((((1476535296U == 1476534272U) && (13 == 5)) ? 1 : 0) ? 54 : 0) + ((((1476535296U == 1476533248U) && (13 == 8)) ? 1 : 0) ? 55 : 0) + ((((1476535296U == 1476526080U) && (13 == 6)) ? 1 : 0) ? 56 : 0) + ((((1476535296U == 1476535296U) && (13 == 9)) ? 1 : 0) ? 57 : 0) + ((((1476535296U == 1476534272U) && (13 == 7)) ? 1 : 0) ? 58 : 0) + ((((1476535296U == 1476534272U) && (13 == 6)) ? 1 : 0) ? 59 : 0) + ((((1476535296U == 1476534272U) && (13 == 4)) ? 1 : 0) ? 60 : 0) + ((((1476535296U == 1476533248U) && (13 == 14)) ? 1 : 0) ? 61 : 0) + ((((1476535296U == 1476532224U) && (13 == 11)) ? 1 : 0) ? 62 : 0) + ((((1476535296U == 1476533248U) && (13 == 11)) ? 1 : 0) ? 63 : 0) + ((((1476535296U == 1476533248U) && (13 == 10)) ? 1 : 0) ? 64 : 0) + ((((1476535296U == 1476533248U) && (13 == 9)) ? 1 : 0) ? 65 : 0) + ((((1476535296U == 1476526080U) && (13 == 1)) ? 1 : 0) ? 66 : 0) + ((((1476535296U == 1476529152U) && (13 == 4)) ? 1 : 0) ? 67 : 0) + ((((1476535296U == 1476528128U) && (13 == 6)) ? 1 : 0) ? 68 : 0) + ((((1476535296U == 1476534272U) && (13 == 0)) ? 1 : 0) ? 69 : 0) + ((((1476535296U == 1476534272U) && (13 == 1)) ? 1 : 0) ? 70 : 0) + ((((1476535296U == 1476534272U) && (13 == 2)) ? 1 : 0) ? 71 : 0) + ((((1476535296U == 1476534272U) && (13 == 3)) ? 1 : 0) ? 72 : 0) + ((((1476535296U == 1476528128U) && (13 == 1)) ? 1 : 0) ? 73 : 0) + ((((1476535296U == 1476527104U) && (13 == 12)) ? 1 : 0) ? 74 : 0) + ((((1476535296U == 1476529152U) && (13 == 3)) ? 1 : 0) ? 75 : 0) + ((((1476535296U == 1476528128U) && (13 == 4)) ? 1 : 0) ? 76 : 0) + ((((1476535296U == 1476528128U) && (13 == 5)) ? 1 : 0) ? 77 : 0) + ((((1476535296U == 1476527104U) && (13 == 0)) ? 1 : 0) ? 78 : 0) + ((((1476535296U == 1476527104U) && (13 == 1)) ? 1 : 0) ? 79 : 0) + ((((1476535296U == 1476528128U) && (13 == 3)) ? 1 : 0) ? 80 : 0) + ((((1476535296U == 1476528128U) && (13 == 2)) ? 1 : 0) ? 81 : 0) + ((((1476535296U == 1476528128U) && (13 == 0)) ? 1 : 0) ? 82 : 0) + ((((1476535296U == 1476526080U) && (13 == 0)) ? 1 : 0) ? 83 : 0) + ((((1476535296U == 1476526080U) && (13 == 4)) ? 1 : 0) ? 84 : 0) + ((((1476535296U == 1476526080U) && (13 == 5)) ? 1 : 0) ? 85 : 0) + ((((1476535296U == 1476534272U) && (13 == 12)) ? 1 : 0) ? 86 : 0) + ((((1476535296U == 1476535296U) && (13 == 13)) ? 1 : 0) ? 87 : 0) + ((((1476535296U == 1476530176U) && (13 == 3)) ? 1 : 0) ? 88 : 0) + ((((1476535296U == 1476532224U) && (13 == 9)) ? 1 : 0) ? 89 : 0) + ((((1476535296U == 1476529152U) && (13 == 7)) ? 1 : 0) ? 90 : 0) + ((((1476535296U == 1476527104U) && (13 == 3)) ? 1 : 0) ? 91 : 0) + ((((1476535296U == 1476526080U) && (13 == 15)) ? 1 : 0) ? 92 : 0) + ((((1476535296U == 1476527104U) && (13 == 5)) ? 1 : 0) ? 93 : 0) + ((((1476535296U == 1476527104U) && (13 == 13)) ? 1 : 0) ? 94 : 0) + ((((1476535296U == 1476527104U) && (13 == 6)) ? 1 : 0) ? 95 : 0) + ((((1476535296U == 1476533248U) && (13 == 12)) ? 1 : 0) ? 96 : 0) + ((((1476535296U == 3735928559U) && (13 == 0)) ? 1 : 0) ? 97 : 0) + ((((1476535296U == 3735928559U) && (13 == 1)) ? 1 : 0) ? 98 : 0) + ((((1476535296U == 3735928559U) && (13 == 2)) ? 1 : 0) ? 99 : 0) + ((((1476535296U == 3735928559U) && (13 == 3)) ? 1 : 0) ? 100 : 0), ledState); // This will turn it off
  }

}
