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

bench: $(sources) # $(illiterate_sources)

$(sources) $(scripts): $(literate)
	for x in $(sources); do \
		notangle -filter btdefn -L -R$$x $^ | cpif $$x; \
		done
	for x in $(scripts); do \
		notangle -filter btdefn -R$$x $^ | cpif $$x; \
		chmod +x $$x; \
		done
