# COMP30023 Project 1
#### By Nico Dinata
Implementation of a simple multi-threaded HTTP/1.0 server that serves files to GET requests. The response statuses that it handles are `200 OK` and `404 Not Found`.

## Dependencies
- gcc
- make
- wget

## How to test
### Running test script
A test script is provided, which would run the server and request for the files in the `test` directory. To run the test script:
1. Clone this repo and `cd` into it.
2. Compile the server with `make`.
3. Run `./test_script.sh server any_port_no` where `any_port_no` is any unregistered port number on your machine.
4. Clean up with `make clobber`.

### Running the server
1. Follow steps 1 and 2 from `Running test script` above.
2. Run `./server port_no path_to_root` where `port_no` is any unregistered port number on your machine, and `path_to_root` is the path to the root directory of your server, e.g. `./www/`
3. Check that the server works properly by requesting files (with `wget` or your browser).
4. When you're done, stop the server with Ctrl+C.
5. Clean up with `make clobber`.
