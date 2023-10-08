/*
 * ZADATAK:
 * Zamislimo sustav s trima dretvama nakupcima i jednom dretvom veletrgovcem računalnom opremom. Nakupci sastavljaju računala i prodaju ih. ž
 * Jedan nakupac već ima samo neograničenu zalihu monitora, drugi tipkovnica, a treći računala (u kućištu, bez monitora i tipkovnice). 
 * Veletrgovac ima sve tri sastavnice u neograničenim količinama. Iz skladišta donosi nasumice po dvije sastavnice i stavlja ih na stol, ispisuje to i odspava jednu sekundu. 
 * Nakupac kojemu nedostaju baš te dvije sastavnice uzima ih, ispisuje to, odspava jednu sekundu, potpisuje primku veletrgovcu, sastavlja i dostavlja potpuno računalo svojemu kupcu, te se ponovno vraća po nove sastavnice za sljedeću narudžbu. 
 * Tek nakon što mu je potpisana primka, veletrgovac može ići u skladište po nove dvije sastavnice i krug se ponavlja. 
 * Na početku je stol prazan. Napišite program s dretvama nakupaca i veletrgovca tako da se one međusobno ispravno usklađuju. Za sinkronizaciju rabite jezgrin monitor.
 * 
 * 
 * Opis: Nesmimo dopustit da se ponekad ne pristupa podacima istovremeno. To su situacije kada dretve koriste zajedničke varijable, a njihovom porabom se izazove greška.
 * Takvi odsječci se zaštičuju međusobnim isključivanjem.
 *  
 * pthread_mutex_t ključ -> definira karakteristike stvorene variable (NULL ovdje). Sve dretve koje pokušaju lockati muttex, ostaju blokirane dok se muttex ne otključa.
 *					Tu imamo red čekanja.
 * 
 * Uvjetne varijable. Daje nam mogučnost da se čekda da se ispuni odrešeni uvijet. pthread_cond_wait otključava mutex, stavlja dretvu u red čekanja.
 * broadcast sve otključava
 * 
 * Monitor
 * Skup procedura i strukture podataka nad kojima procedure djeluju i nisu vidljive izvan monitora. Ne smiju se izodifit paralelno te moraju 
 * ispuniti uvijet koji utječe na odvijanje zadatka. Ako je uvj. ispunjen, onda se obaljaju potrebne akcije i te se oslobađaju sredstva kako bi drugom monitoru omogučio ulaz.
 * Djelovi monitora:  
 */

#include <unistd.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

/* Konstante po kojima ćemo definirat koji broj predstavlja koji skupac proizvoda */
#define	tipkovnica_racunala	0
#define monitor_racunala	1
#define	monitor_tipkovnica	2

pthread_t *thr_id;
pthread_mutex_t mut;
pthread_cond_t cond_tipkovnica_racunala, cond_monitor_racunala, cond_monitor_tipkovnica, cond_primka;
uint8_t dostupno;

/* Proto */
void nakupac_finish (char *);	// Zajednicka funkcija za postavljanje var. dostupno i postavljanje sig. i unlock
void *nakupac_monitor (void *);
void *nakupac_racunalo (void *);
void *nakupac_tipkovnica (void *);
void *veletrgovac (void *);

void clear_a (int i) {
//	for (i = 0; i < n; i++) {
//		pthread_cancel (thr_id[i]);
//		pthread_kill (thr_id[i], SIGKILL);
//	}
//	delete (thread_id);
	pthread_mutex_destroy (&mut);
	pthread_cond_destroy (&cond_tipkovnica_racunala);
	pthread_cond_destroy (&cond_monitor_racunala);
	pthread_cond_destroy (&cond_monitor_tipkovnica);
	pthread_cond_destroy (&cond_primka);

	exit(0);
}

void *veletrgovac (void *i) {
	while (1) {
        pthread_mutex_lock(&mut);
        
        uint8_t skupac = rand() % 3;	// Generira broj skupca koji vletgovac stavlja
        dostupno = skupac;
       // printf ("%d", dostupno);
        switch (skupac) {
			case monitor_racunala:
				printf ("Veletrgovac stavio monitor i računalo.\n");
				pthread_cond_signal (&cond_monitor_racunala);
				break;
			case monitor_tipkovnica:
				printf ("Veletrgovac stavio monitor i tipkovnicu.\n");
				pthread_cond_signal (&cond_monitor_tipkovnica);
				break;
			case tipkovnica_racunala:
				printf ("Veletrgovac stavio tipkovnicu i računalo.\n");
				pthread_cond_signal (&cond_tipkovnica_racunala);
				pthread_cond_broadcast (&cond_tipkovnica_racunala);
				break;			
		}
		sleep (1);
		
		pthread_cond_wait (&cond_primka, &mut);
        pthread_mutex_unlock(&mut);
	}
}

