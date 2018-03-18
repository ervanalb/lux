EESchema Schematic File Version 2
LIBS:led spot-rescue
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:special
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:max15062
LIBS:custom
LIBS:switches
LIBS:led spot-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 4
Title ""
Date "2 jun 2015"
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Sheet
S 6650 700  700  450 
U 556D27B1
F0 "DriverR" 50
F1 "led_driver.sch" 50
F2 "LEDCat" I R 7350 900 60 
F3 "DIM" I L 6650 900 60 
$EndSheet
$Sheet
S 6650 1300 700  450 
U 556D3024
F0 "DriverG" 50
F1 "led_driver.sch" 50
F2 "LEDCat" I R 7350 1500 60 
F3 "DIM" I L 6650 1500 60 
$EndSheet
$Sheet
S 6650 1900 700  450 
U 556D375F
F0 "DriverB" 50
F1 "led_driver.sch" 50
F2 "LEDCat" I R 7350 2100 60 
F3 "DIM" I L 6650 2100 60 
$EndSheet
$Comp
L CONN_4 P2
U 1 1 556D3B21
P 7600 3000
F 0 "P2" V 7550 3000 50  0000 C CNN
F 1 "LED" V 7650 3000 50  0000 C CNN
F 2 "custom:CHINESE-LARGE-LED" H 7600 3000 60  0001 C CNN
F 3 "" H 7600 3000 60  0000 C CNN
	1    7600 3000
	1    0    0    1   
$EndComp
Text Label 6250 900  0    60   ~ 0
PWM_R
Text Label 6250 1500 0    60   ~ 0
PWM_G
Text Label 6250 2100 0    60   ~ 0
PWM_B
Text Label 6600 2850 0    60   ~ 0
LED_R
Text Label 6600 2950 0    60   ~ 0
LED_G
Text Label 6600 3050 0    60   ~ 0
LED_B
Text Label 6600 3150 0    60   ~ 0
LED_ANODE
$Comp
L +48V #PWR34
U 1 1 556D3EA8
P 6450 3050
F 0 "#PWR34" H 6450 3180 20  0001 C CNN
F 1 "+48V" H 6450 3150 30  0000 C CNN
F 2 "" H 6450 3050 60  0000 C CNN
F 3 "" H 6450 3050 60  0000 C CNN
	1    6450 3050
	1    0    0    -1  
$EndComp
Text Label 7850 900  2    60   ~ 0
LED_R
Text Label 7850 1500 2    60   ~ 0
LED_G
Text Label 7850 2100 2    60   ~ 0
LED_B
$Comp
L STM32F030-20 U2
U 1 1 556D7A8F
P 2550 2600
F 0 "U2" H 2550 2500 50  0000 C CNN
F 1 "STM32F030-20" H 2550 2700 50  0000 C CNN
F 2 "Housings_SSOP:TSSOP-20_4.4x6.5mm_Pitch0.65mm" H 2550 2600 50  0001 C CNN
F 3 "DOCUMENTATION" H 2550 2600 50  0001 C CNN
	1    2550 2600
	1    0    0    -1  
$EndComp
$Comp
L GND-RESCUE-led_spot #PWR20
U 1 1 556D8437
P 3400 2650
F 0 "#PWR20" H 3400 2650 30  0001 C CNN
F 1 "GND" H 3400 2580 30  0001 C CNN
F 2 "" H 3400 2650 60  0000 C CNN
F 3 "" H 3400 2650 60  0000 C CNN
	1    3400 2650
	0    -1   -1   0   
$EndComp
$Comp
L VCC #PWR19
U 1 1 556D849F
P 3400 2550
F 0 "#PWR19" H 3400 2650 30  0001 C CNN
F 1 "VCC" H 3400 2650 30  0000 C CNN
F 2 "" H 3400 2550 60  0000 C CNN
F 3 "" H 3400 2550 60  0000 C CNN
	1    3400 2550
	0    1    1    0   
$EndComp
Text Label 4000 2350 2    60   ~ 0
RX
Text Label 4000 2450 2    60   ~ 0
TX
NoConn ~ 1700 2450
Text Label 3900 2750 2    60   ~ 0
PWM_R
Text Label 3900 2850 2    60   ~ 0
PWM_G
Text Label 3900 2950 2    60   ~ 0
PWM_B
$Comp
L LED-RESCUE-led_spot D4
U 1 1 556DA060
P 5650 2800
F 0 "D4" H 5550 2900 50  0000 C CNN
F 1 "USR" H 5750 2900 50  0000 C CNN
F 2 "LEDs:LED_0603" H 5650 2800 60  0000 C CNN
F 3 "~" H 5650 2800 60  0000 C CNN
	1    5650 2800
	1    0    0    -1  
