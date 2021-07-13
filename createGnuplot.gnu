set terminal png size 800,600 
set output 'Speed_Comparison.png'

#set key inside bottom right
set xlabel 'Matrix size'
set ylabel 'Time (Milliseconds)'
set title 'Matrix multiplication with gcc'
plot  "Algo1Time.txt" using 1:2 title 'Original'with linespoint, \
      "Algo2Time.txt" using 1:2 title 'Opt' with linespoint
