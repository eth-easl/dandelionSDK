#!/bin/sh
# gen_csv.sh — One detailed test results CSV
# Usage: gen_csv.sh [build_dir]
#
# Output columns:
#   test,result,failure_stage,evidence_file
#
# result is PASS/FAIL where:
#   FAIL includes compile, link, shared-link, and runtime failures.

B="${1:-src}"
TMP_RAN="$(mktemp)"
TMP_BUILD="$(mktemp)"
TMP_OUT="$(mktemp)"
trap 'rm -f "$TMP_RAN" "$TMP_BUILD" "$TMP_OUT"' EXIT INT TERM

artifact_exists() {
  case "$1" in
    *.ld.err)
      [ -e "${1%.ld.err}.exe" ]
      ;;
    *.so.err)
      [ -e "${1%.so.err}.so" ]
      ;;
    *.o.err)
      [ -e "${1%.o.err}.o" ]
      ;;
    *.lo.err)
      [ -e "${1%.lo.err}.lo" ]
      ;;
    *)
      return 1
      ;;
  esac
}

# Runtime outcomes for tests that reached execution.
find "$B" -name '*.err' \
  ! -name '*.o.err' \
  ! -name '*.lo.err' \
  ! -name '*.ld.err' \
  ! -name '*.so.err' \
  ! -path '*/common/*' \
  | sort \
  | while read -r f; do
    name="${f%.err}"
    name="${name#$B/}"
    echo "$name" >> "$TMP_RAN"
    if grep -q '^FAIL ' "$f" 2>/dev/null; then
      echo "$name,FAIL,runtime,$f" >> "$TMP_OUT"
    else
      echo "$name,PASS,pass,$f" >> "$TMP_OUT"
    fi
done

# Build/link outcomes for tests that never produced runtime .err.
find "$B" \( -name '*.o.err' -o -name '*.lo.err' -o -name '*.ld.err' -o -name '*.so.err' \) \
  ! -path '*/common/*' \
  | sort \
  | while read -r f; do
    [ -s "$f" ] || continue
    artifact_exists "$f" && continue
    case "$f" in
      *.ld.err)
        name="${f%.ld.err}"
        stage="link"
        prio=4
        ;;
      *.so.err)
        name="${f%.so.err}"
        stage="shared_link"
        prio=3
        ;;
      *.o.err)
        name="${f%.o.err}"
        stage="compile"
        prio=2
        ;;
      *.lo.err)
        name="${f%.lo.err}"
        stage="shared_compile"
        prio=1
        ;;
      *)
        continue
        ;;
    esac
    name="${name#$B/}"
    if ! grep -qxF "$name" "$TMP_RAN" 2>/dev/null; then
      echo "$name,$stage,$f,$prio" >> "$TMP_BUILD"
    fi
done

# Keep one build/link failure row per test, preferring higher-priority stage.
awk -F, '
{
  n=$1; st=$2; ev=$3; p=$4+0;
  if (!(n in best) || p > best[n]) {
    best[n]=p; stage[n]=st; evidence[n]=ev;
  }
}
END {
  for (n in best) {
    print n ",FAIL," stage[n] "," evidence[n];
  }
}' "$TMP_BUILD" >> "$TMP_OUT"

echo "test,result,failure_stage,evidence_file"
sort -t, -k1,1 "$TMP_OUT"
