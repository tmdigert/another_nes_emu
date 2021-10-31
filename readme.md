Welcome to <unnamed NES emulator>! An open source emulator for the NES. While the goal will (eventually) be mostly accurate emulation of most games, the short term goal is a dedicated Donkey Kong emulator. 
  
The general idea behind the implementation is to step each component of the NES as little as possible, trying to keep everything in sync. The CPU runs a single instruction, and the PPU/APU will try to catch up that many cycles worth of execution.
  
Due to the "global" nature of many computer hardware system, we opted for a sort of binary blob that will house all the parts (see src/nes.h). We will try to leave doc references scattered here and there to justify some design choices. For the most part, it'll be a trial by fire, whatever works works kind of project. There are many strategies for organizing emulators, and many other open source emulators, but we'll be trying to avoid looking at anyone else's implementations.
  
