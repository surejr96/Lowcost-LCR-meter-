# Lowcost-LCR-meter

**Overview:**

An LCR meter is an electronic test equipment used to measure the inductance (L), capacitance (C), and resistance (R) of an electronic component(DUT). A design goal of this project is to limit the total cost of the daughterboard and components added to the TM4C123GXL evaluation board to $3.
Project provides complete user interface through virtual COM port (UART) on the evaluation board.
Hardware Description:
Microcontroller : An ARM M4F core (TM4C123GH6PMI microcontroller) is required.
Power LED: A power LED must be connected through a current-limiting resister to indicate the daughterboard has power.
Serial interface: If using the EK-TM4C123GXL evaluation board, then the UART0 tx/rx pair is routed to the ICDI that provides a virtual COM port through a USB endpoint.
LCR measurement interface: A circuit is provided that will interface with the microcontroller and allow the user to test an L, C, or R value. The output of this circuit can be connected to the analog comparator and analog-to-digital converter inputs.
3.3V supply: The circuit is powered completely from the 3.3V regulator output on the evaluation board.
Device under test (DUT) connection: Two connectors, made of wire loop to save cost, are required to allow the DUT to be connected.

**Suggested Parts :**

2N3904 NPN             x 5
2N3906 PNP             x 2
33 ohm , 1/2 W         x 1
3.3k ohm , 1/4 W       x 7
10k ohm , 1/4 W        x 7
1N5819 Schottky diode  x 4
1uF capacitor          x 1
47uF capacitor         x 1

**Software Description:**

A virtual COM power using a 115200 baud, 8N1 protocol with no hardware handshaking shall be provided with support to the following commands.

Debug:

If “reset” is received, the hardware shall reset.

If “voltage” is received, the hardware shall return the voltage across DUT2-DUT1.

LCR commands:

If “resistor” is received, return the capacitance of the DUT.

If “capacitance” is received, return the capacitance of the DUT.

If “inductance” is received, return the inductance of the DUT.

If “esr” is received, return the ESR of the inductor under test.

If “auto” is received, return the value of the DUT that is most predominant (i.e. an inductor with 1ohm ESR and 10mH inductance will return the inductance and ESR values, a 100kohm resistor will return the resistance, and a 10uF capacitor will return the capacitance

Tested Ratings/Ranges:

The voltage is limited to 0 to 3.3V.

Resistance value from 10ohms to 1Mohm.

Capacitance value from 1nF to 100uF.

Inductance value from 1nH to 100mH.

Instructions for using code with TI's Code Composer Studio (CCS):

Download the zip.

Add the LCR meter.c file to your project folder.

Replace your startup file with tm4c123gh6pm_startup_ccs.c from the zip.

Debug and setup terminal on your pc for 115200 baud, 8N1 protocol.

Run.

Hardware build instructions:

Refer schematic for circuit diagram.

Refer LCR-meter.c for the pins used on the board.

Make sure the schottky diodes are connected with correct polarity.

Add 47uF capcitor between VCC and gnd.
