# RailwaySignalController
Course Project: Digital Logic Design Lab (CS 254) <br>

## Team Members
Ritesh Goenka       (160050047)<br>
Manoj Middepogu     (160050075)<br>
Sathvik Kollu Reddy (160050077)<br>
Saiteja Nangunoori  (160050089)<br>

## Description

The bash script <span style="background-color: #FFFF00">run_host.sh</span> is run in the terminal at filci and the bash script "run_board.sh" is run in the terminal at cksum/vhdl.

## VHDL files
|File 				    |   Location                                                                              |
------------------|-----------------------------------------------------------------------------------------|
|encrypter.vhd    |~/20140524/makestuff/makestuff/hdlmake/apps/makestuff/swled/cksum/vhdl                   |
|decrypter.vhd 	  |~/20140524/makestuff/makestuff/hdlmake/apps/makestuff/swled/cksum/vhdl                   |
|hdlmake.cfg 		  |~/20140524/makestuff/makestuff/hdlmake/apps/makestuff/swled/cksum/vhdl                   |
|baudrate_gen.vhd |~/20140524/makestuff/makestuff/hdlmake/apps/makestuff/swled/cksum/vhdl                   |
|uart_tx.vhd 		  |~/20140524/makestuff/makestuff/hdlmake/apps/makestuff/swled/cksum/vhdl                   |
|uart_rx.vhd 		  |~/20140524/makestuff/makestuff/hdlmake/apps/makestuff/swled/cksum/vhdl                   |
|harness.vhdl		  |~/20140524/makestuff/makestuff/hdlmake/apps/makestuff/swled/templates                    |
|board.ucf		    |~/20140524/makestuff/makestuff/hdlmake/apps/makestuff/swled/templates/fx2all/boards/atlys|
|hdlmake.cfg		  |~/20140524/makestuff/makestuff/hdlmake/apps/makestuff/swled/templates/fx2all/vhdl        |
|top_level.vhdl		|~/20140524/makestuff/makestuff/hdlmake/apps/makestuff/swled/templates/fx2all/vhdl        |
|debouncer.vhd		|~/20140524/makestuff/makestuff/hdlmake/apps/makestuff/swled/templates/fx2all/vhdl        |

## C files
|File 				|Location                                 |
--------------|-----------------------------------------|
|main.c				|~/20140524/makestuff/makestuff/apps/flcli|

## Optional Part
|File        |Location                                 |
-------------|-----------------------------------------|
|one-two.py	 |~/20140524/makestuff/makestuff/apps/flcli|
|two-one.py	 |~/20140524/makestuff/makestuff/apps/flcli|

## UART communication
For the mandatoy UART communication, data is sent to the controller from another computer using gtkterminal and the received data from the controller is displayed on the same gtkterminal.

For the optional part, two python codes (one-two.py & two-one.py) are run on relay computer which directs the data flow from one controller to other.

## Compilation
- VHDL: After running run_host.sh (C-compilation) in filci, run run_board.sh in ..../swled/cksum/vhdl to connect to atlys board
- Mandatory UART: Open gtkterm with destined UART port with baudrate 115200 (ex:- sudo gtkterm -p /dev/ttyXRUSB0 -s 115200) and then send or receive data according to the protocol
- Optional UART: Run the above two .py files to create 2 paths or threads where it can act as a relay for 1->2 and 2->1 communication and send to and fro by switches
