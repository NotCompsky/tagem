# Why has the Final Docker Departed from his Parents?

The final Dockerfile has to be in the project's root directory, otherwise the docker build system will not allow copying the necessary files from the host to the container

# Commands

	docker build -t notcompsky/tagem-base:latest tagem-base
	docker build -t notcompsky/tagem-compile-1:latest tagem-compile-1
	
	docker push notcompsky/tagem-base:latest
	docker push notcompsky/tagem-compile-1:latest
