# CS539 - Database System
B+Tree Implementation in C++.
This project is complete implementation of a B+ Tree.
All fundemental operations of a B+ tree including Insertion, Removeing, Searching are implemented.
Also there is a RangeScan operation that will for records that are within a specific range of ids.
There is also a test Record structure with two sample fields.

## Build

run the following commands to build the system:

```
$ mkdir build
$ cd build
$ cmake ..
$ make
```

## Testing

```
$ cd build
$ make test
$ ./test
```