#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/wait.h>
#include <stdio.h>


int main() {
	int res = 0;
	res = unshare(CLONE_NEWIPC | CLONE_NEWNS | CLONE_NEWNET | CLONE_NEWPID |
		      CLONE_NEWUTS);

	if (res < 0) {
		perror("unshare");
		return 0;
	}

	if(fork()) {
		wait(NULL);
		return 0;
	}

	res = umount("/proc");
	if (res < 0) {
		perror("unmount proc, not critical");
	}

	res = mount("proc", "/proc", "proc", 0, 0);
	if (res < 0) {
		perror("mount new proc");
	}

	res = execl("/bin/bash", "/bin/bash", NULL);
	if (res < 0) {
		perror("execl");
	}
}
