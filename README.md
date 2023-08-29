[![MSBuild](https://github.com/mrstefangrimm/TemplateStateMachine/actions/workflows/msbuild.yml/badge.svg)](https://github.com/mrstefangrimm/TemplateStateMachine/actions/workflows/msbuild.yml)

# Template State Machine

Template State Machine C++ library to to create UML<sup>1</sup> state machines in C++.



By making use of C++ templates, the state machines created with tsm have a smaller memory footprint than state machines using polymorphism (C++ virtual methods or C function pointers).


Some comparisons with other implementations on an Arduino. The example [LED On Off](https://wokwi.com/projects/374412197981887489)  with two states has the smallest memory footprint. The example [Washing Machine](https://wokwi.com/projects/374413481521914881) with five states and a state with sub-states uses 5 more bytes than "LED On Off". QPN is equally good when it comes to dynamic memory, but uses a lot of program space.

| Example sketch | Arduino IDE 2.2.0 |
| -- | -- |
| [LedOnOff (tsm)](https://wokwi.com/projects/374412197981887489) | Sketch uses 2548 bytes (7%) of program storage space. Maximum is 32256 bytes.<br/>Global variables use 207 bytes (10%) of dynamic memory, leaving 1841 bytes for local variables. Maximum is 2048 bytes. |
| [LedOnOff (GOF<sup>2</sup>)](https://wokwi.com/projects/374415929109364737) | Sketch uses 2672 bytes (8%) of program storage space. Maximum is 32256 bytes.<br/>Global variables use 212 bytes (10%) of dynamic memory, leaving 1836 bytes for local variables. Maximum is 2048 bytes. |
| [LedOnOff (qpn<sup>3</sup>)](https://wokwi.com/projects/374416715311795201) | Sketch uses 3976 bytes (12%) of program storage space. Maximum is 32256 bytes.<br/>Global variables use 289 bytes (14%) of dynamic memory, leaving 1759 bytes for local variables. Maximum is 2048 bytes. |
| [Washingmachine (tsm)](https://wokwi.com/projects/374413481521914881) | Sketch uses 2640 bytes (8%) of program storage space. Maximum is 32256 bytes.<br/>Global variables use 212 bytes (10%) of dynamic memory, leaving 1836 bytes for local variables. Maximum is 2048 bytes. |
| [Washing Machine_(qpn)](https://wokwi.com/projects/374417034159644673) | Sketch uses 4404 bytes (13%) of program storage space. Maximum is 32256 bytes.<br/>Global variables use 295 bytes (14%) of dynamic memory, leaving 1753 bytes for local variables. Maximum is 2048 bytes. |

---
[1] UML: Unified Modelling Language. What is covered by tsm is descripted in the section Features. 

[2] GOF: Gang of Four. The state machine is implemented using the design pattern.

[3] QPN: [Quantum Platform Nano](https://github.com/QuantumLeaps/qpn) by Quantum Leaps. The examples where created with the [QM modeler](https://www.state-machine.com/qm/).



### Compatibility

TSM is compatible with the current Arduino tool chain that comes with the Arduino IDE. It requires C++14 or higher and has no dependencies on other libraries.

### Installation

To use tsm in an Arduino sketch, just copy it to Arduino/Libraries and include "tsm.h". Add the tsm/src folder to your includes to use it on other platforms.



## When should you use state machines?

To learn everything about state machines, I recommend the book "Practical UML State Charts is C/C++" by Miro Samek. Also worth watching is [Rise of the State Machines](https://www.youtube.com/watch?v=Zb6xcd2as6o) by Kris Jusiak. I'll leave it at that, otherwise I would just repeat the expert voices.



## When should you use tsm?

You should only use it to experiment with it or for hobby projects at this time. The runtime behavior has not yet been sufficiently tested.

[qpn](https://github.com/QuantumLeaps/qpn) has been around for years and does an excellent job.

While thinking how I could use C++ templates to generate a state machine, SML](https://github.com/boost-ext/sml) got released. Too bad for tsm, they  did a great job. It is a pity that SML no longer works with C++14.



## Why should you use tsm?

With tsm, you write state classes with state and behavior. Sounds weird? The opposite of tsm is the [Stateless C# library](https://github.com/dotnet-state-machine/stateless) or [SML](https://github.com/boost-ext/sml). I am not saying state classes are not possible with these libraries, but state classes are not an inherent part of these libraries.

You can define the life cycle model. According to UML, the state object is deleted on exit and created on entry. This can cause memory fragmentation and state objects are usually implemented as Singletons<sup>4</sup>. With tsm you can choose and even write your own memory model.

---

[4] Singleton: Design pattern



## Features

[![statemachine example](docs/sm_example.png)



### UML state machine

The states have the methods "entry", "exit" and "do".  States can have sub-states. Transitions have an "action" and a "guard". The state machine has the method "begin" that triggers an initial transition and the method <code>end</code> that calls exit on the current state and its substates.

### Interchangeable memory model

States can be singletons, this is how most implementations work. States can be dynamically created when entered and deleted when exited. Because of memory fragmentation this is not often done.






