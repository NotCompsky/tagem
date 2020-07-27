/*
Copyright 2020 Adam Gray
This file is part of the tagem program.
The tagem program is free software: you can redistribute it and/or
modify it under the terms of the GNU General Public License as published by the
Free Software Foundation version 3 of the License.
The tagem program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
This copyright notice should be included in any copy or substantial copy of the tagem source code.
The absense of this copyright notices on some other files in this project does not indicate that those files do not also fall under this license, unless they have a different license written at the top of the file.
*/
#pragma once

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <fcntl.h>


namespace proc {

inline
bool exec(int timeout,  const char* const* argv,  const int which_std_to_pipe,  char* const output_buf,  const size_t output_buf_sz){
	int status = 0;
	const char* which_err;
	int pipefd[2];
	
	pipe(pipefd);
	const pid_t pid = fork();
	if (unlikely(pid < 0)){
		fprintf(stderr, "Forking failed\n");
		return true;
	}
	if (pid == 0){
		// Redirect fork's stdout to pipefd
		close(pipefd[0]);
		dup2(pipefd[1], which_std_to_pipe);
		
		if (execvp(argv[0], (char**)argv)){
			fprintf(stderr, "Cannot execute: %s\n", argv[0]);
			exit(1);
		}
		
		exit(0);
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
	fprintf(stderr, "[%d] %s\nwhile executing: %s\n", status, which_err, argv[0]);
	fprintf(stderr, "with args\n");
	while(*(++argv) != nullptr){
		fprintf(stderr, "\t%s\n", *argv);
	}
	return true;
}

template<size_t output_buf_sz>
bool exec(int timeout,  const char* const* argv,  const int which_std_to_pipe,  char(&output_buf)[output_buf_sz]){
	return exec(timeout, argv, which_std_to_pipe, output_buf, output_buf_sz);
}

} // namespace proc
