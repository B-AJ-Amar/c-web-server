# Simple C Web Server

A lightweight web server written in C, built from scratch to better understand networking and the HTTP protocol at a low level.

## Quick Start

### Prerequisites

- GCC compiler
- Make
- Linux/Unix environment

### Installation & Usage

1. **Clone the repository:**

   ```bash
   git clone <repository-url>
   cd c-web-server
   ```

2. **Build and run in one command:**

   ```bash
   make cbr
   ```
   
   This single command will:
   - Clean previous builds (`clean`)
   - Compile the project (`build`)
   - Run the web server (`run`)

3. **Alternative commands:**

   ```bash
   # Build only
   make

   # Clean build files
   make clean

   # Run the server (after building)
   make run

   # Format code
   make format
   ```

4. **Configuration:**

   - The server reads configuration from `cws.conf`
   - Static files are served from the `public/` directory
   - Default port and other settings can be configured in the config file

5. **Access the server:**

   - Open your browser and navigate to `http://localhost:PORT` (check `cws.conf` for the configured port)


## Roadmap / To-Do List

- **Basic Server**
  - [x] Create a simple TCP server that listens on a port.
  - [x] Respond with a fixed static response (e.g., "Hello, World!").

- **Serve Static Files & Logging**
  - [x] Add ability to serve static files (HTML, CSS, images).
  - [ ] Add autoindexing for directories without an index file. (low priority)
  - [x] Implement a basic logging system to track requests and responses.
  - [x] Add basic configuration file support.

- **Proxy Features**
  - [x] Add reverse proxy capabilities to forward requests to other applications (Node.js, Python, Java, etc.).

- **Large Request Handling**
  - [x] Fix buffer overflow issues when sending large files.
  - [ ] Handle receiving large requests (buffer overflow). (i will do this after multi-threading to avoid conflicts)

- **Multi-Threading Support**
  - [ ] Enable the server to handle multiple clients concurrently using thread pool.

- **Security & HTTPS**
  - [ ] Add basic security checks and input validation.
  - [ ] Implement TLS/SSL support for HTTPS.

- **Dynamic Content (PHP Support)**
  - [ ] Integrate with a PHP interpreter to handle dynamic web pages.
  - [ ] Support executing PHP scripts and returning the output.

### Future Enhancements

- [ ] Inhance the multi threading system (add event loops)
- [ ] Add caching support.
- [ ] Support HTTP/2 & HTTP/3.
- [ ] Add WebSocket support.


## Configuration

The server uses a TOML configuration file (`cws.conf`) to define its behavior. Here's a complete breakdown of all configuration options:

### Server Configuration

```toml
[server]
host = "0.0.0.0"              # IP address to bind to (0.0.0.0 for all interfaces)
port = 8080                   # Port to listen on
max_connections = 100         # Maximum number of concurrent connections
sock_buffer_size = 4096       # Socket buffer size in bytes
workers = 4                   # Number of worker threads (default: 1)
default_index_name = "index.html"  # Default file to serve for directories
```

### Logging Configuration

```toml
[logging]
level = "DEBUG"               # Log level: DEBUG, INFO, WARN, ERROR
use_colors = true             # Enable colored output in logs
time_format = "%Y-%m-%d %H:%M:%S"  # Timestamp format (strftime format)
output = "stdout"             # Log output: "stdout", "stderr", or file path
```

### Route Configuration

Routes define how different URL paths are handled. You can define multiple routes using the `[[routes]]` syntax:

```toml
# Static file serving
[[routes]]
path = "/"                    # URL path to match
root = "./public"             # Directory to serve files from
index = "index.html"          # Default file for directories
autoindex = false             # Enable/disable directory listing
methods = ["GET"]             # Allowed HTTP methods (optional)

# Reverse proxy
[[routes]]
path = "/api"                 # URL path to proxy
proxy_pass = "http://localhost:3000"  # Backend server URL
methods = ["GET", "POST"]     # Allowed HTTP methods

# Static files with directory listing
[[routes]]
path = "/static/"
root = "./static"
index = "index.html"
autoindex = true              # Shows directory contents if no index file
methods = ["GET"]
```

### Route Types

1. **Static File Routes**: Serve files from a local directory
   - Requires: `path`, `root`
   - Optional: `index`, `autoindex`, `methods`

2. **Proxy Routes**: Forward requests to another server
   - Requires: `path`, `proxy_pass`
   - Optional: `methods`

### Configuration Examples

**Basic static server:**

```toml
[server]
port = 8080

[logging]
level = "INFO"

[[routes]]
path = "/"
root = "./public"
```

**With proxy for API:**

```toml
[server]
port = 8080

[[routes]]
path = "/"
root = "./public"

[[routes]]
path = "/api"
proxy_pass = "http://localhost:3000"
methods = ["GET", "POST", "PUT", "DELETE"]
```

## Contributing

All contributions are welcome! Whether you're fixing bugs, adding new features, improving documentation, or suggesting enhancements, your help is appreciated.

