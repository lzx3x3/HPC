#!/usr/bin/env python

import matplotlib.pyplot as plt

def parse_benchmarks_data(f, commName):
    f.readline() # skip the line with the headers
    msgs = []
    bws  = []
    line = f.readline()
    while line:
        if line.startswith("MPI"): break
        (P, msgSize, totalSize, time, bw) = line.split()
        msgSize = int(msgSize)
        bw = float(bw)
        found = False
        for i in range(len(msgs)):
            if (msgs[i] == msgSize):
                found = True
                if (bw > bws[i]):
                    bws[i] = bw
        if not found:
            msgs.append(msgSize)
            bws.append(bw)
        line = f.readline()
    return ((commName, msgs, bws), line)

ax = plt.gca()
ax.set_title("MPI Benchmark Bandwidths")
ax.set_xlabel("Message Size (B)")
ax.set_xscale("log")
ax.set_ylabel("Bandwidth (B/s)")
ax.set_yscale("log")
lines = []

import sys

with sys.stdin as f:
    line = f.readline()
    while line:
        if line.startswith("MPI Point-to-point test:"):
            (data, line) = parse_benchmarks_data(f, "Send/Recv")
            lines.append(data)
        elif line.startswith("MPI Point-to-point synchronous test:"):
            (data, line) = parse_benchmarks_data(f, "Ssend/Recv")
            lines.append(data)
        elif line.startswith("MPI One-to-all broadcast test:"):
            (data, line) = parse_benchmarks_data(f, "Bcast")
            lines.append(data)
        elif line.startswith("MPI One-to-all scatter test:"):
            (data, line) = parse_benchmarks_data(f, "Scatter")
            lines.append(data)
        elif line.startswith("MPI All-to-one reduce test:"):
            (data, line) = parse_benchmarks_data(f, "Reduce")
            lines.append(data)
        elif line.startswith("MPI All-to-one gather test:"):
            (data, line) = parse_benchmarks_data(f, "Gather")
            lines.append(data)
        elif line.startswith("MPI All-to-all reduce test:"):
            (data, line) = parse_benchmarks_data(f, "Allreduce")
            lines.append(data)
        elif line.startswith("MPI All-to-all gather test:"):
            (data, line) = parse_benchmarks_data(f, "Allgather")
            lines.append(data)
        elif line.startswith("MPI All-to-all transpose test:"):
            (data, line) = parse_benchmarks_data(f, "Alltoall")
            lines.append(data)
        else:
            line = f.readline()

for line in lines:
    print(line)
    ax.plot(line[1],line[2],label=line[0])

ax.legend()
plt.savefig('benchmarks.png')