$EndComp
$Comp
L R-RESCUE-led_spot R6
U 1 1 556DA0B1
P 5150 2800
F 0 "R6" V 5230 2800 40  0000 C CNN
F 1 "220R" V 5157 2801 40  0000 C CNN
F 2 "Resistors_SMD:R_0603" V 5080 2800 30  0001 C CNN
F 3 "~" H 5150 2800 30  0000 C CNN
	1    5150 2800
	0    -1   -1   0   
$EndComp
$Comp
L GND-RESCUE-led_spot #PWR33
U 1 1 556DA2C7
P 6000 2900
F 0 "#PWR33" H 6000 2900 30  0001 C CNN
F 1 "GND" H 6000 2830 30  0001 C CNN
F 2 "" H 6000 2900 60  0000 C CNN
F 3 "" H 6000 2900 60  0000 C CNN
	1    6000 2900
	1    0    0    -1  
$EndComp
Text Label 4500 2800 0    60   ~ 0
USR_LED
Text Label 1100 3050 0    60   ~ 0
USR_LED
$Comp
L GND-RESCUE-led_spot #PWR4
U 1 1 556DABB3
P 1700 2150
F 0 "#PWR4" H 1700 2150 30  0001 C CNN
F 1 "GND" H 1700 2080 30  0001 C CNN
F 2 "" H 1700 2150 60  0000 C CNN
F 3 "" H 1700 2150 60  0000 C CNN
	1    1700 2150
	0    1    1    0   
$EndComp
Wire Wire Line
	6650 900  6250 900 
Wire Wire Line
	6250 1500 6650 1500
Wire Wire Line
	6650 2100 6250 2100
Wire Wire Line
	7250 2850 6600 2850
Wire Wire Line
	6600 2950 7250 2950
Wire Wire Line
	7250 3050 6600 3050
Wire Wire Line
	6450 3150 7250 3150
Wire Wire Line
	7350 900  7850 900 
Wire Wire Line
	7350 1500 7850 1500
Wire Wire Line
	7350 2100 7850 2100
Wire Wire Line
	6450 3050 6450 3150
Wire Wire Line
	3400 2450 4000 2450
Wire Wire Line
	3400 2350 4000 2350
Wire Wire Line
	3400 2250 4000 2250
Wire Wire Line
	3400 2750 3900 2750
Wire Wire Line
	3900 2850 3400 2850
Wire Wire Line
	3400 2950 3900 2950
Wire Wire Line
	5400 2800 5450 2800
Wire Wire Line
	4900 2800 4500 2800
Wire Wire Line
	1100 3050 1700 3050
$Comp
L C-RESCUE-led_spot C2
U 1 1 556E0763
P 1150 2200
F 0 "C2" H 1150 2300 40  0000 L CNN
F 1 "100n" H 1156 2115 40  0000 L CNN
F 2 "Capacitors_SMD:C_0603" H 1188 2050 30  0000 C CNN
F 3 "~" H 1150 2200 60  0000 C CNN
	1    1150 2200
	1    0    0    -1  
$EndComp
$Comp
L GND-RESCUE-led_spot #PWR2
U 1 1 556E078D
P 1150 2450
F 0 "#PWR2" H 1150 2450 30  0001 C CNN
F 1 "GND" H 1150 2380 30  0001 C CNN
F 2 "" H 1150 2450 60  0000 C CNN
F 3 "" H 1150 2450 60  0000 C CNN
	1    1150 2450
	1    0    0    -1  
$EndComp
Wire Wire Line
	1150 2450 1150 2400
$Comp
L VCC #PWR1
U 1 1 556E0832
P 1150 1900
F 0 "#PWR1" H 1150 2000 30  0001 C CNN
F 1 "VCC" H 1150 2000 30  0000 C CNN
F 2 "" H 1150 1900 60  0000 C CNN
F 3 "" H 1150 1900 60  0000 C CNN
	1    1150 1900
	1    0    0    -1  
