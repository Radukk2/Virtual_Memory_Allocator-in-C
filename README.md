Virtual-memory-allocator

1. main.c
In main e face apelul functiilor, citind mai intai comenzile propriu zise, si apoi paramentrii functiilor.

2. vma.h
Headerul vma.h fisierul care contine structurile: nod, lista arena_t, block_t si miniblock_t, dar si functiile care urmeaza sa fie apelate in main.

3. vma.c
Acesta este fisierul care face de fapt toate operatiile. Functiile din acest fisier se impart in trei categorii: functiile pentru operatiile pe listele dublu inlantuite, functiile principale pentru rularea programului si functiile ajutatoare care fac anumite operatii pentru functiile principale(sau care mentin restrictiile de linii). Voi incepe prin a descrie functiile listelor, apoi functiile principlae, iar pe cele ajutatoare le voi descrie pe parcurs ce voi ajunge la ele.
	
