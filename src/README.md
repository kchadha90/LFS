# Log-structured File System on FUSE

Setup environment on Ubuntu 12.04:  

	sudo apt-get update
	sudo apt-get dist-upgrade
	sudo apt-get install build-essential libfuse-dev libprotobuf-dev protobuf-compiler

Build with `make`.  
Run unit tests with `make runtest`.

## Programs

* mklfs: create a filesystem
* lfs: mount a filesystem
* lfsck: verify the integrity of a filesystem