$EndComp
Wire Wire Line
	1150 1900 1150 2000
Text Label 4000 2250 2    60   ~ 0
SWDIO
Wire Wire Line
	3400 2150 4000 2150
Text Label 4000 2150 2    60   ~ 0
SWCLK
Text Label 4000 3050 2    60   ~ 0
DE
NoConn ~ 1700 2650
NoConn ~ 1700 2350
NoConn ~ 1700 2250
$Comp
L CONN_4 P3
U 1 1 556E1FF9
P 7600 3800
F 0 "P3" V 7550 3800 50  0000 C CNN
F 1 "SWD" V 7650 3800 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x04" H 7600 3800 60  0001 C CNN
F 3 "" H 7600 3800 60  0000 C CNN
	1    7600 3800
	1    0    0    -1  
$EndComp
Wire Wire Line
	6600 3650 7250 3650
Wire Wire Line
	6650 3750 7250 3750
Wire Wire Line
	6600 3850 7250 3850
Wire Wire Line
	6650 3950 7250 3950
Wire Wire Line
	6600 3650 6600 3450
$Comp
L VCC #PWR35
U 1 1 556E247F
P 6600 3450
F 0 "#PWR35" H 6600 3550 30  0001 C CNN
F 1 "VCC" H 6600 3550 30  0000 C CNN
F 2 "" H 6600 3450 60  0000 C CNN
F 3 "" H 6600 3450 60  0000 C CNN
	1    6600 3450
	1    0    0    -1  
$EndComp
$Comp
L GND-RESCUE-led_spot #PWR36
U 1 1 556E24A9
P 6600 4100
F 0 "#PWR36" H 6600 4100 30  0001 C CNN
F 1 "GND" H 6600 4030 30  0001 C CNN
F 2 "" H 6600 4100 60  0000 C CNN
F 3 "" H 6600 4100 60  0000 C CNN
	1    6600 4100
	1    0    0    -1  
$EndComp
Wire Wire Line
	6600 3850 6600 4100
Text Label 6650 3750 0    60   ~ 0
SWCLK
Text Label 6650 3950 0    60   ~ 0
SWDIO
$Comp
L CONN_4 P1
U 1 1 556FD5D7
P 1200 3800
F 0 "P1" V 1150 3800 50  0000 C CNN
F 1 "mod_jack_in" V 1250 3800 50  0000 C CNN
F 2 "custom:Assman-6p4c" H 1200 3800 60  0001 C CNN
F 3 "" H 1200 3800 60  0000 C CNN
	1    1200 3800
	-1   0    0    1   
$EndComp
$Comp
L GND-RESCUE-led_spot #PWR3
U 1 1 556FD5DD
P 1550 4050
F 0 "#PWR3" H 1550 4050 30  0001 C CNN
F 1 "GND" H 1550 3980 30  0001 C CNN
F 2 "" H 1550 4050 60  0000 C CNN
F 3 "" H 1550 4050 60  0000 C CNN
	1    1550 4050
	1    0    0    -1  
$EndComp
$Comp
L C-RESCUE-led_spot C8
U 1 1 556FD5E3
P 3550 3850
F 0 "C8" H 3550 3950 40  0000 L CNN
F 1 "2u2" H 3556 3765 40  0000 L CNN
F 2 "Capacitors_SMD:C_1210" H 3588 3700 30  0001 C CNN
F 3 "" H 3550 3850 60  0000 C CNN
	1    3550 3850
	1    0    0    -1  
$EndComp
$Comp
L GND-RESCUE-led_spot #PWR22
U 1 1 556FD602
P 3550 4050
F 0 "#PWR22" H 3550 4050 30  0001 C CNN
F 1 "GND" H 3550 3980 30  0001 C CNN
F 2 "" H 3550 4050 60  0000 C CNN
F 3 "" H 3550 4050 60  0000 C CNN
	1    3550 4050
	1    0    0    -1  
$EndComp
$Comp
L ST485E U4
U 1 1 556FD620
P 4500 5900
F 0 "U4" H 4500 5850 60  0000 C CNN
F 1 "ISL83485IBZ" H 4500 5600 60  0000 C CNN
F 2 "Housings_SOIC:SOIC-8_3.9x4.9mm_Pitch1.27mm" H 4550 5900 60  0001 C CNN
F 3 "" H 4550 5900 60  0000 C CNN
	1    4500 5900
	-1   0    0    -1  
