all:
	gcc -o main *.c -Wall
clear:
	rm main *.dat *.itl *.ill *.itf *.ilf
dotests:
	./main teste < bateriaDeTestes
clean:
	rm main *.dat *.itl *.ill *.itf *.ilf
