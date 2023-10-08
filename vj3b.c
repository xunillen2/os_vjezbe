#include <unistd.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <csignal>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/shm.h>

#define PISI	0
#define PUN	1
#define	PRAZAN	2
#define	POSTAVI	1
#define ISPITAJ	-1

key_t seg_key;
int SemId;   /* identifikacijski broj skupa semafora */
int br_procesa;
int *ULAZ, *IZLAZ, *UKUPNO, *M;

void SemGet(int n) {  /* dobavi skup semafora sa ukupno n semafora */
   SemId = semget(IPC_PRIVATE, n, 0600);
   if (SemId == -1) {
      printf("Nema semafora!\n");
      exit(1);
   }
}

int SemSetVal(int SemNum, int SemVal) {  /* postavi vrijednost semafora SemNum na SemVal */
   return semctl(SemId, SemNum, SETVAL, SemVal);
}

/*
int semop(int semid, struct sembuf (*sops)[], int nsops); koristimo za rad sa semaforima
*/
int SemOp(int SemNum, int SemOp) {  /* obavi operaciju SemOp sa semaforom SemNum */
   struct sembuf SemBuf;

   SemBuf.sem_num = SemNum;
   SemBuf.sem_op  = SemOp;
   SemBuf.sem_flg = 0;
   return semop(SemId, &SemBuf, 1);
}

void SemRemove() {  /* unisti skup semafora */  
 semctl(SemId, 0, IPC_RMID, 0);	
}

void potrosac () {
	int total = 0;
	for (int i = 0; i < *UKUPNO; i++) {
		SemOp (PRAZAN, ISPITAJ);
		printf ("Potrosac prima %d\n", M[*IZLAZ]);
		
		total += M[*IZLAZ];
		*IZLAZ = ((*IZLAZ)+1) % 5;
		sleep (1);
		SemOp (PUN, POSTAVI);
	}
	printf ("Potrosac - zbroj primljenih brojeva = %d", total);
	return;
}
void proizvodac (int br_rng, int id) {
	for (int i = 0; i < br_rng; i++) {
		SemOp (PUN, ISPITAJ);
		SemOp (PISI, ISPITAJ);

		M[*ULAZ] = (rand()%1000) + 1;
		printf ("Proizvodac %d salje %d\n", id, M[*ULAZ]);
		*ULAZ = ((*ULAZ)+1) % 5;
		sleep (1);

                SemOp (PISI, POSTAVI);
                SemOp (PRAZAN, POSTAVI);
	}
	printf ("Proizvodac %d zavrsio sa slanjem.\n", id);
	return;
}

void clear_a (int i) {
	for (int i = 0; i < br_procesa+1; i++)
		wait(NULL);
	shmdt(ULAZ);	// Brišemo sve zajedničke var.
	shmdt(IZLAZ);
	shmdt(UKUPNO);
	shmdt(M);
	shmctl (seg_key, IPC_RMID, NULL);
	SemRemove();	// semctl samo sa IPC_RMID zastavicom.
	exit (0);
}