$EndComp
Wire Wire Line
	1550 3950 1550 4050
Connection ~ 3550 3650
Wire Wire Line
	1550 3750 1900 3750
Wire Wire Line
	1550 3850 1800 3850
$Comp
L R-RESCUE-led_spot R4
U 1 1 556FD638
P 3400 5900
F 0 "R4" V 3480 5900 40  0000 C CNN
F 1 "120R" V 3407 5901 40  0000 C CNN
F 2 "Resistors_SMD:R_0603" V 3330 5900 30  0001 C CNN
F 3 "" H 3400 5900 30  0000 C CNN
	1    3400 5900
	1    0    0    -1  
$EndComp
Wire Wire Line
	1800 3850 1800 6100
Wire Wire Line
	3600 6150 3600 5950
Wire Wire Line
	3600 5950 3800 5950
Connection ~ 3400 6150
Wire Wire Line
	3800 5850 3600 5850
Wire Wire Line
	3600 5850 3600 5650
Wire Wire Line
	1900 3750 1900 5800
Connection ~ 3400 5650
$Comp
L GND-RESCUE-led_spot #PWR26
U 1 1 556FD646
P 3800 6150
F 0 "#PWR26" H 3800 6150 30  0001 C CNN
F 1 "GND" H 3800 6080 30  0001 C CNN
F 2 "" H 3800 6150 60  0000 C CNN
F 3 "" H 3800 6150 60  0000 C CNN
	1    3800 6150
	1    0    0    -1  
$EndComp
Wire Wire Line
	3800 6150 3800 6050
Wire Wire Line
	3800 5650 3800 5750
Wire Wire Line
	5200 5900 5550 5900
Wire Wire Line
	5200 5850 5200 5950
Connection ~ 5200 5900
Wire Wire Line
	5550 5750 5200 5750
$Comp
L C-RESCUE-led_spot C9
U 1 1 556FD677
P 4200 5300
F 0 "C9" H 4200 5400 40  0000 L CNN
F 1 "100n" H 4206 5215 40  0000 L CNN
F 2 "Capacitors_SMD:C_0603" H 4238 5150 30  0001 C CNN
F 3 "" H 4200 5300 60  0000 C CNN
	1    4200 5300
	1    0    0    -1  
$EndComp
$Comp
L GND-RESCUE-led_spot #PWR31
U 1 1 556FD67D
P 4200 5500
F 0 "#PWR31" H 4200 5500 30  0001 C CNN
F 1 "GND" H 4200 5430 30  0001 C CNN
F 2 "" H 4200 5500 60  0000 C CNN
F 3 "" H 4200 5500 60  0000 C CNN
	1    4200 5500
	1    0    0    -1  
$EndComp
$Comp
L +48V #PWR23
U 1 1 556FD6A4
P 3650 4600
F 0 "#PWR23" H 3650 4730 20  0001 C CNN
F 1 "+48V" H 3650 4700 30  0000 C CNN
F 2 "" H 3650 4600 60  0000 C CNN
F 3 "" H 3650 4600 60  0000 C CNN
	1    3650 4600
	1    0    0    -1  
$EndComp
$Comp
L GND-RESCUE-led_spot #PWR27
U 1 1 556FD6BC
P 3850 4600
F 0 "#PWR27" H 3850 4600 30  0001 C CNN
F 1 "GND" H 3850 4530 30  0001 C CNN
F 2 "" H 3850 4600 60  0000 C CNN
F 3 "" H 3850 4600 60  0000 C CNN
	1    3850 4600
	-1   0    0    1   
$EndComp
$Comp
L MOSFET_P Q1
U 1 1 556FD6C6
P 3100 3750
F 0 "Q1" H 3100 3940 60  0000 R CNN
F 1 "FDN5618P" V 3300 4050 60  0000 R CNN
F 2 "TO_SOT_Packages_SMD:SOT-23" H 3100 3750 60  0001 C CNN
F 3 "" H 3100 3750 60  0000 C CNN
	1    3100 3750
	0    1    -1   0   
$EndComp
$Comp
L R-RESCUE-led_spot R1
U 1 1 556FD6CC
P 2700 3900
F 0 "R1" V 2780 3900 40  0000 C CNN
F 1 "22K" V 2707 3901 40  0000 C CNN
F 2 "Resistors_SMD:R_0603" V 2630 3900 30  0001 C CNN
F 3 "" H 2700 3900 30  0000 C CNN
	1    2700 3900
	1    0    0    -1  
