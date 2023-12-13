#line 7 "bench.nw"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cstdint>
#include <sycl/sycl.hpp>
#include <omp.h>
#line 329 "bench.nw"
#include "sha224.hpp"
#line 345 "bench.nw"
#include "sha224.cpp"
#line 350 "bench.nw"
#include "sha256.hpp"
#line 360 "bench.nw"
#include "sha256.cpp"
#line 365 "bench.nw"
#include "blake3.h"
#line 379 "bench.nw"
#include "blake3.cpp"
#include "blake3_dispatch.cpp"
#include "blake3_portable.cpp"
#line 37 "bench.nw"
#define OUTPUT_TEMPLATE "hashes_per_block =\t%u\t" \
	"num_blocks =\t%u\t" \
	"algorithm =\t%s\t" \
	"runner =\t%s\t" \
	"elapsed (s) =\t%f\n"
#line 92 "bench.nw"
#define LENGTH(arr) (sizeof(arr) / sizeof((arr)[0]))
#line 32 "bench.nw"
enum algorithm {SHA224, SHA256, BLAKE3};
enum method {SERIAL, SYCL_CPU, SYCL_GPU};
#line 80 "bench.nw"
static unsigned parse_unsigned(const char **argv, int arg_num);
static int parse_enumerator(const char **argv, int arg_num,
		const char **enumerators, const unsigned num_enumerators);
#line 208 "bench.nw"
static void run_hash(enum algorithm algorithm, uint64_t input,
		unsigned char *buf, unsigned slot);
