# COMP30023 Project 1
#### By Nico Dinata
Implementation of a simple HTTP/1.0 server that serves files to GET requests. The response statuses that it handles are "200 OK" and "404 Not Found".

## Dependencies
- gcc
- make
- curl/wget

## How to test
1. Clone this repo onto your own machine and `cd` into it.
2. `make`
3. `./server port_no path_to_root` where `port_no` is any unregistered port number on your machine, and `path_to_root` is the path to the root directory of your server, e.g. `./www/`
4. Choose any of the methods below:
    - Open up your browser and head to `localhost:port_no/any_file`
    - `curl -v localhost:port_no/any_file`
    - `wget localhost:port_no/any_file`
5. When you're done, stop the server with Ctrl+C (SIGINT).
6. `make clobber`
