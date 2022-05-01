# intersection-project
Project for Micrcocontroller Programming and Applications (EGR226)

Review Intesection for model screenshots and report on the project

This project is to design and program a replica 4-way intersection. The intersection must be an automated traffic light system that sequences through colored LEDs (the traffic lights) using a finite state program implemented on the MSP432 microcontroller. The microtroller will control the sequencing of the traffic lights based on a timed cycle. Emergency vehicles equipped with an infrared transmitter can overrride the light time cycle to facilitate their safe travel through the intersection. The model design must also allow pedestrians to request a light change in order to sefaely cross the roadway in the pedistrian walkway, this is done through the press of a pushbutton. An aduible assist is availbale in case of a blind pedistrian, by holding the crosswalk button, where a 10 Hz burst of tone is emitted signaling the beginning of the walk cycle. Then a 1 Hz rate of tone indicating the walking interval and lastly a 3 Hz rate of tone as a warning that the light is about to change. THe model contains two LCDs, one that displays the current state of the roads and the other the current temperature of the room. 

The project model utilizes two MSP432 microntrollers:
* Main microcontroller controls the intersection
* Secondary microcontroller controls the IR emitter and temperature sensor

FILES include:

ProjectMain.c (contains the code for the main microcontroller)
ProjectSecondary.c (contains the code for the secondary microcontroller)

Libraries for ProjectMain.c:
* Crosswalk.h
* LEDStoplight.h
* Buzzer.h
* AeronLCDSetup.h
* IRReceiver.h
