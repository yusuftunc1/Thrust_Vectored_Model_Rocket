# Astrea
THRUST VECTORED MODEL ROCKET 

It is a simple system that aims to determine the state of the rocket and keep the rocket stable with the angle and acceleration sensor (mpu6050).  
The system generates a response using the PID control system. By transferring this response to the servos thrust is actively controlled by system.

System also has a bmp180 pressure sensor to determine altitude. and there is a nrf24 in the system to provide communication and data transfer with the ground station.

Ground station has a nrf24 to communication with flight comp. It just takes the data and displays it on the serial port.

Desktop app takes the data using by serial port and saves as a csv file. And using a simple interface, it displays the different types of values like altitude,
acceleration, flight status as a graph or text during flight.
