Lux Hub Hardware
================

History
-------

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
- *Greenwire*: Inrush limiter changed: `R42 -> C; C1 -> R; R1 unpop`
