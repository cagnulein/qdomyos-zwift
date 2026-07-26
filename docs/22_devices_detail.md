# Devices detail

## Echelon


## Sportstech

### ESX 500
#### HRM
The [cardio belt provided](https://www.sports-tech.uk.com/chest-strap-sportstech-uncoded) with the bike is also supported, the bike reads the value and forwards it in its bluetooth signal.
The cardio captors are reported to not be accurate.
#### Resistance 
The resistance is adjusted after a few seconds delay (time for the engine to adapt magnetic resistance).

## Stages

### SB20

#### Handlebar buttons
The SB20's six handlebar buttons can control QZ — target power +/-, Peloton offset +/-, and gear
down/up — behind **Settings -> Stages Bike Options -> SB20 handlebar buttons**. Holding a button
repeats the action.

They arrive on a vendor characteristic (`0c46be60`, service `0c46be5f`) as
`<type:u8> 00 <bitmask:u16 LE>`, where the bitmask is a one-hot button id (bits 0-5: LEFT
up/down/3rd, RIGHT up/down/3rd). A press streams `0x01` frames while held and ends with a `0x04`
or `0x08` terminator; a `0x03` "commit" frame is only sent for a minority of presses, so the
decode keys off the held-stream rather than the commit frame.
