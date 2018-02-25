/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

int main(int argc, char **argv) {
	int pid = atoi(argv[1]);
	char pathbuf[100];

	snprintf(pathbuf, 100, "/proc/%d/ns/net", pid);
	setns(open(pathbuf, O_RDONLY), 0);

	snprintf(pathbuf, 100, "/proc/%d/ns/ipc", pid);
	setns(open(pathbuf, O_RDONLY), 0);

	snprintf(pathbuf, 100, "/proc/%d/ns/uts", pid);
	setns(open(pathbuf, O_RDONLY), 0);

	snprintf(pathbuf, 100, "/proc/%d/ns/pid", pid);
	setns(open(pathbuf, O_RDONLY), 0);

	snprintf(pathbuf, 100, "/proc/%d/ns/mnt", pid);
	setns(open(pathbuf, O_RDONLY), 0);

	if(fork()) {
		wait(NULL);
		return 0;
	}

	execl("/bin/bash", "/bin/bash", NULL);
}
