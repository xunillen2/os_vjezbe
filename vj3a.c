#include <unistd.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdio.h>
#include <signal.h>
#include <signal.h>
#include <stdlib.h>

sem_t *sem;
pthread_t *thr_id;

void *zadatak_dretve (void *argv) {
	int br = *(int *)argv;
	int rng = (rand () % 9) + 1;

	switch (br) {
		case 1:
			for (int i = 1; i <= rng; i++) {
				sleep (1);
				printf ("Izvodim zadatak %d: %d/%d\n", br, i, rng);
			}
			sem_post (&sem [4]);
			break;
		case 2:
                        for (int i = 1; i <= rng; i++) {
                                sleep (1);
                                printf ("Izvodim zadatak %d: %d/%d\n", br, i, rng);
			}
			sem_post (&sem [2]);
			sem_post (&sem [3]);
			break;
		case 3:
			sem_wait (&sem [2]);
			for (int i = 1; i <= rng; i++) {
				sleep (1);
				printf ("Izvodim zadatak %d: %d/%d\n", br, i, rng);
			}
			sem_post (&sem [5]);
			break;
		case 4:
			sem_wait (&sem [3]);
			for (int i = 1; i <= rng; i++) {
				sleep (1);
				printf ("Izvodim zadatak %d: %d/%d\n", br, i, rng);
			}
			sem_post (&sem [0]);
			break;
		case 5:
			sem_wait (&sem [4]);
			for (int i = 1; i <= rng; i++) {
				sleep (1);
                                printf ("Izvodim zadatak %d: %d/%d\n", br, i, rng);
			}
			sem_post (&sem [1]);
			break;
		case 6:
			sem_wait (&sem [5]);
                        for (int i = 1; i <= rng; i++) {
                                sleep (1);
                                printf ("Izvodim zadatak %d: %d/%d\n", br, i, rng); 
			}
			sem_post (&sem [6]);
			break;
		case 7:
			sem_wait (&sem [0]);
                        sem_wait (&sem [1]);
                        sem_wait (&sem [6]);
                        for (int i = 1; i <= rng; i++) {
                                sleep (1);
                                printf ("Izvodim zadatak %d: %d/%d\n", br, i, rng);
			}
			break;
		default:
			break;
	}	
}

void clear_a (int i) {
	// Cekamo dretve
	for (int i = 0; i <= 6; i++)
		pthread_join (thr_id[i], NULL);
	for (int i = 0; i <=6; i++)
		sem_destroy (&sem[i]);
	free (thr_id);
	free (sem);
	exit (0);
}


int main (int argc, char *argv[]) {
	int t[7] = {1,2,3,4,5,6,7};

	// Alokacija memorije
	sem = (sem_t *) malloc (sizeof (sem_t) * 7);
	thr_id = (pthread_t *) malloc (sizeof (pthread_t) * 7);
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

	// random gen
	srand (time(NULL));

	// Postavljamo semafore
	for (int i = 0; i <= 6; i++)
		sem_init (&sem[i], 0, 0);
		
	// Alociramo dretve
	for (int i = 0; i <= 6; i++)
		pthread_create (&thr_id[i], NULL, zadatak_dretve, &t[i]);
	// Cekamo dretve
	for (int i = 0; i <= 6; i++)
		pthread_join (thr_id[i], NULL);

	clear_a (0);

}
