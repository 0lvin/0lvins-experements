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
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "utils.h"

int main() {
	int res = 0;
	int pid = 0;
	char cwd[4048] = "\0";
	int flags = 0;

	// create and mount all dirs
	mkdir_if_not_exists("sys", S_IRUSR | S_IXUSR |
				   S_IRGRP | S_IXGRP |
				   S_IROTH | S_IXOTH);

	mkdir_if_not_exists("proc", S_IRUSR | S_IXUSR |
				    S_IRGRP | S_IXGRP |
				    S_IROTH | S_IXOTH);

	mkdir_if_not_exists("tmp", S_IRUSR | S_IWUSR | S_IXUSR |
				   S_IRGRP | S_IWGRP | S_IXGRP |
				   S_IROTH | S_IWOTH | S_IXOTH);

	mkdir_if_not_exists("dev", S_IRUSR | S_IWUSR | S_IXUSR |
				   S_IRGRP | S_IXGRP |
				   S_IROTH | S_IXOTH);

/*#ifdef CLONE_NEWCGROUP
	flags |= CLONE_NEWCGROUP;
#else
	printf("CLONE_NEWCGROUP unsupported.\n");
#endif

#ifdef CLONE_NEWUSER
	flags |= CLONE_NEWUSER;
#else
	printf("CLONE_NEWUSER unsupported.\n");
#endif
*/

#ifdef CLONE_NEWIPC
	flags |= CLONE_NEWIPC;
#else
	printf("CLONE_NEWIPC unsupported.\n");
#endif

#ifdef CLONE_NEWNS
	flags |= CLONE_NEWNS;
#else
	printf("CLONE_NEWNS unsupported.\n");
#endif

/*
#ifdef CLONE_NEWNET
	flags |= CLONE_NEWNET;
#else
	printf("CLONE_NEWNET unsupported.\n");
#endif
*/

#ifdef CLONE_NEWPID
	flags |= CLONE_NEWPID;
#else
	printf("CLONE_NEWPID unsupported.\n");
#endif

#ifdef CLONE_NEWUTS
	flags |= CLONE_NEWUTS;
#else
	printf("CLONE_NEWUTS unsupported.\n");
#endif

	// go to separate namespace
	res = unshare(flags);

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
	res = mount("sysfs", "/sys", "sysfs", MS_NODEV | MS_NOEXEC | MS_NOSUID, 0);
	if (res < 0) {
		perror("mount sys");
		return 0;
	}

	res = mount("proc", "/proc", "proc", MS_NODEV | MS_NOEXEC | MS_NOSUID, 0);
	if (res < 0) {
		perror("mount proc");
		return 0;
	}

	res = mount("tmpfs", "/tmp", "tmpfs", 0, "size=65536k");
	if (res < 0) {
		perror("mount tmp");
		return 0;
	}

	res = mount("tmpfs", "/dev", "tmpfs", 0, "size=65536k");
	if (res < 0) {
		perror("mount dev");
		return 0;
	}

	res = setresgid(999, 999, 999);
	if (res < 0) {
		perror("setgid to nobody");
	}

	res = setresuid(65534, 65534, 65534);
	if (res < 0) {
		perror("setuid to nobody");
	}

	res = execl("/bin/sh", "/bin/sh", NULL);
	if (res < 0) {
		perror("execl");
	}
}
