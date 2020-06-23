#pragma once

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>


namespace proc {
bool exec(int timeout,  const char* const* argv){
	int status;
	const char* which_err;
	
	const pid_t pid = fork();
	if (pid == 0){
		if (execvp(argv[0], (char**)argv)){
			which_err = "Cannot";
			goto print_args_and_return;
		}
	}
	
	while (not waitpid(pid , &status, WNOHANG)){
		if (--timeout == 0){
			which_err = "Timeout";
			goto print_args_and_return;
		}
		sleep(1);
	}

	if ((WEXITSTATUS(status) != 0) or not WIFEXITED(status)){
		which_err = "Failure";
		goto print_args_and_return;
	}
	
	return false;
	
	print_args_and_return:
	fprintf(stderr, "%s\nwhile executing: %s\n", which_err, argv[0]);
	fprintf(stderr, "with args\n");
	while(*(++argv) != nullptr){
		fprintf(stderr, "\t%s\n", *argv);
	}
	return true;
}
} // namespace proc
