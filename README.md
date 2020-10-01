# Producer-Consumer-Timer
Real Time Embedded Systems 2nd Assignment 

## Installation and Execution 


```
To execute the program at a raspberry Pi you need a cross compiler 

For Pi's versions 3++ you can  sudo apt-get install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf at the Linux Terminal

In case you are using Pi Zero you need to download the correct compiler ( I used cross-pi-gcc-6.3.0-0 ) 

To local link :
PATH=/<The path to the compiler folder>/cross-pi-gcc-6.3.0-0/bin:$PATH LD_LIBRARY_PATH=<The path to the compiler folder>/2nd/cross-pi-gcc-6.3.0-0/lib:$LD_LIBRARY_PATH

And then 
arm-linux-gnueabihf-gcc -pthread  timer.c  -o rtes_prod  -lm
Upload your executable at your Pi using scp or sftp 

```

### Modes
There are 4 Available Modes 
1) One Timer with 1 sec period
2) One Timer with 0.1 sec period
3) One Timer with 0.01 sec period
4) Three Timers with 1 / 0.1 / 0.01 sec periods



## Author

* **Apostolos Moustaklis**  