$EndComp
$Comp
L R-RESCUE-led_spot R2
U 1 1 556FD6D2
P 2700 4400
F 0 "R2" V 2780 4400 40  0000 C CNN
F 1 "47K" V 2707 4401 40  0000 C CNN
F 2 "Resistors_SMD:R_0603" V 2630 4400 30  0001 C CNN
F 3 "" H 2700 4400 30  0000 C CNN
	1    2700 4400
	1    0    0    -1  
$EndComp
Wire Wire Line
	3100 4150 3100 3950
Connection ~ 2700 3650
$Comp
L GND-RESCUE-led_spot #PWR10
U 1 1 556FD6DA
P 2700 4650
F 0 "#PWR10" H 2700 4650 30  0001 C CNN
F 1 "GND" H 2700 4580 30  0001 C CNN
F 2 "" H 2700 4650 60  0000 C CNN
F 3 "" H 2700 4650 60  0000 C CNN
	1    2700 4650
	1    0    0    -1  
$EndComp
Connection ~ 2700 4150
Wire Wire Line
	1550 3650 2900 3650
$Comp
L +48V #PWR21
U 1 1 556FD6E2
P 3550 3650
F 0 "#PWR21" H 3550 3780 20  0001 C CNN
F 1 "+48V" H 3550 3750 30  0000 C CNN
F 2 "" H 3550 3650 60  0000 C CNN
F 3 "" H 3550 3650 60  0000 C CNN
	1    3550 3650
	1    0    0    -1  
$EndComp
Wire Wire Line
	2700 4150 3100 4150
$Comp
L DIODESCH D1
U 1 1 556FD6EA
P 2250 3850
F 0 "D1" H 2250 3950 40  0000 C CNN
F 1 "1SMB58AT3G" H 2250 3750 40  0000 C CNN
F 2 "Diodes_SMD:SMB_Standard" H 2250 3850 60  0001 C CNN
F 3 "" H 2250 3850 60  0000 C CNN
	1    2250 3850
	0    -1   -1   0   
$EndComp
$Comp
L GND-RESCUE-led_spot #PWR9
U 1 1 556FD6F0
P 2250 4050
F 0 "#PWR9" H 2250 4050 30  0001 C CNN
F 1 "GND" H 2250 3980 30  0001 C CNN
F 2 "" H 2250 4050 60  0000 C CNN
F 3 "" H 2250 4050 60  0000 C CNN
	1    2250 4050
	1    0    0    -1  
$EndComp
Connection ~ 2250 3650
$Comp
L R-RESCUE-led_spot R3
U 1 1 556FD6F7
P 3400 5400
F 0 "R3" V 3480 5400 40  0000 C CNN
F 1 "680R" V 3407 5401 40  0000 C CNN
F 2 "Resistors_SMD:R_0603" V 3330 5400 30  0001 C CNN
F 3 "" H 3400 5400 30  0000 C CNN
	1    3400 5400
	1    0    0    -1  
$EndComp
$Comp
L R-RESCUE-led_spot R5
U 1 1 556FD6FD
P 3400 6400
F 0 "R5" V 3480 6400 40  0000 C CNN
F 1 "680R" V 3407 6401 40  0000 C CNN
F 2 "Resistors_SMD:R_0603" V 3330 6400 30  0001 C CNN
F 3 "" H 3400 6400 30  0000 C CNN
	1    3400 6400
	1    0    0    -1  
$EndComp
Wire Wire Line
	3200 6650 3400 6650
Wire Wire Line
	3400 5150 3200 5150
Wire Wire Line
	3200 5150 3200 5200
$Comp
L GND-RESCUE-led_spot #PWR15
U 1 1 556FD70C
P 3200 5200
F 0 "#PWR15" H 3200 5200 30  0001 C CNN
F 1 "GND" H 3200 5130 30  0001 C CNN
F 2 "" H 3200 5200 60  0000 C CNN
F 3 "" H 3200 5200 60  0000 C CNN
	1    3200 5200
	1    0    0    -1  
