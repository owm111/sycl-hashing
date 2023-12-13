.TL
Benchmark Results
.EQ
delim $$
.EN
.SH
Personal Computer
.IP \(bu 3n
AMD R7 3700X
.IP \(bu
32GB DDR4-3200 RAM
.IP \(bu
Radeon RX 7900 XTX
.IP \(bu
NixOS 23.11 Unstable
.LP
Compiled using AdapativeCpp v0.9.4, LLVM 15, and HIP 5.4.4 with
\f(CW-O3\fP and \f(CW-march=native\fP.
.G1
frame ht 3 wid 4
label top "Time to Generate 16GiB of Hashes" "on a Personal Computer"
copy "bench-results-pc.d"
.G2
.bp
.SH
Mystic Eightsocket System
.LP
DataSys Mystic Eightsocket system.
Compiled using AdaptiveCpp v0.9.4 and LLVM 15 with \f(CW-O3\fP and
\f(CW-march=native\fP.
.G1
frame ht 3 wid 4
label top "Time to Generate 16GiB of Hashes" "on Eightsocket"
copy "bench-results-eightsocket.d"
.G2
.bp
.SH
Chameleon Cloud Node
.IP \(bu 3n
Intel Xeon E5-2670 v3
.IP \(bu
128GB RAM
.IP \(bu
NVIDIA Tesla P100
.IP \(bu
CC-Ubuntu20.04-CUDA
.LP
Baremetal CHI@TACC gpu_p100 instance.
Compiled using AdaptiveCpp v0.9.4, LLVM 15, and CUDA 11.5 with \f(CW-O3\fP
and \f(CW-march=native\fP.
.G1
frame ht 3 wid 4
label top "Time to Generate 16GiB of Hashes" "on CHI@TACC gpu_p100"
copy "bench-results-cc.d"
.G2
