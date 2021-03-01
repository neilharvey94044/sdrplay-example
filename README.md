# sdrplay-example
 
A c/c++ example program for testing out the SDRPlay API version 3.07.

## Notes
* This code was copy/pasted from the actual API documentation in pdf form.
* It has been modified to mostly ignore the RSPDuo peculiarities.  Works for RSP1A and RSPdx.
* It will stop and wait for keyboard input.
  * 'u' - increases the IF Gain by 1
  * 'd' - decreases the IF Gain by 1
  * 'n' - alternates turning RF Notch off/on
* The beginning of the program writes out the sizes of various structs.  This was the primary purpose
  for modifying this program.  To discover the actual sizes of structs that the API is using so that
  the SDRplayJava interface can be made to accurately map to the SDRPlay API.
