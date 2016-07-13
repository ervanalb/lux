Lux Hub Hardware
================

History
-------

### Rev 6

#### Errata

- 

##### Bad parts

- U3
- R42
- C3 is short
- Fuse clips
- CON1 holes should be bigger

### Rev 5

- STM32F042
- 6 Lux outputs
    - 48V power, 5V/0V RS-422 data
    - Data lines tied together
    - Rx/Tx enable are indepedent
    - TVS on data lines
    - Per-output PTC fuses (which weren't useful)
- USB for comms & non-48V power
    - Spec'd for 500mA current draw to drive 6 ports
- *Greenwire*: 3v3 reg `U1` pads are incorrect
- *Greenwire*: Inrush limiter changed: `R42 -> C; C1 -> R; R1` unpopulated
- *Greenwire*: RX pin incorrect; shorted to TX LED
