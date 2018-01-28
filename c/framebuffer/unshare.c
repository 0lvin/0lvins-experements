#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>


int create_for_mount(const char *target, mode_t mode) {
	int res = 0;

	res =  mkdir(target, mode);
	if (res < 0) {
		perror("mkdir");
		printf("can't create '%s' dir for mount\n", target);
	}

	printf("New dir for %s mount\n", target);
	return 0;
}

int main() {
	int res = 0;
	int pid = 0;
	char cwd[4048] = "\0";

	// create and mount all dirs
	if (create_for_mount("sys", S_IRUSR | S_IXUSR |
				    S_IRGRP | S_IXGRP |
				    S_IROTH | S_IXOTH) != 0) {
		return 0;
	}

	if (create_for_mount("proc", S_IRUSR | S_IXUSR |
				     S_IRGRP | S_IXGRP |
				     S_IROTH | S_IXOTH) != 0) {
		return 0;
	}

	if (create_for_mount("tmp", S_IRUSR | S_IWUSR | S_IXUSR |
				    S_IRGRP | S_IWGRP | S_IXGRP |
				    S_IROTH | S_IWOTH | S_IXOTH) != 0) {
		return 0;
	}

	if (create_for_mount("dev", S_IRUSR | S_IWUSR | S_IXUSR |
				    S_IRGRP | S_IXGRP |
				    S_IROTH | S_IXOTH) != 0) {
		return 0;
	}

	// go to separate namespace
	res = unshare(CLONE_NEWCGROUP | CLONE_NEWUSER | CLONE_NEWIPC |
		      CLONE_NEWNS | CLONE_NEWNET | CLONE_NEWPID | CLONE_NEWUTS);

	if (res < 0) {
		perror("unshare");
		return 0;
	}

	// fork
	pid = fork();
	if (pid < 0) {
		perror("no fork?");
		return 0;
	} else if (pid) {
		printf("I am parent for %d\n", pid);
		wait(NULL);
		umount("sys");
		umount("proc");
		umount("tmp");
		umount("dev");
		return 0;
	}

	printf("I am child for %d\n", pid);

	// hide origin root
	if (getcwd(cwd, sizeof(cwd)) == NULL) {
		perror("current dir");
		return 0;
	}

	printf("Curent dir is %s\n", cwd);

	res = chroot(cwd);
	if (res < 0) {
		perror("chroot");
		return 0;
	}

	// create and mount all dirs
	res =  mount("sysfs", "sys", "sysfs", 0, 0);
	if (res < 0) {
		perror("mount sys");
		return 0;
	}

	res =  mount("proc", "proc", "proc", 0, 0);
	if (res < 0) {
		perror("mount sys");
		return 0;
	}

	res =  mount("tmpfs", "tmp", "tmpfs", 0, 0);
	if (res < 0) {
		perror("mount sys");
		return 0;
	}

	res =  mount("tmpfs", "dev", "tmpfs", 0, 0);
	if (res < 0) {
		perror("mount dev, not critical");
		return 0;
	}

	res = execl("/bin/sh", "/bin/sh", NULL);
	if (res < 0) {
		perror("execl");
	}
}