$EndComp
$Comp
L 2CH-TVS U3
U 1 1 556FD712
P 2750 5950
F 0 "U3" H 2750 6250 60  0000 C CNN
F 1 "2CH-TVS" H 2750 5650 60  0000 C CNN
F 2 "TO_SOT_Packages_SMD:SOT-23" H 2750 5950 60  0001 C CNN
F 3 "" H 2750 5950 60  0000 C CNN
	1    2750 5950
	1    0    0    -1  
$EndComp
Wire Wire Line
	1900 5800 2250 5800
Wire Wire Line
	2250 5800 2250 5500
Wire Wire Line
	2250 5500 3150 5500
Wire Wire Line
	3150 5500 3150 5650
Wire Wire Line
	3150 5650 3600 5650
Wire Wire Line
	3150 6150 3600 6150
Wire Wire Line
	3150 6150 3150 6350
Wire Wire Line
	3150 6350 2250 6350
Wire Wire Line
	2250 6350 2250 6100
Wire Wire Line
	2250 6100 1800 6100
Connection ~ 2250 6100
Connection ~ 2250 5800
$Comp
L GND-RESCUE-led_spot #PWR17
U 1 1 556FD724
P 3250 6050
F 0 "#PWR17" H 3250 6050 30  0001 C CNN
F 1 "GND" H 3250 5980 30  0001 C CNN
F 2 "" H 3250 6050 60  0000 C CNN
F 3 "" H 3250 6050 60  0000 C CNN
	1    3250 6050
	1    0    0    -1  
$EndComp
Wire Wire Line
	3250 5950 3250 6050
Wire Wire Line
	3300 3650 3550 3650
Wire Wire Line
	5850 2800 6000 2800
Wire Wire Line
	6000 2800 6000 2900
$Comp
L CONN_3 K1
U 1 1 556FFAE4
P 3750 4950
F 0 "K1" V 3700 4950 50  0000 C CNN
F 1 "PWR" V 3800 4950 40  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x03" H 3750 4950 60  0001 C CNN
F 3 "" H 3750 4950 60  0000 C CNN
	1    3750 4950
	0    -1   1    0   
$EndComp
Wire Wire Line
	5200 6050 5550 6050
Text Label 5350 5750 0    60   ~ 0
RX
Text Label 5350 6050 0    60   ~ 0
TX
Text Label 5350 5900 0    60   ~ 0
DE
$Comp
L VCC #PWR5
U 1 1 55710952
P 1700 2550
F 0 "#PWR5" H 1700 2650 30  0001 C CNN
F 1 "VCC" H 1700 2650 30  0000 C CNN
F 2 "" H 1700 2550 60  0000 C CNN
F 3 "" H 1700 2550 60  0000 C CNN
	1    1700 2550
	0    -1   -1   0   
$EndComp
Wire Wire Line
	3400 3050 4000 3050
NoConn ~ 1700 2850
NoConn ~ 1700 2750
$Comp
L LM2594 U1
U 1 1 5571074E
P 2400 1250
F 0 "U1" H 2350 950 60  0000 C CNN
F 1 "LM2594-HV" H 2350 1550 60  0000 C CNN
F 2 "Housings_SOIC:SOIC-8_3.9x4.9mm_Pitch1.27mm" H 2350 850 60  0001 C CNN
F 3 "" H 2350 850 60  0000 C CNN
	1    2400 1250
	1    0    0    -1  
$EndComp
$Comp
L GND-RESCUE-led_spot #PWR11
U 1 1 557107C9
P 3000 1300
F 0 "#PWR11" H 3000 1300 30  0001 C CNN
F 1 "GND" H 3000 1230 30  0001 C CNN
F 2 "" H 3000 1300 60  0000 C CNN
F 3 "" H 3000 1300 60  0000 C CNN
	1    3000 1300
	0    -1   -1   0   
$EndComp
$Comp
L +48V #PWR13
U 1 1 5571080E
P 3150 1000
F 0 "#PWR13" H 3150 1130 20  0001 C CNN
F 1 "+48V" H 3150 1100 30  0000 C CNN
F 2 "" H 3150 1000 60  0000 C CNN
F 3 "" H 3150 1000 60  0000 C CNN
	1    3150 1000
	1    0    0    -1  