void *nakupac_monitor (void *i) {
	while (1) {
        pthread_mutex_lock(&mut);
        if (dostupno != tipkovnica_racunala) {
			pthread_cond_wait (&cond_tipkovnica_racunala, &mut);
		}
		// Nakupac uzima
		nakupac_finish ("Nakupac sa monitorima uzeo tipkovnicu i računalo\n");
	}
}

void *nakupac_tipkovnica (void *i) {
	while (1) {
        pthread_mutex_lock(&mut);
        if (dostupno != monitor_racunala) {
			pthread_cond_wait (&cond_monitor_racunala, &mut);
		}
		// Nakupac uzima
		nakupac_finish ("Nakupac sa tipkovnicama uzeo monitor i računalo\n");
	}
}

void *nakupac_racunalo (void *i) {
	while (1) {
        pthread_mutex_lock(&mut);
        if (dostupno != monitor_tipkovnica) {
			pthread_cond_wait (&cond_monitor_tipkovnica, &mut);	/* Znači, postvalja dretvu u uvijet (condition) dok se uvijet ne ispuni. Također otključava mutex
																	Kada dobi signal prva dretva u redu čekanja koja ima taj uvijet prvo zaključa mutex i nastavlja.
																	broadcast otkljuučava svim takvim dretvama da nastave izvršavanje*/
		}
		// Nakupac uzima
		nakupac_finish ("Nakupac sa računalima uzeo monitor i tipkovnicu\n");
	}
}

void nakupac_finish(char *s) {
	dostupno = 4;	// yeah.. jer mi počinje od 0 konstante.
	
	printf ("%s", s);
	sleep (1);
	// Stavljamo cond za primku. tj. da je potpisana primka
	// pthread_cond_broadcast(&cond_primka);
	pthread_cond_signal (&cond_primka);
	pthread_mutex_unlock(&mut);
}

int main(int argc, char *argv[]) {
	uint8_t i;
	
	// Postavljanje signala
	struct sigaction newact;
	newact.sa_handler = clear_a;
	sigemptyset(&newact.sa_mask);
	if (sigaction (SIGINT, &newact, NULL) == -1)
		return 1;
		
	// Postavljanje mutexa
	if (pthread_cond_init (&cond_monitor_racunala, NULL) || 	/* */
			pthread_cond_init (&cond_monitor_tipkovnica, NULL) ||
			pthread_cond_init (&cond_tipkovnica_racunala, NULL) ||
			pthread_cond_init (&cond_primka, NULL) == -1) {
		printf ("pthread_cond_init failed.\n");
	}
	if (pthread_mutex_init (&mut, NULL) == -1) {
		printf ("pthread_mutex_init failed");
	}

	// Stvaranje dretva
	// Alokacija
	thr_id = (pthread_t *) malloc (sizeof(pthread_t) * 4);
	
//	printf ("Element po dretvi: %d\n", el_po_dretvi);
//	printf ("Br. elemenata za zadnju dretvu: %d\n", el_po_dretvi_zadnja);
	for (i = 0; i <= 3; i++) {
		switch (i) {
			case 0:
				pthread_create (&thr_id[i], NULL, veletrgovac, &i);
				break;
			case 1:
				pthread_create (&thr_id[i], NULL, nakupac_monitor, &i);
				break;
			case 2:
				pthread_create (&thr_id[i], NULL, nakupac_racunalo, &i);
				break;			
			case 3:
				pthread_create (&thr_id[i], NULL, nakupac_tipkovnica, &i);
				break;
			default:
				printf ("Kak smo došli do ovjde????\n");
				break;
		}
		//usleep(1000); 
	}

	for (i = 0; i < 3; i++) {
		pthread_join (thr_id[i], NULL);
	}
	
	clear_a (1);
	return 0;
}
