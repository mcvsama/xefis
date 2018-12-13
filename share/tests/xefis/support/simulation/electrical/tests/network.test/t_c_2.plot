#!/usr/bin/env gnuplot

plot 't_c_2.dat' using 1:2 with lines lw 1 title 'R1', \
     't_c_2.dat' using 1:3 with lines lw 1 title 'C1', \
     't_c_2.dat' using 1:4 with lines lw 1 title 'C2', \
     't_c_2.dat' using 1:5 with lines lw 1 title 'S1', \
     't_c_2.dat' using 1:6 with lines lw 1 title 'S2', \

pause -1;