/*
Semafori
--------------
* dva stanja - propisno i nepropusno
* Brojac se smanjuje kada zahtjeva semafor, a povecava kada se oslobada. -> ako je na nuli, a zahtjeva se semafor, tada proces ceka da neki drugi proces oslobodi semafor (poveca vrijednost)
*/
int main (int argc, char *argv[]) {
	int br_rng;

	if (argc != 3) {
		printf ("Broj argumenata je kriv. Izlaz\n");
		return 1;
	}
	// Castanje i provjera argumenata
	br_procesa = atoi (argv[1]);
	br_rng = atoi (argv[2]);
	if (br_procesa < 1) {
		printf ("Broj procesa nesmije biti manji od 1\n");
		return 1;
	}
	if (br_rng < 1) {
		printf ("Broj brojeva koje potrosac mora generirati ne smije biti manji od 1");
		return 1;
	}

	// Postavljanje rand gen seeda.
	srand (time(NULL));

// Postavljanje signala SIGINT
//	Ok, kak to dela?
//	sigaction struktura:
//	struct sigaction {
//	union {		/* signal handler */
//		void	(*__sa_handler)(int);
//		void	(*__sa_sigaction)(int, siginfo_t *, void *);
//	} __sigaction_u;
//	sigset_t sa_mask;          /* signal mask to apply */
//	int	 sa_flags;         /* see signal options below */
//	};
//	sigaction(int sig, const struct sigaction *act, struct sigaction *oact);
//
//	sigaction - dodjeljuje akciju odredenom signalu (difiniran sa sig). Ako je act neki broj, onda
//	se gleda kao jedan od akcija (SIG_DFL...), a maska se moze koristit za slanje specificnih signala.
//	Vise signala odjednom se moze procesirat, ali oni koji su od strane "trapa" se prvi procesiraju.
//	Ostali signali koji su u redu cekanja, mogu se dobit sigpending() funkcijom.
//	Zato ako posaljemo dva puta SIGINT signal, igrac A ide dva puta zaredom.
//	Kada se uhvati signal, trenutno stanje procesa se sprema, nova maska signala se kalkulira, te se poziva handler.
//
//	struct:
//	union - Moze bit postavljen samo jedan u unionu, sa_handler (funkcija kao handler. pointer) ili sa_sigaction (SIG_DFL, SIG_IGN).
//	Sa tim definiramo ponasanje programa prilikom dolaska signala.
//	ako je sa_handler postavljen na SIG_DFL, default akcija je da se signal odbaci
//	sa_mask	 - Specificira masku koja govori koji signali trebaju biti bllokirani.
//	sa_flags - Definira ponasanje poslanog signala. To samo stavit prazno.
	struct sigaction newact;
	newact.sa_handler = &clear_a;
	sigemptyset(&newact.sa_mask);
	if (sigaction (SIGINT, &newact, NULL) == -1)
		return 1;

	// Postavljanje zajednicke memorije procesa
	seg_key = shmget (IPC_PRIVATE, 8 * sizeof(uint), 0600);
	if (seg_key == -1) {
		printf ("Greska prilikom dodavanja kljuca za zajednicku memoriju");
		exit (1);
	}
	ULAZ = (int *) shmat (seg_key, NULL, 0);
	*ULAZ = 0;
	IZLAZ = ULAZ + sizeof(int);
	*IZLAZ = 0;
	UKUPNO = IZLAZ + sizeof(int);
	*UKUPNO = br_procesa * br_rng;
	M = UKUPNO + sizeof (int);

	// Semafori
	/* semget(key_t key, int nsems, int flags) - Stvara skup semafora.
	* Ovisno kak nam treba, za key_t šaljemo IPC_PRIVATE sa kojim cemo dobit novi identifikator, ili dobiti postojeci identifikator ako stavimo za key_t drugačiju vrijednost od IPC_PRIVATE.
	* Novi semafori imaju vrijednost 0, to se more namjestit sa semctl funkcijom kasnije (ak hocemo namjestit sve u nizu, onda stavljamo SETALL u cmd).
	*/
	SemGet (3);
	/* Ok, ovo je samo wraper za semctl(int semid, int semnum, int cmd, union semun arg);
	* union semnum {
			int		 val;		 value for SETVAL 
			 semid_ds	*buf;		 buffer for IPC_{STAT,SET}
			u_short		*array;		 array for GETALL & SETALL 
		}
	* Kak dela? Ako se se za cmd SETVAL stavi, a za arg union se stavi neg. broj tada će se namjestit semafor. 
	*/
	SemSetVal (PISI, 1);
	SemSetVal (PUN, 5);
	SemSetVal (PRAZAN, 0);

	// Stvaranje procesa
	// potrosac
	if (!fork ()) {
		potrosac ();
		exit (1);
	}
	// Proizvodac
	for (int i = 0; i < br_procesa; i++) {
		switch (fork ()) {
			case 0:
				proizvodac (br_rng, i+1);
				exit (0);
			case 1:
				printf ("Greska prilikom stvaranja procesa\n");
			default:
				break;
		}
	}
	clear_a(0);
}
