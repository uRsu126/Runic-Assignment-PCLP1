//Urse Andrei 312CB

Programul este modularizat in jurul structurii 'state' care retine
sistemul Lindenmeyer incarcat, imaginea curenta si istoricul comenzilor.
Procesul de generare a sirului de caractere bazat pe axioma si reguli 
gestioneaza dinamic memoria, realocand spatiu pentru noul sir la fiecare
iteratie. Sirul se parcurge caracter cu caracter.
Imaginea este stocata ca o matrice liniarizata de pixeli.
Pentru grafica, am folosit o stiva pentru a memora pozitia si unghiul curent
pentru a permite crearea structurilor complexe. Liniile sunt trasate folosind
Algoritmul Bresenham implementat cu numere intregi. 
La logica undo/redo am folosit structura "singly linked list", care retine sirul
de caractere al comenzii executate. Structura mentine un pointer la capatul listei
si un contor care indica numarul curent de comenzi, momentul la care se afla istoricul.
Astfel, pentru undo, se executa din nou comenzile de la inceput pana la cea dinaintea 
careia i-a fost dat undo (primele n - 1 comenzi). Pentru redo se incrementeaza indexul si 
se executa din nou comenzile pana la pozitia n + 1. 