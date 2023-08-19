# TemplateStateMachine



Work in progress. The aim is a statemachine template library with a small RAM footprint.



Comparison 15 Sep. 2022

| Example sketch     | Arduino IDE 2.1.1                                           |
| ------------------ | ------------------------------------------------------------ |
| LedOnOff           | Sketch uses 3366 bytes (10%) of program storage space. Maximum is 32256 bytes.<br/>Global variables use 231 bytes (11%) of dynamic memory, leaving 1817 bytes for local variables. Maximum is 2048 bytes. |
| LedOnOff_Gof       | Sketch uses 3016 bytes (9%) of program storage space. Maximum is 32256 bytes.<br/>Global variables use 216 bytes (10%) of dynamic memory, leaving 1832 bytes for local variables. Maximum is 2048 bytes. |
| LedOnOff_Qpn       | Sketch uses 4318 bytes (13%) of program storage space. Maximum is 32256 bytes.<br/>Global variables use 297 bytes (14%) of dynamic memory, leaving 1751 bytes for local variables. Maximum is 2048 bytes. |
| Washingmachine     | Sketch uses 3738 bytes (11%) of program storage space. Maximum is 32256 bytes.<br/>Global variables use 263 bytes (12%) of dynamic memory, leaving 1785 bytes for local variables. Maximum is 2048 bytes. |
| Washingmachine_Qpn | Sketch uses 4404 bytes (13%) of program storage space. Maximum is 32256 bytes.<br/>Global variables use 295 bytes (14%) of dynamic memory, leaving 1753 bytes for local variables. Maximum is 2048 bytes. |

