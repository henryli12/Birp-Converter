# Birp-Converter
A tool that convert image data between the PGM ("Portable Gray Map") format and a non-standard birp format that represents 
the pixel rasters using a variant of a data structure called binary decision diagrams (BDDs).

## Compiling

In the root directory, executing `make` or `make all` will compile
anything that needs to be, `make debug` does the same except that it compiles the code
with options suitable for debugging, and `make clean` removes files that resulted from
a previous compilation.

## Running Program

Executing `bin/birp` will run the program.

Here is a detailed usage of `bin/birp`:
<pre>
USAGE: bin/birp [-h] [-i FORMAT] [-o FORMAT] [-n|-r|-t THRESHOLD|-z FACTOR|-Z FACTOR]
   -h       Help: displays this help menu.
   -i       Input format: `pgm` or `birp` (default `birp`)
   -o       Output format: `pgm`, `birp`, or `ascii` (default `birp`)

In all cases, the program reads image data from the standard input and writes image
data to the standard output.  If the input and output formats are both `birp`,
then one of the following transformations may be specified (the default is an
identity transformation; *i.e.* the image is passed unchanged):
   -n       Complement each pixel value
   -r       Rotate the image 90-degrees counterclockwise
   -t       Apply a threshold filter (with THRESHOLD in [0, 255]) to the image
   -z       Zoom out (by FACTOR in [0, 16]), producing a smaller raster
   -Z       Zoom in, (by FACTOR in [0, 16]), producing a larger raster
</pre>

In any of its operating modes, `bin/birp` reads from `stdin` and writes to `stdout`.

Instead of directly inputing the file to the terminal or displaying output in the terminal, using **input and output redirection** would be helpful.
Here is a simple example:
```
bin/birp -i birp -o pgm < rsrc/M.birp > output/M.pmg
```

In addition, using the pipeline is helpful to combine mulitple commands:
```
bin/birp -z 2 < rsrc/M.birp | bin/birp -n | bin/birp t 100 > output/M.birp
```
## Sample Runs
I will always convert the file to ascii for the ease of visualization

**Convert Birp to ascii:**
```
$ bin/birp -o ascii < rsrc/M.birp
@@@@@@@@@@@@@@@@
@    @@@@@@    @
@@    @@@@    @@
@@    @@@@    @@
@@  @  @@  @  @@
@@  @  @@  @  @@
@@  @  @@  @  @@
@@  @@    @@  @@
@@  @@@  @@@  @@
@@  @@@  @@@  @@
@@  @@@@@@@@  @@
@@  @@@@@@@@  @@
@@  @@@@@@@@  @@
@     @@@@     @
@     @@@@     @
@@@@@@@@@@@@@@@@
```
**Zoom out by factor of 1:**
```
$ bin/birp -z 1 < rsrc/M.birp | bin/birp -o ascii
@@@@@@@@
@  @@  @
@ @@@@ @
@ @@@@ @
@ @@@@ @
@ @@@@ @
@ @@@@ @
@@@@@@@@
```
**Convert from PGM to Birp to ascii**
```
$ bin/birp -i pgm < rsrc/checker.pgm | bin/birp -o ascii
@@@@ @ @
@@@@@ @ 
@@@@ @ @
@@@@@ @ 
  @@    
  @@    
@@      
@@
```

## Unit Testing

I have used a C unit testing framework called [Criterion](https://github.com/Snaipe/Criterion).
The tests are located in the `tests` directory. After compiling, you can run the tests by executing `bin/birp_tests`. 
