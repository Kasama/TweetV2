all:
	gcc -o main *.c -Wall
clear:
	rm main *.dat *.itl *.ill *.itf *.ilf
aaaa:
	./main teste < in
clean:
	rm main *.dat *.itl *.ill *.itf *.ilf
