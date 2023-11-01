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
Compiled using AdapativeCpp with \f(CW-O3\fP and \f(CW-march=native\fP.
.G1
frame ht 3 wid 4
label top "Time to Generate 16GiB of Hashes" "on a Personal Computer"
copy "bench-results-pc.d"
.G2
.
.SH
Mystic Eightsocket System
.LP
DataSys Mystic Eightsocket system.
Compiled using DP++ with \f(CW-O3\fP and \f(CW-march=native\fP.
See notes in README.
.G1
frame ht 3 wid 4
label top "Time to Generate 16GiB of Hashes" "on Eightsocket"
copy "bench-results-eightsocket.d"
.G2
