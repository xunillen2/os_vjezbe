#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

int m, n;

void player_input (char player) {
        int input, res;
        printf ("Zigica igrac %c uzeo sa stola:\n", player);

        if (scanf ("%d", &input) != 1) {
                printf ("Pogreska prilikom unosa.\n");
		exit (1);
        }
        if ((input > m) || (input < 1)) {
                printf ("Nije moguce uzeti taj broj zigica\n");
		return;
        }
        n = n - input;
}

void sigint_handler (int sig) {
	player_input ('A');
	if (n < 1) {
                printf ("Zigica na stolu: 0. Pobjedio igrac B\n");
		exit (0);
	}
}
void sigquit_handler (int sig) {
	player_input ('B');
	if (n < 1) {
                printf ("Zigica na stolu: 0. Pobjedio igrac A\n");
		exit (0);
        }
}

void sigtstp_handler () {
	exit (0);
}

int set_sigact(int signal, void (*handler)) {
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
	newact.sa_handler = handler;
	sigemptyset(&newact.sa_mask);
	if (sigaction (signal, &newact, NULL) == -1)
		return 1;
	return 0;
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf ("Broj argumenata je kriv. Izlaz\n");
		return 1;
	}
	m = 3;
	n = atoi (argv[1]);
	if (n <= m) {
		printf ("Broj zigica na stolu mora biti za barem jedan veci od preddefinirane varijable m\n");
		exit (1);
	}
	if (m < 3) {
		printf ("Preddefinirani broj m mora biti veci od 2\n");
		exit (1);
	}

	if (set_sigact (SIGINT, sigint_handler) || set_sigact (SIGQUIT, sigquit_handler) ||
	set_sigact (SIGTSTP, sigtstp_handler)) {
		printf ("Problem tijekom postavljanja akcija za signale. Izlaz....");
		exit (1);
	}

	printf ("M = %d. N = %d\n", m, n);
	printf ("Parametri su ispravni. Zapocinjem igru.\n");
	while (1) {
		printf ("Zigica na stolu: %d. Na redu igrac: A\n", n);
		pause ();
		printf ("Zigica na stolu: %d. Na redu igrac: B\n", n);
		pause ();
	}
	return 0;
}
