# Heliclone #
Welcome to the home of _**Heliclone**_. This first page will try and guide you through what this is...and what it isn't. Thanx for stumbling in here!

## LATEST NEWS! - MAIDEN FLIGHT DONE - ##
I have released a new compiled version...this is a small update. Reworked a lot of things since last version (see description in source commits). The biggest news is that I have done the maiden flight with this source...it was a small hover of my little T-Rex 250...but it was flying at least!

I am currently working on some kind of simple PC editor for Heliclone. It will be rather inspired by the very nice EEPE (for Er9X). It will however be written in C# and for Windows only (at the moment). Hope to be able to upload a basic version to the source repo soon...no dates or promises however...stay tuned!

## Disclaimer ##
This software is not tested or proven in any situation. The author does not take any responsible for anything that may cause any kind of failure. This software does not come with any guaranties. If you feel uncertain...do not use this software!

_**You have been warned!**_

## Introduction ##

There are multiple replacement SW (or firmwares) for the Eurgle/FlySky/Imax/Turnigy 9X transmitter and this is another attempt.
The focus of this project is to make a good replacement firmware for Helicopters. The SW will try to address the specific needs for Helicopters on a transmitter. The project has (currently) no aim to develop a software with extensive mixing capabilities (see ER9X and TH9X for better options).

_**DO NOT USE THIS SW ON REAL HELIS YET!**_

The project is inspired by (but not branches of):

  * RadioClone - http://radioclone.org/
  * ER9X - http://code.google.com/p/er9x/
  * TH9X - http://code.google.com/p/th9x/

See these projects for really good replacement software for your transmitter!

The source in Heliclone is pretty much written from scratch. However, basic HW interface is inspired/cloned by some of the other projects.

  * The LCD-driver is basically the same as in ER9X/TH9X. I have changed a few small details (like coding standard and added a line-routine).
  * The PPM interrupts is very close to the one in Radioclone. It uses the same basics, slightly changed to my needs.
  * GPIO mapping are taken from Radioclone. This is actually the same in all projects (as the HW is fixed...) but I felt that the Radioclone was closest to my needs.
  * Other stuff I might have forgotten...if you feel that something is missing, please contact me and I will update here as soon as possible.


## Implemented Features ##
This is a list of some of the features that I want to highlight. It is not a complete set and the list may be inaccurate due to changes. I will try to keep it as updated as possible.

  * Basic Heli mode:With flight modes ID0, ID1 and ID2 incl. curves.
  * Swash-mix for 120 degrees done.
  * Simulator mode (no swash, no gyro settings)
  * Flybarless mode (no swash, incl gyro).
  * Gyro menu (AVCS/RATE), select what gain to have in both modes.
  * Swash-mix (i.e. throw) menu. Customize the throw of PIT, AIL and ELE.
  * Model management: create/delete/clone/edit.
  * E2PROM currently hold 8 models in memory. (Will try and optimize this later when all features are done...but will not spend time on this now.)
  * EndPoint settings up to 0..120%
  * Timer. Either manual mode (i.e. switch on/off) or auto mode (will use the throttle to determine if the timer should run or not). Can set time, alarm time and mode.
  * One of the POTs (on the front/left) is used as GUI-setup for smoother/faster setups. This is used in menus where one want to trim large intervals (like subtrim/curves/expo/timer...).
  * Compatible with s\_mack's nice programmer add-on inclusive backlight. See this link for more information regarding that: http://www.smartieparts.com
  * Voltage alarm adjustable to be able to use 2S LiPo:s and other lower voltage sources.
  * Safety: Force switches in certain position at boot.
  * Servo mapping menu: Be able to select what servo is mounted on what channel. The idea is to have simple presets like "Spektrum" and "Futaba" etc.


## Still to come features ##

  * Safety: Will work on some kind of "Softstart" or throttle safety mechanism.
  * Swash-leveling menu: Instead of using subtrims/endpoint menus have a more simple and customized menu for leveling the swash.
  * Switch mapping menu: What switch should be connected to what feature? Today, the defaults are used.
  * Windows tool to edit/create models. (Will be a basic programmer also...much inspired by the EEPC tool for ER9X.) It will however not be that tool...something else will be created.


## Testing/progress ##

I have tested the setup on my T-Rex 250 and it seems to be working nice. I have however not made any flights with it yet...so it is NOT recommended to use this SW for flying yet.

I have also been flying the SW in Phoenix RC sim for quite some time and it is performing very well there. The difference to the real thing is minimal as the simulator mode uses the same PPM mode output as when using the TX... The basic difference to my T-Rex 250 is that it is not using the swash-mix.

## Downloads ##

There are no downloads that are to be used for real flight available yet...

This is because I have not done the maiden flight with the software yet and until then you are urged to only test with simulator. I am sure you understand the risks in flying with non-proven software...

To compile:
  * You need WinAVR
  * AVR Studio

Both are freely available on the internet!

However, you can possibly test it and see what you think this is going. Stay tuned for more features soon.


Regards
Stefan Grufman