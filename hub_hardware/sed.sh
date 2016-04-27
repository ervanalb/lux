cp "lux hub.kicad_pcb" "lux hub.kicad_pcb.bkp"
sed -r '/fp_text/N;s/(\(fp_text.+\n.*\(size )[^)]+\) \(thickness [^)]+/\10.762 0.762) (thickness 0.130/' "lux hub.kicad_pcb.bkp" > "lux hub.kicad_pcb"
