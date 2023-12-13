literate = bench.nw
sources = bench.cpp
illiterate_sources = \
	sha224.cpp sha256.cpp \
	blake3.cpp blake3_dispatch.cpp blake3_portable.cpp
scripts = run-bench.sh to-grap.sh check-dumps.sh generate-inputs.sh
programs = bench
pictures = bench-results-pc.d bench-results-eightsocket.d
documents = source.pdf source.html report.ps report.pdf
junk = source.tex *.aux *.log

CXX = syclcc
SYCLFLAGS = --hipsycl-targets=omp
CPPFLAGS = -D_XOPEN_SOURCE=700
CXXFLAGS = -std=c++17 -g -Wall -Wextra -Wpedantic -fopenmp $(SYCLFLAGS)
LDFLAGS = -fopenmp $(SYCLFLAGS)
LDLIBS = -lstdc++

-include config.mk

.PHONY: all clean deepclean

all: $(documents) $(programs) $(scripts)

clean:
	$(RM) $(junk) $(programs)

deepclean: clean
	$(RM) $(sources) $(scripts) $(documents)

source.tex: $(literate)
	noweave -filter btdefn -index -latex $^ >$@

source.html: $(literate)
	noweave -filter btdefn -filter l2h -index -html $^ >$@

%.pdf: %.tex
	pdflatex $<
	pdflatex $<

report.ps: report.ms $(pictures)
	groff -G -Kutf8 -Tps -e -ms -p $< >$@

%.pdf: %.ps
	ps2pdf $< $@

%: %.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

bench: $(sources:cpp=o) blake3_avx512.o blake3_avx2.o blake3_sse41.o blake3_sse2.o

blake3_avx512.o: CFLAGS += -mavx512vl -mavx512f
blake3_avx2.o: CFLAGS += -mavx2
blake3_sse41.o: CFLAGS += -msse4
blake3_sse2.o: CFLAGS += -msse2

$(sources) $(scripts): $(literate)
	for x in $(sources); do \
		notangle -filter btdefn -L -R$$x $^ | cpif $$x; \
		done
	for x in $(scripts); do \
		notangle -filter btdefn -R$$x $^ | cpif $$x; \
		chmod +x $$x; \
		done
