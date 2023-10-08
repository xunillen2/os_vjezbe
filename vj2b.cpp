#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <cstdlib>
#include <cmath>
#include <csignal>
#include <unistd.h>

using namespace std;

long double *vektor_a;
long double *vektor_b;
int m, n;
struct thread_inf {
	int id;
	int el_start;
	int el_num;
	int el_end;
};
pthread_t *thr_id;
thread_inf *thread_vars;

long double factorial (int num) {
	if (num <= 1)
		return 1;
	return num * factorial(--num);
}

void *izracun_eksponencijala (void *x) {
	struct thread_inf thread_vars = 
		*(struct thread_inf *) x;
	long double res = 0;
	long double num;
//	printf ("Dretva %d obrađuje od %d-%d\n\n",
//		thread_vars.id, thread_vars.el_start, thread_vars.el_end);
	for (int b = thread_vars.el_start; b < thread_vars.el_end; b++) {
		num = vektor_a [b];
//		printf ("Dretva %d obrađuje broj: %.12Lf\n", thread_vars.id, num);
		for (int i = 0; i <= m; i++) {
			res += pow (num, i) / factorial (i);
		}
		vektor_b[b] = res;
		//printf ("resultat: %.12Lf\n", res);
		res = 0;
	}
	return NULL;
}

void clear_a (int i) {
//	for (i = 0; i < n; i++) {
//		pthread_cancel (thr_id[i]);
//		pthread_kill (thr_id[i], SIGKILL);
//	}
	free (vektor_a);
	free (vektor_b);
	delete (thread_vars);
	exit(0);
}

int main(int argc, char *argv[]) {
	int l, i,				// l = vektori, n = dretve
		el_start;			// broj elemenata po dretvi;
	int el_po_dretvi, el_po_dretvi_zadnja;
	
	sigset (SIGINT, &clear_a);		// Postavi signal
	
	if (argc != 4) {
		printf ("Broj argumenata je kriv. Izlaz\n");
		return 1;
	}
	l = atoi (argv[1]);
	m = atoi (argv[2]);
	n = atoi (argv[3]);
	if (n < 1) {
		printf ("Broj dretava mora biti >= 1");
		exit (1);
	}
	
	// Alokacija
	vektor_a = (long double *) 
		malloc (sizeof(long double) * l);
	vektor_b = (long double *) 
		malloc (sizeof(long double) * l);

	printf ("\nEksponenti:\n");
	// Random gen.
	srand (time(NULL));
	for (i = 0; i < l; i++) {
		//scanf ("%.12Lf", &vektor_a[i]);		Makni komentar za sample upis
		//printf ("%.12Lf\n", vektor_a[i]);
		vektor_a[i] = (long double)rand()/RAND_MAX*10.0;
		printf ("%.12Lf\n", vektor_a[i]);
	}
	
	// Stvaranje dretva
	// Alokacija
	thr_id = new pthread_t[n];
	thread_vars = new thread_inf [n];
	// Izračun elemenata po dretvi
	el_po_dretvi = l / n;
	el_po_dretvi_zadnja = l % n;
	el_start = 0;
//	printf ("Element po dretvi: %d\n", el_po_dretvi);
//	printf ("Br. elemenata za zadnju dretvu: %d\n", el_po_dretvi_zadnja);
	for (i = 0; i < n; i++) {
	///	printf ("el_start dretve %d: %d", i, el_start);
		thread_vars[i].id = i;
		thread_vars[i].el_start = el_start;
		thread_vars[i].el_num = el_po_dretvi;
		thread_vars[i].el_end = el_po_dretvi + el_start;
		if (i == (n-1))	{					// Jebeno ružno napisano. Ispravim sutra. Ugl. el_po_dretvi_zadnja je samo da se vidi koliko zadnja dretva mora 
									// dodatnih el. obradit.
			thread_vars[i].el_num += el_po_dretvi_zadnja;
			thread_vars[i].el_end += el_po_dretvi_zadnja;
		}
		pthread_create (&thr_id[i], NULL, izracun_eksponencijala, &thread_vars[i]);
		
		el_start += el_po_dretvi;
		  usleep(1000); 
	}
	for (i = 0; i < n; i++) {
		pthread_join (thr_id[i], NULL);
	}
	printf ("\n\nEksponencijale: \n");
	for (i = 0; i < l; i++) {
		printf ("%.12Lf\n", vektor_b[i]);
	}
	
	clear_a (1);
	return 0;
}
