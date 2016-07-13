LED Strip Hardware
==================

Unified (v5, v6, v7)
------------

v7 Changelog
------------
- Doubled `C1` with `C10` (`+48V` filtering)
- Doubled `C9` with `C11` (`3V3` filtering)
- Added `C12`, 100nF decoupling cap to `U3`
- Improved DFM

v6 Changelog
------------
(Minor revision)
- Added soft-start circuit (`Q1`, `R9`, `R10`) to `+48V`
- Added bias resistors (`R11`, `R12`) to RS-422 lines
- Added transient voltage suppressor (`U5`) to RS-422 lines
- "*Greenwire*": Doubled `C1` (`+48V` filtering)
- "*Greenwire*": Doubled `C9` (`3V3` filtering)
- "*Greenwire*": Added 2x 100nF decoupling caps to `U3`

History
-------

### Rev 5-7

- 2015-2016
- STM32-based
- Comms: RS-422 half-duplex (actually works now)
    - Hub for multiplexing, but not a router
    - Network (UDP) bridge
- Power: 48V + buck
- Rendering: Control-side, full frames sent
- Control: Radiance: https://github.com/zbanks/radiance
    - Visual UI written in C & SDL
    - Effects on a 2D plane, rendered onto strips
    - Tight audio integration & visualization
        - Beat tracking: https://github.com/zbanks/BTrack

### Rev 3-4???

- 2014
- STM32-based
- Comms: RS-422 half-duplex
    - Non-simplex was incredibly unreliable
    - Full router prototyped, never worked
- Power: 48V + buck
- Rendering: Control-side, full frames sent
    - (Bespeckle used intermittently? Unclear)
- Control: Numerous revisions
    - Livecoding python: https://github.com/zbanks/beetle
        - Visual UI with grassroots: https://github.com/zbanks/grassroots
        - #doitlive: https://github.com/zbanks/doitlive
        - Effects projected on a 2D plane and rendered for strips
        - Used audio information (but not well), beats manually sync'd (?)
    - Beat-off: https://github.com/ervanalb/beat-off/tree/python-1d
        - Visual UI
        - 1D LED strips with effects composited
        - Audio info displayed, beats manually sync'd, effects use timing
- Repo: https://github.com/zbanks/aurora

### Rev 2

- Aug 2013
- STM32-based
- Comms: CAN
    - Bastardization of the protocol to get multicast
    - Possibly send-only?
- Power: 48V + buck
- Rendering: Bespeckle, rendering computed on-board
    - https://github.com/zbanks/bespeckle
- Control: Cursedlight, many-keyboards + TUI
    - https://github.com/zbanks/cursedlight
    - Audio manually sync'd
- Repo: https://github.com/ervanalb/canautomation

### Rev 1

- Aug 2012
- CPU: AVR("arduino compatible"!)
- Comms: MIDI
- Power: 12V DC jack
- Rendering: effects computed on board
    - Each MIDI note corresponded to effect + color
- Control: seq24 used for control
    - MIDI clock sync'd from Mixxx
