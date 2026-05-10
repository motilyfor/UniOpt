#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 1 ]]; then
  echo "Usage: $0 <google-benchmark-csv>"
  exit 1
fi

csv="$1"

awk -F',' '
BEGIN {
  printf("%-10s %-8s %-14s %-14s %-8s\n", "type", "count", "std GiB/s", "algo GiB/s", "ratio");
}

$1 ~ /_mean"$/ {
  name=$1;
  gsub(/"/, "", name);

  bps=$6+0.0;
  gibps=bps/1024/1024/1024;

  if (match(name, /BM_StdMemcpy_(Int32|Float)\/([0-9]+)/, m)) {
    key=m[1]":"m[2];
    std[key]=gibps;
  }

  if (match(name, /BM_AlgoMemcpy_(Int32|Float)\/([0-9]+)/, m)) {
    key=m[1]":"m[2];
    algo[key]=gibps;
  }
}

END {
  n=0;
  for (k in std) {
    split(k, p, ":");
    type=p[1];
    count=p[2]+0;
    if (algo[k] > 0) {
      ratio=algo[k]/std[k];
      idx=type":"sprintf("%08d", count);
      line[idx]=sprintf("%-10s %-8d %-14.3f %-14.3f %-8.3f", type, count, std[k], algo[k], ratio);
      n++;
    }
  }

  if (n == 0) {
    print "No matching mean rows found.";
    exit 2;
  }

  # Print Int32 first then Float; within each, sort by count
  for (pass=1; pass<=2; pass++) {
    prefix=(pass==1?"Int32":"Float");
    for (i=0; i<=99999999; i++) {
      idx=prefix":"sprintf("%08d", i);
      if (idx in line) print line[idx];
    }
  }
}
' "$csv"
