# Simple C Web Server

A lightweight web server written in C, built from scratch to understand how networking and HTTP works at a low level.

## Roadmap / To-Do List

- **Basic Server**

   [x] Create a simple TCP server that listens on a port.
   
   [x] Respond with a fixed static response (e.g., "Hello, World!").

- **Serve Static Files & Logging**

   [x] Add ability to serve static files (HTML, CSS, images).
   
   [X] Implement a basic logging system to track requests and responses.
     
   * add basic configuration file support.

- **Proxy Features**

   [x] Add reverse proxy capabilities to forward requests to other applications (Node.js, Python, Java, etc.).

- **Large requests handling**
  
   * fix buffer overflow issues (send/recive big files).

- **Multi-Threading Support**

   * Enable the server to handle multiple clients concurrently using threads or worker processes.

- **Security & HTTPS**

   * Add basic security checks and input validation.
   * Implement TLS/SSL support for HTTPS.


- **Dynamic Content (PHP Support)**

   * Integrate with a PHP interpreter to handle dynamic web pages.
   * Support executing PHP scripts and returning the output.


## Future Enhancements

* Add caching.
* Add configuration files.
* Support HTTP/2.