#line 57 "bench.nw"
static unsigned hashes_per_block;
static unsigned num_blocks;
static enum algorithm algorithm;
static enum method method;
#line 88 "bench.nw"
static const char *algorithms[] = {"sha224", "sha256", "blake3"};
static const char *methods[] = {"serial", "sycl-cpu", "sycl-gpu"};
#line 166 "bench.nw"
#ifdef DUMP
static FILE *dump_stream;
#endif
#line 259 "bench.nw"
static const unsigned digest_size[] = {28u, 32u, 32u};
#line 106 "bench.nw"
static unsigned parse_unsigned(const char **argv, int arg_num)
{
	int rc;
	unsigned result;
	rc = sscanf(argv[arg_num], "%u", &result);
	if (rc == EOF) {
		fprintf(stderr, "%s: could not parse “%s” as an unsigned "
			"integer\n", argv[0], argv[arg_num]);
		exit(1);
	}
	return result;
}
#line 124 "bench.nw"
static int parse_enumerator(const char **argv, int arg_num,
		const char **enumerators, const unsigned num_enumerators)
{
	unsigned i;
	for (i = 0; i < num_enumerators; i++)
		if (strcmp(argv[arg_num], enumerators[i]) == 0)
			return i;
	fprintf(stderr, "%s: could not match “%s” to ",
			argv[0], argv[arg_num]);
	for (i = 0; i < num_enumerators - 1; i++)
		fprintf(stderr, "“%s”, ", enumerators[i]);
	fprintf(stderr, "or “%s”.\n", enumerators[i]);
	exit(1);
	return 0;
}
#line 181 "bench.nw"
#ifdef DUMP
static void dump(unsigned char *buffer, size_t num_hashes, size_t hash_size)
{
	
#line 192 "bench.nw"
unsigned i, j;
for (i = 0; i < num_hashes; i++) {
	for (j = 0; j < hash_size; j++) {
		fprintf(dump_stream, "%02x", buffer[i * hash_size + j]);
	}
	fprintf(dump_stream, "\n");
}
#line 185 "bench.nw"
}
#else
#define dump(x, y, z) /* dump */
#endif
#line 216 "bench.nw"
static void run_hash(enum algorithm algorithm, uint64_t input,
		unsigned char *buf, unsigned slot)
{
	switch (algorithm) {
	case SHA224:
		
#line 334 "bench.nw"
{
	class SHA224 ctx;
	ctx.init();
	ctx.update((const unsigned char *)&input, sizeof(input));
	ctx.final(buf + slot * digest_size[algorithm]);
}
#line 222 "bench.nw"
		break;
	case SHA256:
		
#line 352 "bench.nw"
{
	class SHA256 ctx;
	ctx.init();
	ctx.update((const unsigned char *)&input, sizeof(input));
	ctx.final(buf + slot * digest_size[algorithm]);
}
#line 225 "bench.nw"
		break;
	case BLAKE3:
		
#line 369 "bench.nw"
{
	blake3_hasher hasher;
	blake3_hasher_init(&hasher);
	blake3_hasher_update(&hasher, &input, sizeof(input));
	blake3_hasher_finalize(&hasher,
			buf + slot * digest_size[algorithm],
			digest_size[algorithm]);
}
#line 228 "bench.nw"
		break;
	}
}
#line 273 "bench.nw"
template <class Selector>
static void run_sycl(Selector sel, unsigned char *host_buffer)
{
	
#line 287 "bench.nw"
using sycl::event;
using sycl::id;
using sycl::malloc_device;
using sycl::queue;
using sycl::range;
#line 277 "bench.nw"
	queue q(sel);
	
#line 296 "bench.nw"
unsigned buffer_size = hashes_per_block * digest_size[algorithm];
unsigned char *sycl_buffer = malloc_device<unsigned char>(buffer_size, q);
#line 305 "bench.nw"
if (sycl_buffer == nullptr) {
	fprintf(stderr, "sycl::malloc_device failed when called %u bytes were "
			"requested.\n", buffer_size);
	exit(1);
}
#line 299 "bench.nw"
for (uint64_t i = 0, base = 0; i < num_blocks; i++, base += hashes_per_block) {
	
#line 316 "bench.nw"
enum algorithm alg = algorithm;
event hashes_ev = q.parallel_for(range<1>(hashes_per_block), [=] (id<1> idx) {
	run_hash(alg, base + idx, sycl_buffer, idx);
});
event copy_ev = q.memcpy(host_buffer, sycl_buffer, buffer_size, hashes_ev);
copy_ev.wait();
dump(host_buffer, hashes_per_block, digest_size[algorithm]);
#line 301 "bench.nw"
}
#line 279 "bench.nw"
}
#line 20 "bench.nw"
int main(int argc, const char **argv)
{
	
#line 66 "bench.nw"
if (argc != 5) {
	fprintf(stderr, "%s: incorrect number of arguments.\n", argv[0]);
	return 1;
}
#line 95 "bench.nw"
hashes_per_block = parse_unsigned(argv, 1);
num_blocks = parse_unsigned(argv, 2);
algorithm = (enum algorithm)
	parse_enumerator(argv, 3, algorithms, LENGTH(algorithms));
method = (enum method)parse_enumerator(argv, 4, methods, LENGTH(algorithms));
#line 23 "bench.nw"
	
#line 170 "bench.nw"
#ifdef DUMP
dump_stream = fopen("bench-hashes.dat", "w");
#endif
#line 24 "bench.nw"
	
#line 145 "bench.nw"
double elapsed;
struct timespec start, end;
clock_gettime(CLOCK_MONOTONIC, &start);
#line 254 "bench.nw"
unsigned char *output_buffer =
		new unsigned char[hashes_per_block * digest_size[algorithm]];
#line 237 "bench.nw"
switch (method) {
case SERIAL:
	
#line 263 "bench.nw"
for (uint64_t i = 0; i < num_blocks; i++) {
	for (uint64_t j = 0; j < hashes_per_block; j++) {
		run_hash(algorithm, i * hashes_per_block + j, output_buffer, j);
	}
	dump(output_buffer, hashes_per_block, digest_size[algorithm]);
}
#line 240 "bench.nw"
	break;
case SYCL_CPU:
	
#line 281 "bench.nw"
run_sycl(sycl::cpu_selector_v, output_buffer);
#line 243 "bench.nw"
	break;
case SYCL_GPU:
	
#line 283 "bench.nw"
run_sycl(sycl::gpu_selector_v, output_buffer);
#line 246 "bench.nw"
	break;
}
#line 257 "bench.nw"
delete[] output_buffer;
#line 149 "bench.nw"
clock_gettime(CLOCK_MONOTONIC, &end);
elapsed = (double)end.tv_sec - (double)start.tv_sec;
elapsed += ((double)end.tv_nsec - (double)start.tv_nsec) / 1e12L;
#line 25 "bench.nw"
	
#line 157 "bench.nw"
printf(OUTPUT_TEMPLATE, hashes_per_block, num_blocks,
		algorithms[algorithm], methods[method], elapsed);
#line 26 "bench.nw"
	
#line 174 "bench.nw"
#ifdef DUMP
fclose(dump_stream);
#endif
#line 27 "bench.nw"
	return 0;
}
