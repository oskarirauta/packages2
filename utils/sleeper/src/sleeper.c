#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv) {

	int secs = 2;

	if ( argc > 1 ) {

		secs = atoi(argv[1]);
		if ( secs == 0 )
			secs = 2;
	}

	// printf("secs to wait: %d\n", secs);

	struct timespec ts;
	ts.tv_sec = secs;
	ts.tv_nsec = 0;
	nanosleep(&ts, NULL);

	return 0;
}
