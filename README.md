[![MSBuild](https://github.com/mrstefangrimm/TemplateStateMachine/actions/workflows/msbuild.yml/badge.svg)](https://github.com/mrstefangrimm/TemplateStateMachine/actions/workflows/msbuild.yml)

# TemplateStateMachine



Work in progress. The aim is a state machine template library with a small RAM footprint.



## Intro

If you have two states, "On" and "Off", there is nothing more efficient than a member variable. When the number of states grows and if the states have sub-states, then it is time for a state machine. XX explains in his book xXX<sup>[1]</sup>  how to write a good state machine.  Also worth watching is [Rise of the State Machines](https://www.youtube.com/watch?v=Zb6xcd2as6o) by Kris Jusiak. I leave it by that because I would otherwise just repeat the experts voices.



While I was thinking how I could use C++ templates to generate a state machine for me, the [SML](https://github.com/boost-ext/sml) was released. Bummer, they  did a great job. You can stop reading here.





## Features

### Compatibility

The TSM is compatible the current Arduino tool chain that comes with the Arduino IDE.

### UML state machine

The states have the methods "entry", "exit" and "do".  States can have sub-states. Transitions have an "action" and a "guard". The state machine has the method "begin" that triggers an initial transition and the method <code>end</code> that calls exit on the current state and its substates.

### Interchangeable memory model

States can be singletons, this is how most implementations work. States can be dynamically created when entered and deleted when exited. Because of memory fragmentation this is not often done.







  





Comparison 15 Sep. 2022

| Example sketch     | Arduino IDE 2.1.1                                           |
| ------------------ | ------------------------------------------------------------ |
| LedOnOff           | Sketch uses 3366 bytes (10%) of program storage space. Maximum is 32256 bytes.<br/>Global variables use 231 bytes (11%) of dynamic memory, leaving 1817 bytes for local variables. Maximum is 2048 bytes. |
| LedOnOff_Gof       | Sketch uses 3016 bytes (9%) of program storage space. Maximum is 32256 bytes.<br/>Global variables use 216 bytes (10%) of dynamic memory, leaving 1832 bytes for local variables. Maximum is 2048 bytes. |
| LedOnOff_Qpn       | Sketch uses 4318 bytes (13%) of program storage space. Maximum is 32256 bytes.<br/>Global variables use 297 bytes (14%) of dynamic memory, leaving 1751 bytes for local variables. Maximum is 2048 bytes. |
| Washingmachine     | Sketch uses 3738 bytes (11%) of program storage space. Maximum is 32256 bytes.<br/>Global variables use 263 bytes (12%) of dynamic memory, leaving 1785 bytes for local variables. Maximum is 2048 bytes. |
| Washingmachine_Qpn | Sketch uses 4404 bytes (13%) of program storage space. Maximum is 32256 bytes.<br/>Global variables use 295 bytes (14%) of dynamic memory, leaving 1753 bytes for local variables. Maximum is 2048 bytes. |

