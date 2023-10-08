#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <cstdlib>
#include <cmath>
#include <csignal>
#include <unistd.h>

pthread_t *thr_id;

void *izracun_eksponencijala (void *x) {
	int a;
	int n = *((int *) x);
	printf ("Dretva %d", n);
	scanf (" Unesi broj: %d", &a);
	return NULL;
}

void clear_a (int i) {
//	for (i = 0; i < n; i++) {
//		pthread_cancel (thr_id[i]);
//		pthread_kill (thr_id[i], SIGKILL);
//	}
//	delete (thread_id);
	exit(0);
}

int main(int argc, char *argv[]) {
	int n, i;
	
	if (argc != 2) {
		printf ("Broj argumenata je kriv. Izlaz\n");
		return 1;
	}
	n = atoi (argv[1]);
/*	struct sigaction newact;
	newact.sa_handler = clear_a;
	sigemptyset(&newact.sa_mask);
	if (sigaction (SIGINT, &newact, NULL) == -1)
		return 1;
		*/
		printf ("%d", argc);

	if (argc != 2) {
		printf ("Broj argumenata je kriv. Izlaz\n");
		return 1;
	}
	n = atoi (argv[2]);
	if (n < 1) {
		printf ("Broj dretava mora biti >= 1");
		exit (1);
	}
	printf ("Broj dretava: %d", n);
	
	// Stvaranje dretva
	// Alokacija
	/*thr_id = (pthread_t *) malloc (sizeof(pthread_t) * n);
	
//	printf ("Element po dretvi: %d\n", el_po_dretvi);
//	printf ("Br. elemenata za zadnju dretvu: %d\n", el_po_dretvi_zadnja);
	for (i = 0; i < n; i++) {
		pthread_create (&thr_id[i], NULL, izracun_eksponencijala, &i);
		usleep(1000); 
	}
	for (i = 0; i < n; i++) {
		pthread_join (thr_id[i], NULL);
	}
	printf ("\n\nEksponencijale: \n");
//	for (i = 0; i < l; i++) {
//		printf ("%.12Lf\n", vektor_b[i]);
//	}
	
	clear_a (1);*/
	return 0;
}
