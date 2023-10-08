#include <unistd.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

pthread_t *thr_id;
pthread_mutex_t mut;
pthread_cond_t cond;
int n;

void *funkc (void *x) {
	int a;
	int dnum = *((int *) x);

	pthread_mutex_lock (&mut);

	// Ovo je K.O. lock zapravo samo. Svaki put kada dretva zaključa mutex onemogući durigm dretvama ulazak i dretve tu čekaju.
	printf ("Dretva %d. Unesite broj\n", dnum);
	scanf ("%d", &a);
	if (dnum < (n-1))
		pthread_cond_wait (&cond, &mut);
	else
		pthread_cond_broadcast (&cond);

	printf ("\nDretva %d. Unesen broj je: %d", dnum, a);
	pthread_mutex_unlock (&mut);	// Ovo je kao da je smeafor 1

	return NULL;
}

void clear_a (int i) {
//	for (i = 0; i < n; i++) {
//		pthread_cancel (thr_id[i]);
//		pthread_kill (thr_id[i], SIGKILL);
//	}
//	delete (thread_id);
	printf ("\n");
	pthread_mutex_destroy (&mut);
	pthread_cond_destroy (&cond);
	exit(0);
}

int main(int argc, char *argv[]) {
	int i;
	pthread_mutex_t mut2;

	if (argc != 2) {
		printf ("Broj argumenata je kriv. Izlaz\n");
		return 1;
	}
	n = atoi (argv[1]);
	if (n < 1) {
		printf ("Broj dretava mora biti >= 1");
		exit (1);
	}
	printf ("Broj dretava=%d\n", n);

	// Pos tavljanje signala
	struct sigaction newact;
	newact.sa_handler = clear_a;
	sigemptyset(&newact.sa_mask);
	if (sigaction (SIGINT, &newact, NULL) == -1)
		return 1;
		
	// Postavljanje mutexa
	if (pthread_cond_init (&cond, NULL) == -1) {
		printf ("sem_init failed.\n");
	}
	if (pthread_mutex_init (&mut, NULL) == -1) {
		printf ("pthread_mutex_init failed");
	}

	// Stvaranje dretva
	// Alokacija
	thr_id = (pthread_t *) malloc (sizeof(pthread_t) * n);
	
//	printf ("Element po dretvi: %d\n", el_po_dretvi);
//	printf ("Br. elemenata za zadnju dretvu: %d\n", el_po_dretvi_zadnja);
	for (i = 0; i < n; i++) {
		pthread_create (&thr_id[i], NULL, funkc, &i);
	}

	for (i = 0; i < n; i++) {
		pthread_join (thr_id[i], NULL);
	}
	
	clear_a (1);
	return 0;
}
