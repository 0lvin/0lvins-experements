// gcc unshare_example.c -o unshare_example
#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
	// Attempt to unshare the network namespace
	if (unshare(CLONE_NEWNET) == -1) {
		perror("unshare");
		exit(EXIT_FAILURE);
	}

	printf("Network namespace unshared successfully.\n");

	// Execute a command to observe the effects
	// For instance, `ifconfig` or `ip addr` will show no network interfaces except `lo`
	system("ip addr");

	// Keep the process running to explore the namespace (for example, from another terminal)
	printf("Press enter to exit...\n");
	getchar();

	return 0;
}
