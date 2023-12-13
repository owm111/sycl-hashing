This repository includes a program to benchmark for hash generation and
a report of the results of a handful of machines and configurations.

The benchmark program will generate a specified number of hashes in
a buffer (one "block"). Nonces for the hashes are a 64-bit integer
corresponding to the position in the block. No other information is stored
with the hash. Once the block is full, the program will either discard
the hashes or print them to a file, depending on the configuration. The
program will repreat this for a set number of blocks. See usage.

The report, built to report.ps and report.pdf, contains the results of
running ./bench on a few different machines with different CPUs and GPUs.

The source code documentation can be found in source.pdf.

Known Issues
------------

BLAKE3 does not work with Intel DPC++ (error below), but it does work
with AdaptiveCpp (no error). I needed to deleted the BLAKE3 case to make
it compile.

	./blake3.cpp:341:20: error: SYCL kernel cannot call a recursive function
	<...>
	./blake3.cpp:264:1: note: function implemented using recursion declared here

Source Files & Structure
------------------------

The following list describes the list of files that are included in
the repository, and descriptions of their purpose.

- Makefile: Describes the build process for Make.
- flake.nix, flake.lock: A Nix flake that includes dependencies and a
reproducible shell environment.

- *.nw: Literate source code for the program, script, and documentation.

- sha2*, blake3*: Source code and headers for the SHA-2 and BLAKE3
hash functions. These were taken from implmentations found online,
and modified to work with SYCL.

- *.d: Graphs for the report in grap(1) format.

As mentioned, this project uses [Nix][1] for dependency management
and development environments.  Nix was chosen because it is able to pin
package versions and build the same environment across operating systems.
No changes should be required to build and run the program when using
the Nix shell.

[1]: https://nixos.org

Also mentioned above, this project is written in a literate style,
specifically using [Noweb.][2] Literate programming allows source code
to be included in the documentation and freely arranged in whatever
order makes it easiest to understand.  Here, C++ source code, Bourne
shell scripts, and troff documentation are all written in bench.nw.
The notangle command extracts the C++ code and shell script, and the
noweave command formats the C++ and script code with troff macros so
it can be built like any other troff document; see the makefile for
more details.

[2]: https://www.cs.tufts.edu/\~nr/noweb

Dependencies
------------

With Nix, use the development shell.

	nix develop

Otherwise, the following programs are needed to compile bench:
(pre-generated source files are included so noweb is not necessary to
get it up-and-running)

- Noweb
- A SYCL-aware C++ compiler: Intel DPC++, AdaptiveCpp, etc.

The following programs are needed to the compile the report:

- GNU roff
- A version of grap(1), such as the one from plan9port
- Ghostscript
- TeXLive

Compiling and Configuration
---------------------------

Everything in this project is built with Make. The makefile is designed
to work out-of-the-box with the included Nix development shell. So,
the following should build a working program and the report:

	nix develop
	make

To compile just the benchmark and necessary scripts:

	make bench run-bench.sh check-dumps.sh generate-inputs.sh

Compilation options can be customized with a file called config.mk.
It is loaded by the makefile after the compilation variables are set, so
config.mk can modify them. Below is an example config.mk that configures
AdapativeCpp (f/k/a hipSYCL) to compile an optimized binary that works
on the CPU (its backend is OpenMP) and for an AMD Radeon RX 7000-series
graphics card (its backend is HIP).

Information about what devices are available and their architectures
can be found in the output of the hipsycl-info command.

The -fno-stack-protector flag is necessary to compile for the GPU.

	CXXFLAGS += -march=native -O3 -fno-stack-protector
	SYCLFLAGS += '--hipsycl-targets=omp;hip:gfx1100' \
			--hipsycl-use-accelerated-cpu

For Intel oneAPI, use the following config.mk:

	CXX = icpx
	CXXFLAGS += -march=native -O3 -fno-stack-protector
	CPPFLAGS += -I/opt/intel/oneapi/compiler/latest/linux/include/sycl
	SYCLFLAGS = -fsycl

If the preprocessor macro DUMP is defined, then the program will dump
the hashes that it generates to disk in a file called bench-hashes.dat.
Add the following line to config.mk to enable this.

	CPPFLAGS += -DDUMP

Bench Usage
-----------

The benchmark program requires four arguments:

1. The number of hashes in each block. 
2. The number of blocks.
3. The hash algorithm to use.
4. The method to generate the hashes.

The program does not read the standard input or any files.

The following hash algorithms are recognized:

- `sha256`
- `sha224`
- `blake3` (256-bit hashes)

The following generation methods are recognized:

- `serial`
- `sycl-cpu`
- `sycl-gpu`

When the program has finished it will output one line containing the
following tab-separated fields:

1. The string `hashes_per_block =`.
2. The first argument.
3. The string `num_blocks =`.
4. The second argument.
5. The string `algorithm =`.
6. The third argument.
7. The string `runner =`.
8. The fourth argument.
9. The string `elapsed (s) =`
10. The real-time seconds that passed during the benchmark.

Example: time how long it takes to generate 100 blocks containing 1024
BLAKE3 hashes each on the CPU with SYCL.

	./bench 1024 100 blake3 sycl-cpu

Support Scripts
---------------

Run-bench.sh reads lines from standard input that contain the first two
arguments two bench, and runs bench with those arguments and `blake3`,
`sha256`, `sycl-cpu`, and `sycl-gpu`.

Use with generate-inputs.sh

Generate-inputs.sh takes two arguments:

1. The total amount of hashes to generate in bytes, expressed as a power
of two (e.g., `34` is interpreted as 2<sup>34</sup>B or 16GiB).
2. The maximum block size in bytes, expressed as a power of two (e.g.,
`31` is interpreted as 2<sup>31</sup> or 2GiB).

It prints input lines for run-bench.sh that will generate the specified
amount of hashes in varying block sizes, starting from 1024 hashes/block
to the specified limit.

Example: run bench with varying block sizes to generate 16GiB of hashes.

	./generate-inputs 34 31 | run-bench.sh | tee bench-results

To-grap.sh is a filter to turn bench-results (see above) into input for
grap. It doesn't work very well.

When compiled with dumping enabled, check-dumps.sh can be used to verify
that all runners are generating the same hashes.

Example:

	./check-dumps.sh
