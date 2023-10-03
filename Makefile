literate = header.nw bench.nw
sources = bench.cpp
illiterate_sources = \
	sha224.cpp sha256.cpp \
	blake3.cpp blake3_dispatch.cpp blake3_portable.cpp
scripts = run-bench.sh
programs = bench
documents = doc.ps doc.pdf
junk = doc.ms *.nwt

CXX = syclcc
SYCLFLAGS = --hipsycl-targets=omp
CPPFLAGS = -D_XOPEN_SOURCE=700
CXXFLAGS = -std=c++17 -g -Wall -Wextra -Wpedantic $(SYCLFLAGS)

# These are needed to make blake3 compile without needing 4+ additional files
CPPFLAGS += \
	-DBLAKE3_NO_SSE2 -DBLAKE3_NO_SSE41 -DBLAKE3_NO_AVX2 -DBLAKE3_NO_AVX512

-include config.mk

.PHONY: all clean deepclean

all: $(documents) $(programs) $(scripts)

clean:
	$(RM) $(junk) $(programs)

deepclean: clean
	$(RM) $(sources) $(scripts) $(documents)

doc.ms: $(literate)
	noweave -filter btdefn -troff $^ >$@

doc.ps: doc.ms
	noroff -Kutf8 -Tps -e -ms $^ >/dev/null
	noroff -Kutf8 -Tps -e -ms $^ >$@

doc.pdf: doc.ps
	ps2pdf $< $@

bench: $(sources) # $(illiterate_sources)

$(sources) $(scripts): $(literate)
	for x in $(sources); do \
		notangle -filter btdefn -L -R$$x $^ | cpif $$x; \
		done
	for x in $(scripts); do \
		notangle -filter btdefn -R$$x $^ | cpif $$x; \
		chmod +x $$x; \
		done
