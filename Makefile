#
# @Authors:
#
# Carlos Alberto Schneider Junior - 9167910
# Frederico de Azevedo Marques    - 8936926
# Henrique Martins Loschiavo      - 8936972
# Lucas Kassouf Crocomo           - 8937420
# Roberto Pommella Alegro         - 8936756
#
#

all:
	gcc -o main *.c -Wall
clear:
	rm main *.dat *.itl *.ill *.itf *.ilf
dotests:
	./main teste < bateriaDeTeste
clean:
	rm main *.dat *.itl *.ill *.itf *.ilf