$EndComp
$Comp
L DIODESCH D2
U 1 1 55710920
P 3400 1300
F 0 "D2" H 3400 1400 40  0000 C CNN
F 1 "STPS1H100U" H 3400 1200 40  0000 C CNN
F 2 "Diodes_SMD:SMB_Standard" H 3400 1300 60  0001 C CNN
F 3 "" H 3400 1300 60  0000 C CNN
	1    3400 1300
	0    -1   -1   0   
$EndComp
$Comp
L INDUCTOR L1
U 1 1 55710981
P 3700 1100
F 0 "L1" V 3650 1100 40  0000 C CNN
F 1 "22u" V 3800 1100 40  0000 C CNN
F 2 "custom:VLF-M" H 3700 1100 60  0001 C CNN
F 3 "" H 3700 1100 60  0000 C CNN
	1    3700 1100
	0    -1   -1   0   
$EndComp
Wire Wire Line
	3400 1100 3000 1100
$Comp
L C-RESCUE-led_spot C3
U 1 1 557109FF
P 4000 1300
F 0 "C3" H 4000 1400 40  0000 L CNN
F 1 "4u7" H 4006 1215 40  0000 L CNN
F 2 "Capacitors_SMD:C_0805" H 4038 1150 30  0001 C CNN
F 3 "" H 4000 1300 60  0000 C CNN
	1    4000 1300
	1    0    0    -1  
$EndComp
$Comp
L GND-RESCUE-led_spot #PWR18
U 1 1 55710A20
P 3400 1500
F 0 "#PWR18" H 3400 1500 30  0001 C CNN
F 1 "GND" H 3400 1430 30  0001 C CNN
F 2 "" H 3400 1500 60  0000 C CNN
F 3 "" H 3400 1500 60  0000 C CNN
	1    3400 1500
	1    0    0    -1  
$EndComp
$Comp
L GND-RESCUE-led_spot #PWR28
U 1 1 55710A41
P 4000 1500
F 0 "#PWR28" H 4000 1500 30  0001 C CNN
F 1 "GND" H 4000 1430 30  0001 C CNN
F 2 "" H 4000 1500 60  0000 C CNN
F 3 "" H 4000 1500 60  0000 C CNN
	1    4000 1500
	1    0    0    -1  
$EndComp
$Comp
L VCC #PWR29
U 1 1 55710A62
P 4200 1100
F 0 "#PWR29" H 4200 1200 30  0001 C CNN
F 1 "VCC" H 4200 1200 30  0000 C CNN
F 2 "" H 4200 1100 60  0000 C CNN
F 3 "" H 4200 1100 60  0000 C CNN
	1    4200 1100
	0    1    1    0   
$EndComp
Wire Wire Line
	4200 1100 4000 1100
Connection ~ 4000 1100
Connection ~ 3400 1100
$Comp
L GND-RESCUE-led_spot #PWR6
U 1 1 55710C18
P 1800 1100
F 0 "#PWR6" H 1800 1100 30  0001 C CNN
F 1 "GND" H 1800 1030 30  0001 C CNN
F 2 "" H 1800 1100 60  0000 C CNN
F 3 "" H 1800 1100 60  0000 C CNN
	1    1800 1100
	0    1    1    0   
$EndComp
$Comp
L GND-RESCUE-led_spot #PWR7
U 1 1 55710C37
P 1800 1200
F 0 "#PWR7" H 1800 1200 30  0001 C CNN
F 1 "GND" H 1800 1130 30  0001 C CNN
F 2 "" H 1800 1200 60  0000 C CNN
F 3 "" H 1800 1200 60  0000 C CNN
	1    1800 1200
	0    1    1    0   
$EndComp
$Comp
L GND-RESCUE-led_spot #PWR8
U 1 1 55710C4F
P 1800 1300
F 0 "#PWR8" H 1800 1300 30  0001 C CNN
F 1 "GND" H 1800 1230 30  0001 C CNN
F 2 "" H 1800 1300 60  0000 C CNN
F 3 "" H 1800 1300 60  0000 C CNN
	1    1800 1300
	0    1    1    0   
$EndComp
Wire Wire Line
	1800 1400 1650 1400
Wire Wire Line
	1650 1400 1650 800 
Wire Wire Line
	1650 800  4000 800 
Wire Wire Line
	4000 800  4000 1100
