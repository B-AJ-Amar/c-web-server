# Simple C Web Server

A lightweight web server written in C, built from scratch to better understand networking and the HTTP protocol at a low level.

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

## Future Enhancements

- [ ] Inhance the multi threading system (add event loops)
- [ ] Add caching support.
- [ ] Support HTTP/2 & HTTP/3.
- [ ] Add WebSocket support.
