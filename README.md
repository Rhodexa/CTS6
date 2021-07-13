# CTS6
Finally! **Chiptune Studio Magic 6** is here! ...why the hype? Well... this is the first fully functional CTS version LOL

## About
· **AVR Chiptune Studio is a relatively fast wavetable based chiptune-oriented polyphonic synthesizer designed for ATMEGA328P** and its family. (i guess it shound work with pretty much any ATMEGAxx8P, the only lmitation is the requirement of Timer2, speed and memory. But i guess that's easily portable to other AVR CPUs). Arduino Pro Mini 3.3v may have some trouble as it has an 8MHz crystal instead of 16MHz or 20Mhz. That may reduce sample rate, making it sound horribly bad. 
<br> · CTS provides simple yet versatile dynamic sound for many AVR projects.
<br> · Two new versions are comming for Blue Pill (STM32F103C8T6) and ESP8266. 

## News and Features
 ### CTS:
 · 7 independent voice operators<br>
 · FM mode is now available!<br>
 · Stereo is now possible!<br>
 · Each operator have its own envelope generator<br>
 · global Vibrato & Tremolo is provided <br>
 · Glide, and Swipe were added! Go funky!<br>
 · As fast as we can, letting time free for your code!<br>
 · Many compiler flags available, making CTS fit your needs<br>
 · Only Timer2 is used, so you have Timer1 and Timer0 all for your self. **Fun fact:** Timer2 is used to handle Arduino's <code>tone()</code> function, but you won't be using that anymore, since you now have CTS! (Seriously, don't use it, it may break CTS and you'll have to reinitialize it)<br>
 · Pre-made wavetables are now written in program memory, freeing RAM space.<br>
 · Your custom wavetables can be easily loaded from outside the library, via <code>#define ADD_CUSTOM_WAVETABLES</code><br>
 
 ### Tracker:
  CTS now comes along a Tracker, **CTSTracker2**. Its features are based on DefleMask's format <br>
  Examples were included too, of course!
 
 ## Getting started
 ### Windows users:
 1 - Download the source <br>
 2 - Install the libray as usual: Documents > Arduino > libraries <br>
 3 - <code> #include "CTS6.h" </code> <br>
 ### Other OS:
 1 - Check the manual libray instalation for your OS <br>
 2 - Install <br>
 3 - <code> #include "CTS6.h" </code> :) <br>
 
 ## Reference / Manual
 I included two HTML files along the library. Those contain all you will need to understand CTS. I really, **really** encourage you to go read them, CTS is full of secrets you must know.
 <br>· References 
 <br>· Animated Graphics
 <br>· Solutions to potential common mistakes
 # 
 Licence? This is Free-ware, don't worry! :D
