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
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/syscall.h>


int main() {
	int pid = 0;
	// fork
	pid = fork();
	if (pid < 0) {
		perror("no fork?");
		return 0;
	} else if (pid) {
		int status;
		printf("I am parent for %d\n", pid);
		waitpid(pid, &status, 0);
		ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_TRACESYSGOOD);
		while (!WIFEXITED(status)) {
			struct user_regs_struct user_regs;
			ptrace(PTRACE_SYSCALL, pid, 0, 0);
			waitpid(pid, &status, 0);

			// at syscall
			if (WIFSTOPPED(status) && WSTOPSIG(status) & 0x80) {
				ptrace(PTRACE_GETREGS, pid, 0, &user_regs);
				if (user_regs.orig_rax == SYS_uname) {
					printf("Uname at %08llx\n", user_regs.rip);
					printf("r15: %08llx\n", user_regs.r15);
					printf("r14: %08llx\n", user_regs.r14);
					printf("r13: %08llx\n", user_regs.r13);
					printf("r12: %08llx\n", user_regs.r12);
					printf("rbp: %08llx\n", user_regs.rbp);
					printf("rbx: %08llx\n", user_regs.rbx);
					printf("r11: %08llx\n", user_regs.r11);
					printf("r10: %08llx\n", user_regs.r10);
					printf("r9: %08llx\n", user_regs.r9);
					printf("r8: %08llx\n", user_regs.r8);
					printf("rax: %08llx\n", user_regs.rax);
					printf("rcx: %08llx\n", user_regs.rcx);
					printf("rdx: %08llx\n", user_regs.rdx);
					printf("rsi: %08llx\n", user_regs.rsi);
					printf("rdi: %08llx\n", user_regs.rdi);
					printf("orig_rax: %08llx\n", user_regs.orig_rax);
					printf("rip: %08llx\n", user_regs.rip);
					printf("cs: %08llx\n", user_regs.cs);
					printf("eflags: %08llx\n", user_regs.eflags);
					printf("rsp: %08llx\n", user_regs.rsp);
					printf("ss: %08llx\n", user_regs.ss);
					printf("fs_base: %08llx\n", user_regs.fs_base);
					printf("gs_base: %08llx\n", user_regs.gs_base);
					printf("ds: %08llx\n", user_regs.ds);
					printf("es: %08llx\n", user_regs.es);
					printf("fs: %08llx\n", user_regs.fs);
					printf("gs: %08llx\n", user_regs.gs);
					ptrace(PTRACE_POKETEXT, pid, user_regs.rsi, 0);
					// skip after syscall
					ptrace(PTRACE_SYSCALL, pid, 0, 0);
					ptrace(PTRACE_POKETEXT, pid, user_regs.rsi, 0);
					waitpid(pid, &status, 0);
				} else {
					printf("SYSCALL %lld at %08llx\n", user_regs.orig_rax, user_regs.rip);
					// skip after syscall
					ptrace(PTRACE_SYSCALL, pid, 0, 0);
					waitpid(pid, &status, 0);
				}
			}
  		}
		wait(NULL);
		return 0;
	} else {
		int res = 0;
		ptrace(PTRACE_TRACEME, 0, 0, 0);
		res = execl("./uname", "./uname", NULL);
		if (res < 0) {
			perror("execl");
		}
		return 0;
	}
}
