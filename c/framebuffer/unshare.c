#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/wait.h>
#include <stdio.h>


int main() {
	int res = 0;
	int pid = 0;
	res = unshare(CLONE_NEWCGROUP | CLONE_NEWUSER | CLONE_NEWIPC |
		      CLONE_NEWNS | CLONE_NEWNET | CLONE_NEWPID | CLONE_NEWUTS);

	if (res < 0) {
		perror("unshare");
		return 0;
	}

	pid = fork();
	if (pid < 0) {
		perror("no fork?");
		return 0;
	}
	if(pid) {
		printf("I am parent for %d\n", pid);
		wait(NULL);
		return 0;
	}

	printf("I am child for %d\n", pid);

	res = umount("/proc");
	if (res < 0) {
		perror("unmount proc, not critical");
	}

	res = mount("proc", "/proc", "proc", 0, 0);
	if (res < 0) {
		perror("mount new proc");
		return 0;
	}
	printf("New proc mounted");

	res = execl("/bin/bash", "/bin/bash", NULL);
	if (res < 0) {
		perror("execl");
	}
}
