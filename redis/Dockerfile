FROM alpine

# Install Redis and required dependencies
RUN apk add --no-cache redis

# Copy custom redis.conf file to container
COPY redis.conf /etc/redis.conf

# Update Redis configuration to disable protected mode
RUN sed -i 's/protected-mode yes/protected-mode no/' /etc/redis.conf

# Expose Redis port
EXPOSE 6379

# Start Redis server with custom configuration
CMD ["redis-server", "/etc/redis.conf"]
