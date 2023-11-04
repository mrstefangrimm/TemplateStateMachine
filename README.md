[![MSBuild and MSTest](https://github.com/mrstefangrimm/TemplateStateMachine/actions/workflows/msbuild.yml/badge.svg)](https://github.com/mrstefangrimm/TemplateStateMachine/actions/workflows/msbuild.yml)
[![Compile Sketches](https://github.com/mrstefangrimm/TemplateStateMachine/actions/workflows/compile-sketches.yml/badge.svg)](https://github.com/mrstefangrimm/TemplateStateMachine/actions/workflows/compile-sketches.yml)

# Template State Machine

A C++ library to create UML<sup>1</sup> state machines in C++.



- TSM creates state machines with a smaller memory footprint than state machines using polymorphism (C++ virtual methods or C function pointers).
- TSM uses C++ templates to generate the state machine. It uses a modern, declarative way of programming.



A comparison with other implementations on an Arduino (Table 1). Using an `enum` and a state variable uses the least memory. The example with the GoF<sup>2</sup> design pattern uses more memory than tsm with more coding for less functionality, and it is not an UML state machine. The example with qpn<sup>3</sup> uses more memory, but that is because it also creates an active object and a message queue. 

| Example sketch            | enum [bytes]                                                 | tsm [bytes]                                                  | GoF<sup>2</sup> [bytes]                                      | qpn<sup>3</sup> [bytes]                                      |
| ------------------------- | ------------------------------------------------------------ | ------------------------------------------------------------ | ------------------------------------------------------------ | ------------------------------------------------------------ |
| LED On/Off (ROM/RAM)      | 1914/186  [![run](res/test_icon.png)](https://wokwi.com/projects/374747201871025153) | 2118/199  [![run](res/test_icon.png)](https://wokwi.com/projects/374516923308308481) | 2622/212  [![run](res/test_icon.png)](https://wokwi.com/projects/374415929109364737) | 3976/289  [![run](res/test_icon.png)](https://wokwi.com/projects/374738713053150209) |
| Washing machine (ROM/RAM) |                                                              | 2606/207  [![run](res/test_icon.png)](https://wokwi.com/projects/374746180590399489) |                                                              | 4404/295  [![run](res/test_icon.png)](https://wokwi.com/projects/374738815795777537) |

> Table 1: Compares different implementation for two different examples. LED On/Off has only two states, washing machine has five states, and one state has sub-states. I used the Arduino IDE 2.2.1 with the default (unchanged) compiler settings.



You can question the significance of the comparison in Table 1. Table 2 shows that tsm and qpn perform about equally well. However, the used techniques are different. TSM uses C++ templates to generate the code, while in qpn there is a switch statement for each state.

| Example | Program Space (ROM) [bytes] | Dynamic Memory (RAM) [bytes] | LOC (number of ";") |
| ------- | --------------------------- | ---------------------------- | ------------------- |
| tsm     | 3400 - 2464 = 936           | 207 - 199 = 8                | 58                  |
| qpn     | 5204 - 4324 = 880           | 295 - 289 = 6                | 110                 |

> Table 2: Compares the delta of memory consumption and Lines of Code (LOC) for tsm and qpn. 



TSM uses a modern, declarative way to program state machines. And it is a UML state machine where states can have an entry, exit, and do method, and transitions can have an action and a guard.



---
[1] UML: Unified Modeling Language. The section Getting Started describes what tsm covers. 

[2] GOF: Gang of Four. The example state machine is implements the design pattern.

[3] QPN: [Quantum Platform Nano](https://github.com/QuantumLeaps/qpn) by Quantum Leaps. I created the examples with the [QM modeler](https://www.state-machine.com/qm/).



### Compatibility

TSM is compatible with the current Arduino tool chain that comes with the Arduino IDE. TSM requires C++11 or higher and has no dependencies on other libraries.

### Installation

To use tsm in an Arduino sketch, download the [latest release](https://github.com/mrstefangrimm/TemplateStateMachine/releases) and follow the [instructions to import a .zip library](https://docs.arduino.cc/software/ide-v1/tutorials/installing-libraries). Add the tsm/src folder to the includes to use it on other platforms.



## When should you use state machines?

To learn everything about state machines, I recommend the book "Practical UML State Charts in C/C++" by Miro Samek. Worth watching is also [Rise of the State Machines](https://www.youtube.com/watch?v=Zb6xcd2as6o) by Kris Jusiak. I will leave it at that, otherwise I would just repeat the expert voices.



## When should you use tsm?

At this time, you should only use it to experiment with it or for hobby projects. The tsm runtime behavior has not yet been sufficiently tested.

[qpn](https://github.com/QuantumLeaps/qpn) has been around for years and does an excellent job.

While experimenting how I could use C++ templates to generate a state machine, [SML](https://github.com/boost-ext/sml) got released. SML also uses C++ templates to declaratively program state machines. It is a pity that SML no longer works with C++14.



## Why should you use tsm?

With tsm, you write state classes with state and behavior. Sounds weird? Examples of the opposite of tsm is the [Stateless C# library](https://github.com/dotnet-state-machine/stateless) or [SML](https://github.com/boost-ext/sml). I am not saying state classes are not possible with these libraries, but state classes are not an inherent part of these libraries.

With tsm, you can define the life cycle model. According to UML, the state object is deleted on exit and created on entry. This can cause memory fragmentation; this is one reason developers usually implemented as Singletons<sup>4</sup>. With tsm you can choose and even write your own memory model.



---

[4] Singleton: Design pattern



## Getting Started

The example with the states A, AA, AAA, AAB, and B is a good starting point. Once you scrolled down to `main` you will see that tsm is easy to use. I recommend playing with it for a while: What happens if you dispatch "B_AA" (read: "to B from AA") when already in B? In what order are the exit methods called when in state AAA and "B_AA" is dispatched?

https://wandbox.org/permlink/0RRO72sSNMh2O1nQ

[![statemachine example](res/sm_example_uml.png)](https://wandbox.org/permlink/0RRO72sSNMh2O1nQ)[![statemachine example](res/sm_example_code.png)](https://wandbox.org/permlink/0RRO72sSNMh2O1nQ)

> Figure 1: Shows a state machine example with states, sub-states, and sub-sub-states. It has transitions which go from a sub-state to a higher-level state.



### Step by step

#### Step 1

Specify whether you want your states to be singletons or created with `new` and `delete`. TSM has these predefined options:

| Comparator              | Factory          | Description                                                  |
| ----------------------- | ---------------- | ------------------------------------------------------------ |
| MemoryAddressComparator | SingletonCreator | TSM creates an object on the stack for each state.           |
| VirtualTypeIdComparator | FactoryCreator   | TSM creates an object on the heap if a transition enters a state and destroys it if a transition exits the state. The VirtualTypeIdComparator does not require RTTI. The states must implement a `getTypeId()` method which returns a unique id. |
| RttiComparator          | FactoryCreator   | TSM creates an object on the heap if a transition enters a state and destroys it if a transition exits the state. |

  All states must have the same "state policy" and factory.

```C++
// Define the policy, in this case Singleton states
using StatePolicy = State<MemoryAddressComparator, true>;

// Derive your states from SingletonCreator
struct LedOn : public BasicState<LedOn, StatePolicy, true>, SingletonCreator<LedOn> {
  template<class Event>
  void entry(const Event&) {
    digitalWrite(LED_BUILTIN, HIGH);
  }
};

struct LedOff : public BasicState<LedOff, StatePolicy, true>, SingletonCreator<LedOff> {
  template<class Event>
  void entry(const Event& ev) {
    digitalWrite(LED_BUILTIN, LOW);
  }
};
```

#### Step 2

Define the events and the transitions. Events can be empty classes or classes with attributes. In this example, the timeout event is  just used to trigger a state change and does not have attributes.

```C++
// Event
struct Timeout {};

// Transitions from LedOn to LedOff and vice versa.
using ToOnFromOff = Transition<Timeout, LedOn, LedOff, NoGuard, NoAction>;
using ToOffFromOn = Transition<Timeout, LedOff, LedOn, NoGuard, NoAction>;
```

#### Step 3

Setup the state machine. TSM requires an initial transition, even though this is optional in UML.

```C++
using Transitions =
  Typelist<ToOnFromOff,
  Typelist<ToOffFromOn,
  NullType>>;

// Initial transition to LedOff
using InitTransition = InitialTransition<LedOff, NoAction>;
using Sm = Statemachine<Transitions, InitTransition>;
```

#### Step 4

State machines have a `begin()` and an `end()`. Begin triggers the initial transition and end the final transition. States require a final transition as soon as they have sub-states.

The `setup()` and `loop()` functions round up the step-by-step example:

```C++ 
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  statemachine.begin();
}

void loop() {
  statemachine.dispatch<Timeout>();
  delay(1000);
}
```



## Tests

Unit tests are part of the Visual Studio solution and are using the VSCppUnit C++ Unit Testing Framework.



## Software Toolchain

| Tool                         | License           |
| ---------------------------- | ----------------- |
| Arduino IDE                  | GPL               |
| avrdude                      | GPL               |
| gcc                          | GPL               |
| Visual Studio Community 2022 | Microsoft license |

Libraries

| Library   | Licence           |
| :-------- | ----------------- |
| Arduino.h | LGPL              |
| VSCppUnit | Microsoft license |



## License

This software is licensed under the Apache License, Version 2.0.



<a rel="license" href="http://creativecommons.org/licenses/by-sa/4.0/"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by-sa/4.0/88x31.png" /></a>

This work is licensed under the Creative Commons Attribution-ShareAlike 4.0 International License. To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/4.0/ or send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
