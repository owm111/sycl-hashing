#line 7 "bench.nw"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cstdint>
#include <sycl/sycl.hpp>
#include <omp.h>
#line 372 "bench.nw"
#include "sha224.hpp"
#line 388 "bench.nw"
#include "sha224.cpp"
#line 393 "bench.nw"
#include "sha256.hpp"
#line 403 "bench.nw"
#include "sha256.cpp"
#line 408 "bench.nw"
#include "blake3.h"
#line 422 "bench.nw"
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
enum method {SERIAL, OPENMP, SYCL_CPU, SYCL_GPU};
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
static const char *methods[] = {"serial", "openmp", "sycl-cpu", "sycl-gpu"};
#line 166 "bench.nw"
#ifdef DUMP
static FILE *dump_stream;
#endif
#line 262 "bench.nw"
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
		
#line 377 "bench.nw"
{
	class SHA224 ctx;
	ctx.init();
	ctx.update((const unsigned char *)&input, sizeof(input));
	ctx.final(buf + slot * digest_size[algorithm]);
}
#line 222 "bench.nw"
		break;
	case SHA256:
		
#line 395 "bench.nw"
{
	class SHA256 ctx;
	ctx.init();
	ctx.update((const unsigned char *)&input, sizeof(input));
	ctx.final(buf + slot * digest_size[algorithm]);
}
#line 225 "bench.nw"
		break;
	case BLAKE3:
		
#line 412 "bench.nw"
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
#line 290 "bench.nw"
template<class Selector>
static std::vector<sycl::queue> make_queues(Selector sel, bool use_all)
{
	sycl::platform p(sel);
	std::vector<sycl::device> ds = p.get_devices();
	if (!use_all) {
		for (unsigned i = 1; i < ds.size(); i++) {
			ds.pop_back();
		}
	}
	std::vector<sycl::queue> result;
	for (sycl::device d : ds) {
		sycl::queue q(d);
		result.push_back(q);
	}
	return result;
}
#line 312 "bench.nw"
static std::vector<unsigned char *> alloc_buffers(std::vector<sycl::queue> qs,
		int buffer_size)
{
	std::vector<unsigned char *> result;
	for (sycl::queue q : qs) {
		unsigned char *b = sycl::malloc_device<unsigned char>(
				buffer_size, q);
		if (b == nullptr) {
			fprintf(stderr, "sycl::malloc_device failed when "
					"called %u bytes were requested.\n",
					buffer_size);
			exit(1);
		}
		result.push_back(b);
	}
	return result;
}
#line 334 "bench.nw"
template <class Selector>
static void run_sycl(Selector sel, unsigned char *host_buffer, bool use_all)
{
	std::vector<sycl::queue> qs = make_queues(sel, use_all);
	int buffer_size = (hashes_per_block * digest_size[algorithm])
			/ qs.size();
	int hashes_per_device = hashes_per_block / qs.size();
	std::vector<unsigned char *> buffers = alloc_buffers(qs, buffer_size);
#pragma omp parallel if(qs.size() > 1) num_threads(qs.size())
	{
		int t = omp_get_thread_num();
		enum algorithm alg = algorithm;
		unsigned char *host_ptr = host_buffer + buffer_size * t;
		for (uint64_t i = 0, base = 0; i < num_blocks;
					i++, base += hashes_per_device) {
			sycl::event hashes_ev = qs[t].parallel_for(sycl::range<1>(hashes_per_device), [=] (sycl::id<1> idx) {
				run_hash(alg, base + idx, buffers[t], idx);
			});
			sycl::event copy_ev = qs[t].memcpy(host_ptr, buffers[t], buffer_size, hashes_ev);
			copy_ev.wait();
			dump(host_ptr, hashes_per_device, digest_size[algorithm]);
		}
	}
	for (unsigned i = 0; i < buffers.size(); i++) {
		sycl::free(buffers[i], qs[i]);
	}
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
#line 257 "bench.nw"
unsigned char *output_buffer =
		new unsigned char[hashes_per_block * digest_size[algorithm]];
#line 237 "bench.nw"
switch (method) {
case SERIAL:
	
#line 266 "bench.nw"
for (uint64_t i = 0; i < num_blocks; i++) {
	for (uint64_t j = 0; j < hashes_per_block; j++) {
		run_hash(algorithm, i * hashes_per_block + j, output_buffer, j);
	}
	dump(output_buffer, hashes_per_block, digest_size[algorithm]);
}
#line 240 "bench.nw"
	break;
case SYCL_CPU:
	
#line 363 "bench.nw"
run_sycl(sycl::cpu_selector_v, output_buffer, true);
#line 243 "bench.nw"
	break;
case SYCL_GPU:
	
#line 365 "bench.nw"
run_sycl(sycl::gpu_selector_v, output_buffer, true);
#line 246 "bench.nw"
	break;
case OPENMP:
	
#line 277 "bench.nw"
#pragma omp parallel
for (uint64_t i = 0; i < num_blocks; i++) {
#pragma omp for
	for (uint64_t j = 0; j < hashes_per_block; j++) {
		run_hash(algorithm, i * hashes_per_block + j, output_buffer, j);
	}
	dump(output_buffer, hashes_per_block, digest_size[algorithm]);
}
#line 249 "bench.nw"
	break;
}
#line 260 "bench.nw"
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
