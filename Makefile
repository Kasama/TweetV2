all:
	gcc -o main *.c -Wall
clear:
	rm main *.dat *.itl *.ill *.itf *.ilf
clean:
	rm main *.dat *.itl *.ill *.itf *.ilf
