cp led\ spot.kicad_pcb led\ spot_old.kicad_pcb
sed -r '/fp_text/N;s/(\(fp_text.+\n.*\(size )[^)]+\) \(thickness [^)]+/\10.762 0.762) (thickness 0.130/' led\ spot_old.kicad_pcb > led\ spot.kicad_pcb
