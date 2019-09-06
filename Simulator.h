

#ifndef UNOJOY_H
#define UNOJOY_H
    #include <stdint.h>
    #include <util/atomic.h>
    #include <Arduino.h>

    // This struct is the core of the library.
    //  You'll create an instance of this and manipulate it,
    //  then use the setControllerData function to send that data out.
    //  Don't change this - the order of the fields is important for
    //  the communication between the Arduino and it's communications chip.
	typedef struct dataForController_t
	{
		uint8_t triangleOn : 1;  // Each of these member variables
		uint8_t circleOn : 1;    //  control if a button is off or on
		uint8_t squareOn : 1;    // For the buttons, 
		uint8_t crossOn : 1;     //  0 is off
		uint8_t l1On : 1;        //  1 is on
		uint8_t l2On : 1;        
		uint8_t l3On : 1;        // The : 1 here just tells the compiler
		uint8_t r1On : 1;        //  to only have 1 bit for each variable.
                                 //  This saves a lot of space for our type!
		uint8_t r2On : 1;
		uint8_t r3On : 1;
		uint8_t selectOn : 1;
		uint8_t startOn : 1;
		uint8_t homeOn : 1;
		uint8_t dpadLeftOn : 1;
		uint8_t dpadUpOn : 1;
		uint8_t dpadRightOn : 1;

		uint8_t dpadDownOn : 1;
        uint8_t padding : 7;     // We end with 7 bytes of padding to make sure we get our data aligned in bytes
                                 
		uint8_t leftStickX : 8;  // Each of the analog stick values can range from 0 to 255
		uint8_t leftStickY : 8;  //  0 is fully left or up
		uint8_t rightStickX : 8; //  255 is fully right or down 
		uint8_t rightStickY : 8; //  128 is centered.
                                 // Important - analogRead(pin) returns a 10 bit value, so if you're getting strange
                                 //  results from analogRead, you may need to do (analogRead(pin) >> 2) to get good data
	} dataForController_t;
    
    // Call setupUnoJoy in the setup block of your program.
    //  It sets up the hardware UnoJoy needs to work properly
    void setupUnoJoy(void);
    
    // You can also call the set
    void setupUnoJoy(int);
    
    // This sets the controller to reflect the button and
    // joystick positions you input (as a dataForController_t).
    // The controller will just send a zeroed (joysticks centered)
    // signal until you tell it otherwise with this function.
    void setControllerData(dataForController_t);
    
    // This function gives you a quick way to get a fresh
    //  dataForController_t with:
    //    No buttons pressed
    //    Joysticks centered
    // Very useful for starting each loop with a blank controller, for instance.
    // It returns a dataForController_t, so you want to call it like:
    //    myControllerData = getBlankDataForController();
    dataForController_t getBlankDataForController(void);
    
    
//----- End of the interface code you should be using -----//
//----- Below here is the actual implementation of
    
  // This dataForController_t is used to store
  //  the controller data that you want to send
  //  out to the controller.  You shouldn't mess
  //  with this directly - call setControllerData instead
  dataForController_t controllerDataBuffer;

  // This updates the data that the controller is sending out.
  //  The system actually works as following:
  //  The UnoJoy firmware on the ATmega8u2 regularly polls the
  //  Arduino chip for individual bytes of a dataForController_t.
  //  
  void setControllerData(dataForController_t controllerData){
    // Probably unecessary, but this guarantees that the data
    //  gets copied to our buffer all at once.
    ATOMIC_BLOCK(ATOMIC_FORCEON){
      controllerDataBuffer = controllerData;
    }
  }
  
  // serialCheckInterval governs how many ms between
  //  checks to the serial port for data.
  //  It shouldn't go above 20 or so, otherwise you might
  //  get unreliable data transmission to the UnoJoy firmware,
  //  since after it sends a request, it waits 25 ms for a response.
  //  If you really need to make it bigger than that, you'll have to
  //  adjust that timeout in the UnoJoy ATmega8u2 firmware code as well.
  volatile uint8_t serialCheckInterval = 1;
  // This is an internal counter variable to count ms between
  //  serial check times
  uint8_t serialCheckCounter = 0;
  
  // This is the setup function - it sets up the serial communication
  //  and the timer interrupt for actually sending the data back and forth.
  void setupUnoJoy(void){
    // First, let's zero out our controller data buffer (center the sticks)
    controllerDataBuffer = getBlankDataForController();
  
    // Start the serial port at the specific, low-error rate UnoJoy uses.
    //  If you want to change the rate, you'll have to change it in the
    //  firmware for the ATmega8u2 as well.  250,000 is actually the best rate,
    //  but it's not supported on Macs, breaking the processing debugger.
    Serial.begin(38400);
    
    // Now set up the Timer 0 compare register A
    //  so that Timer0 (used for millis() and such)
    //  also fires an interrupt when it's equal to
    //  128, not just on overflow.
    // This will fire our timer interrupt almost
    //  every 1 ms (1024 us to be exact).
    OCR0A = 128;
    TIMSK0 |= (1 << OCIE0A);
  }
  
  // If you really need to change the serial polling
  //  interval, use this function to initialize UnoJoy.
  //  interval is the polling frequency, in ms.
  void setupUnoJoy(uint8_t interval){
    serialCheckInterval = interval;
    setupUnoJoy();
  }
  
  // This interrupt gets called approximately once per ms.
  //  It counts how many ms between serial port polls,
  //  and if it's been long enough, polls the serial
  //  port to see if the UnoJoy firmware requested data.
  //  If it did, it transmits the appropriate data back.
  ISR(TIMER0_COMPA_vect){
    serialCheckCounter++;
    if (serialCheckCounter >= serialCheckInterval){
      serialCheckCounter = 0;
      // If there is incoming data stored in the Arduino serial buffer
      while (Serial.available() > 0) {
        pinMode(13, OUTPUT);
        //digitalWrite(13, HIGH);
        // Get incoming byte from the ATmega8u2
        byte inByte = Serial.read();
        // That number tells us which byte of the dataForController_t struct
        //  to send out.
        Serial.write(((uint8_t*)&controllerDataBuffer)[inByte]);
        //digitalWrite(13, LOW);
      }
    }
  }
  
  // Returns a zeroed out (joysticks centered) 
  //  dataForController_t variable
  dataForController_t getBlankDataForController(void){
    // Create a dataForController_t
    dataForController_t controllerData;
    // Make the buttons zero
    controllerData.triangleOn = 0;
    controllerData.circleOn = 0;
    controllerData.squareOn = 0;
    controllerData.crossOn = 0;
    controllerData.l1On = 0;
    controllerData.l2On = 0;
    controllerData.l3On = 0;
    controllerData.r1On = 0;
    controllerData.r2On = 0;
    controllerData.r3On = 0;
    controllerData.dpadLeftOn = 0;
    controllerData.dpadUpOn = 0;
    controllerData.dpadRightOn = 0;
    controllerData.dpadDownOn = 0;  
    controllerData.selectOn = 0;
    controllerData.startOn = 0;
    controllerData.homeOn = 0;
    //Set the sticks to 128 - centered
    controllerData.leftStickX = 128;
    controllerData.leftStickY = 128;
    controllerData.rightStickX = 128;
    controllerData.rightStickY = 128;
    // And return the data!
    return controllerData;
  }

#endif
