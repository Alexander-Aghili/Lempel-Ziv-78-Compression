# Lempel-Ziv 78 Compression Algorithm
## Short Description
Lempel-Ziv Compression is a core componeent of modern compression algorithms used throughout the internet. Without it, we would struggle to store, send, or share most of the information we do today. This program contains two executables, encode and decode, with can compress and decompress a file, respectively. Only a file encoded by this specific algorithm can be decoded using the executable because of a static magic number written into the file to verify it will use the same algorithm to decode the executable. While this algorithm can be used for all data types, it is best suited for plain text since the encoding algorithm is optimized for repeated symbols.

## How to Build
To build, you must have the Makefile. This operates by collecting the C files, generating the object files, and linking them into the binary executable. You must have all of the .c and .h files from this repository to build. Once you have all the appropriate files, you can build each executable independantly or all together at once. To build all the files:
```
make
```
or 
```
make all
```
To make each executable independantly:
```
make encode
make decode
```
To see the command line arguments for each executable, run the following commands or see below.
```
./encode -h
./decode -h 
```

## Encode Command Line Arguments
- -i *input_file*: Compresses contents from *input_file* (default: stdin)
- -o *output_file*: Compressed data is placed into *output_file* (default: stdout)
- -v: Enables verbose program output
- -h: Prints help usage

## Decode Command Line Arguments
- -i *input_file*: Decompresses contents from compressed file *input_file* (default: stdin)
- -o *output_file*: Decompressed data (original message) is placed into *output_file* (default: stdout)
- -v: Enables verbose program output
- -h: Prints help usage


## To Run
The following is an example of how to encode a message in *input.txt* and output that encoded message to *encoded.txt*. It will then decode that encoded message into *output.txt*. Other inputs will be default.

First, encide the message in *input.txt*:
```
./encode -i input.txt -o encrypted_message.txt
```
Now an encoded message is located in *encoded.txt*.

Finally, decode that encoded message and place the output into *output.txt*:
```
./decode -i encode.txt -o output.txt
```
Now the message in *input.txt* and *output.txt* are the same. 
