#!/usr/bin/env gnuplot

plot 't_c_1.dat' using 1:2 with lines lw 1 title 'V1', \
     't_c_1.dat' using 1:3 with lines lw 1 title 'R1', \
     't_c_1.dat' using 1:4 with lines lw 1 title 'C1', \

pause -1;