$Comp
L C-RESCUE-led_spot C1
U 1 1 55710DDF
P 3150 1500
F 0 "C1" H 3150 1600 40  0000 L CNN
F 1 "100n" H 3156 1415 40  0000 L CNN
F 2 "Capacitors_SMD:C_0603" H 3188 1350 30  0001 C CNN
F 3 "" H 3150 1500 60  0000 C CNN
	1    3150 1500
	1    0    0    -1  
$EndComp
Wire Wire Line
	3150 1000 3150 1300
Wire Wire Line
	3150 1200 3000 1200
Connection ~ 3150 1200
$Comp
L GND-RESCUE-led_spot #PWR14
U 1 1 55710ED5
P 3150 1700
F 0 "#PWR14" H 3150 1700 30  0001 C CNN
F 1 "GND" H 3150 1630 30  0001 C CNN
F 2 "" H 3150 1700 60  0000 C CNN
F 3 "" H 3150 1700 60  0000 C CNN
	1    3150 1700
	1    0    0    -1  
$EndComp
$Comp
L GND-RESCUE-led_spot #PWR12
U 1 1 5571101B
P 3000 1400
F 0 "#PWR12" H 3000 1400 30  0001 C CNN
F 1 "GND" H 3000 1330 30  0001 C CNN
F 2 "" H 3000 1400 60  0000 C CNN
F 3 "" H 3000 1400 60  0000 C CNN
	1    3000 1400
	0    -1   -1   0   
$EndComp
Text Notes 3350 950  0    60   ~ 0
VLF252012MT-220M
$Comp
L VCC #PWR30
U 1 1 55712691
P 4200 5100
F 0 "#PWR30" H 4200 5200 30  0001 C CNN
F 1 "VCC" H 4200 5200 30  0000 C CNN
F 2 "" H 4200 5100 60  0000 C CNN
F 3 "" H 4200 5100 60  0000 C CNN
	1    4200 5100
	1    0    0    -1  
$EndComp
$Comp
L VCC #PWR16
U 1 1 557127D3
P 3200 6650
F 0 "#PWR16" H 3200 6750 30  0001 C CNN
F 1 "VCC" H 3200 6750 30  0000 C CNN
F 2 "" H 3200 6650 60  0000 C CNN
F 3 "" H 3200 6650 60  0000 C CNN
	1    3200 6650
	1    0    0    -1  
$EndComp
$Comp
L VCC #PWR25
U 1 1 5571280F
P 3800 5650
F 0 "#PWR25" H 3800 5750 30  0001 C CNN
F 1 "VCC" H 3800 5750 30  0000 C CNN
F 2 "" H 3800 5650 60  0000 C CNN
F 3 "" H 3800 5650 60  0000 C CNN
	1    3800 5650
	1    0    0    -1  
$EndComp
$Comp
L VCC #PWR24
U 1 1 557128ED
P 3750 4600
F 0 "#PWR24" H 3750 4700 30  0001 C CNN
F 1 "VCC" H 3750 4700 30  0000 C CNN
F 2 "" H 3750 4600 60  0000 C CNN
F 3 "" H 3750 4600 60  0000 C CNN
	1    3750 4600
	1    0    0    -1  
$EndComp
Wire Wire Line
	1700 2950 1100 2950
Text Label 1100 2950 0    60   ~ 0
USR_BUTTON
Wire Wire Line
	5400 3200 5700 3200
Wire Wire Line
	5700 3200 5700 3300
$Comp
L GND #PWR32
U 1 1 5AAEF1AA
P 5700 3300
F 0 "#PWR32" H 5700 3050 50  0001 C CNN
F 1 "GND" H 5700 3150 50  0000 C CNN
F 2 "" H 5700 3300 50  0001 C CNN
F 3 "" H 5700 3300 50  0001 C CNN
	1    5700 3300
	1    0    0    -1  
$EndComp
Wire Wire Line
	5000 3200 4350 3200
Text Label 4350 3200 0    60   ~ 0
USR_BUTTON
$Comp
L SW_Push SW1
U 1 1 5AAF70D8
P 5200 3200
F 0 "SW1" H 5250 3300 50  0000 L CNN
F 1 "SW_Push" H 5200 3140 50  0000 C CNN
F 2 "Buttons_Switches_SMD:SW_SPST_EVQPE1" H 5200 3400 50  0001 C CNN
F 3 "" H 5200 3400 50  0001 C CNN
	1    5200 3200
	1    0    0    -1  
$EndComp
$EndSCHEMATC
