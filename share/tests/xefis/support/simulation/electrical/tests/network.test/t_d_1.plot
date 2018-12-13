#!/usr/bin/env gnuplot

plot 't_d_1.dat' using 1:2 with lines lw 1 title 'V1', \
     't_d_1.dat' using 1:3 with lines lw 1 title 'D1', \
     't_d_1.dat' using 1:4 with lines lw 1 title 'R1', \

pause -1;

