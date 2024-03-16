# Use an Ubuntu-based image as the base image
#FROM ubuntu:latest
FROM buildpack-deps:20.04

 # Run apt-get update to refresh package lists
 RUN apt-get update

 # Install MySQL client and GCC
 RUN apt-get install -y libhiredis-dev 

 # Clean up the package cache to reduce image size
 #RUN rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source code to container
COPY . /app

# Compile the C program
RUN gcc -o http_server 1server.c -lhiredis


ENV REDIS_HOST="redis"
ENV REDIS_PORT="6379"
 EXPOSE 8888

# Run the HTTP server when container starts
CMD ["./http_server"]
