#pragma once

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <fcntl.h>


namespace proc {

template<size_t output_buf_sz>
bool exec(int timeout,  const char* const* argv,  char(&output_buf)[output_buf_sz]){
	int status;
	const char* which_err;
	int pipefd[2];
	
	pipe(pipefd);
	const pid_t pid = fork();
	if (pid == 0){
		// Redirect fork's stdout to pipefd
		close(pipefd[0]);
		dup2(pipefd[1], STDOUT_FILENO);
		
		if (execvp(argv[0], (char**)argv)){
			which_err = "Cannot";
			goto print_args_and_return;
		}
	}
	close(pipefd[1]);
	fcntl(pipefd[0], F_SETFL, fcntl(pipefd[0], F_GETFL) | O_NONBLOCK);
	
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
	
	read(pipefd[0], output_buf, output_buf_sz);
	
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
