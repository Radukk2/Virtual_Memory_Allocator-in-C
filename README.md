Virtual-memory-allocator

1. main.c
In main e face apelul functiilor, citind mai intai comenzile propriu zise, si apoi paramentrii functiilor.

2. vma.h
Headerul vma.h fisierul care contine structurile: nod, lista arena_t, block_t si miniblock_t, dar si functiile care urmeaza sa fie apelate in main.

3. vma.c
Acesta este fisierul care face de fapt toate operatiile. Functiile din acest fisier se impart in trei categorii: functiile pentru operatiile pe listele dublu inlantuite, functiile principale pentru rularea programului si functiile ajutatoare care fac anumite operatii pentru functiile principale(sau care mentin restrictiile de linii). Voi incepe prin a descrie functiile listelor, apoi functiile principlae, iar pe cele ajutatoare le voi descrie pe parcurs ce voi ajunge la ele.
	1. create_list este fucntia care creeaza o lista propriu-zisa, allocandu-i memorie. Functia seteaza list->head-ul pe NULL si pune dimensiunea listei pe 0;
	
	2. get_node este functia care gaseste un nod in functie de pozitia primita ca parametru;
	
	3. add_node este functia care adauga creeaza un nod cu data primita ca parametru, si il adauga pe o pozitie primita de la tastatura. Functia trateaza mai mule cazuri particulare: cel in care 	lista este goala, cel in care nodul trebuie adaugat pe prima pozitie, cel in care nodul trebuie adaugat pe ultima pozitie, dar si cel in care nodul trebuie adaugat undeva la mijloc. dupa apelul 	acestei functii este necesara o eliberare a datelor adaugate, deoarece acestea au fost copiate in nodul proaspat introdus;
	
	4. remove_node este functia care scoate un nod din lista. Asemanator cu add_node functie remove_node trateaza aceleasi cazuri dar si cazul in care trebuie scos elementul de pe pozitia 	1 (elementele sunt indexate de la 0, deci al doilea element). remove_node intoarce un pointer catre nodul proaspat scos din functie, asadar el va fi eliberat ulterior;
	
	5. free_function este functia care parcurge toata lista eliberand toate elementele, si apoi eliberand si sturctura de lista;
	
	6. alloc_arena este functia care aloca memorie structurii de arena, seteaza arena->sizeul, atribuindu-i parametrul primit de la tastatura, si creeaza lista de blockuri;
	
	7. alloc_block este functia care creeaza un block in arena. Mai intai verifica daca parametrii primiti de la tastatura sunt corecti(in caz contrar returnand mesajele de eroare), apoi creeaza structura de block, alocandu-i sizeulsi creandu-i lista de miniblockuri. In acelasi loc se creaza un miniblock in interiorul blockului. Miniblockului i se da un size, care coincide cu cel al blocului, i se atribuie permisiuni de scriere si de citire, si i se aloca bufferul(aceste atribuiri se faca in functia creator care creeaza un miniblock). Apoi prin intermediul functiei intersection aflam, in primul rand, daca blockul nu se intersecteaza cu alte blockuri, si daca nu, pozitia in arena->alloc_list pe care acesta ar trebui pus. Se adauga blockul in lista de blockuri si se verifica daca acesta se va unii cu alte blockuri adiacente. Daca da,  functia merge adauga a doua lista de miniblockuri la prima, urmand ca in functia mare sa se elibereze blockul ramas gol, dar si lista ale carei elemente au fost adaugate;
	
	8. free_block, in ciuda numelui, este functia care elibereaza cate un miniblock. Functia block_finder cauta blockul in care se gaseste miniblockul. In cazul in care acesta nu este gast se afiseaza mesajul de eroare si iese din funcite. In blockul gasit se cauta miniblockul de care avem nevoie si i se retine sizeul, prin variabila counter, initializata cu -1. In cazul in care aceasta ramane -1, se afiseaza mesajul de eroare si se iese din functie. Altfel avem 3 cazuri. Daca miniblockul se afla singur in block, atunci dealocam tot ce continea miniblockul, lista de miniblockuri si tot ce ma continea blockul(size, etc). Al doilea caz este cel in care miniblockul se afla fie la inceput, fie la sfarsit. In acest caz se recalibreaza dimensiunile blockului, se inlatura nodul din lista si se elibereaza memoria lui(in functiile free_block_and_mini_list, free_node_ande_data se face exact ceea ce spune si numele functiei). Functia auxiliara block_nr gaseste numarul blockului ca sa il putem sterge din lista. Ultimul caz, si cel mai complicat este cel in care se extrage un miniblock din mijloc. Atunci programul creeaza doua liste separate ale caror dinemnsiuni sunt actualizate ca atare. Primului block i se atribuie prima lista e blockuri pana la miniblockul eliberat, iar celui de-al doilea i se atribuie de la primul element de dupa cel eliberat pana la final. Apoi blockul initial se elibereaza cu tot ceea ce continea;
	
	9. dealloc_arena este functia in care se elibereaza toata memoria, parcurgand fiecare block, eliberandu-i lista de miniblockuri si rwbufferul, prin functia de eliberare aferenta listelor, dar si prin functia free_rw_buffer care parcurge lista de miniblockuri si ii elibereaza bufferul; 
	
	10. read & write sunt doua functii destul de similare, asa ca voi vorbii despre ele in paralel. In primul rand se cauta blockul, si daca nu il gasim afisam mesajul de eroare si iesim din functie. Se da mesajul de warning daca sizeul citit este mai mare decat maximul posibil. Retinem true_sizeul care trebuie citi/scris. Se cauta apoi miniblockul de unde este adresa data. In functia write se aloca din nou bufferul pentru a putea contine valori char. Apoi pentru fiecare se verifica permisiunile de scriere/citire. Daca cel putin un miniblock dintre cele pe care se va opera nu au permisiuni, citirea/scrierea nu se poate realiza. Intram apoi in whileul de citire/scriere. Se ia un curosr care verifica daca s a ajuns la adresa pe care trebuie sa se scrie/citeasca. Cand se ajunge in punctul respectiv se face face operatia propriu-zisa. In write se atirbuie valoarea pozitie din vectorul rw_buffer, iar in read se afiseaza acea valoare. Apoi s verifica daca este necesara tecerea in urmatorul miniblock;
	
	11. pmap este o functie destul de simpla care afiseaza memoria totala, cea ramasa, nr de blockuri si de miniblockuri si adresele de start si stop ale acestora; 
	
	12. Functia mprotect primeste permisiunile prelucrate(acestea au fost date ca stringuri insa prin functiile interpretare_string si verificare_str, le-am transformat in valoare numerica. Verificam existenta miniblockului. Daca acesta exista, se actualizeaza permisiunile. Daca nu, se iese din functie;
	
