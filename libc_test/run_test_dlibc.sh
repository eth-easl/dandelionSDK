#!/bin/sh
#
# Shell-based test runner for dlibc.
# Replaces the C-based runtest which requires fork/exec (unsupported by dlibc).
#
# Interface matches the original: runtest [-t timeout] [-w wrap] cmd [args..]
# Output on stdout/stderr still goes to the .err file via Makefile redirection,
# but we also preserve per-test host stdio and Dandelion output_sets artifacts.
#
# The Dandelion debug runtime requires input_sets/ and output_sets/ in CWD.
# We create a temporary directory per test so tests don't interfere with
# each other's I/O state.

TIMEOUT=5
WRAP=""

while [ $# -gt 0 ]; do
    case "$1" in
        -t) TIMEOUT="$2"; shift 2 ;;
        -w) WRAP="$2"; shift 2 ;;
        *)  break ;;
    esac
done

test_bin="$1"
shift

if [ -z "$test_bin" ]; then
    echo "FAIL (no test binary specified) [internal]"
    exit 0
fi

# Resolve to absolute path before cd-ing into tmpdir
case "$test_bin" in
    /*) ;;
    *)  test_bin="$(pwd)/$test_bin" ;;
esac

# Persist per-test artifacts under libc-test/logs unless overridden.
script_dir=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
artifact_root="${DLIBC_STDIO_ARTIFACTS:-$script_dir/logs/test_stdio}"
mkdir -p "$artifact_root"

test_rel="$test_bin"
case "$test_bin" in
    "$script_dir"/*)
        test_rel="${test_bin#$script_dir/}"
        ;;
esac
artifact_dir="$artifact_root/$test_rel/"
rm -rf "$artifact_dir"
mkdir -p "$artifact_dir"

# Per-test isolated working directory with Dandelion runtime dirs
tmpdir=$(mktemp -d)
mkdir -p "$tmpdir/input_sets" "$tmpdir/output_sets/stdio"

host_stdout="$artifact_dir/host.stdout"
host_stderr="$artifact_dir/host.stderr"
meta_file="$artifact_dir/meta.txt"
dandelion_stdout="$artifact_dir/dandelion.stdout"
dandelion_stderr="$artifact_dir/dandelion.stderr"

# Run the test binary with a timeout.
# Preserve host stdout/stderr separately, then replay them so Makefile still
# captures the output in the traditional .err file.
if [ -n "$WRAP" ]; then
    (cd "$tmpdir" && timeout "$TIMEOUT" "$WRAP" "$test_bin" "$@" >"$host_stdout" 2>"$host_stderr")
else
    (cd "$tmpdir" && timeout "$TIMEOUT" "$test_bin" "$@" >"$host_stdout" 2>"$host_stderr")
fi
rc=$?

{
    printf 'test_bin=%s\n' "$test_bin"
    printf 'test_rel=%s\n' "$test_rel"
    printf 'timeout=%s\n' "$TIMEOUT"
    printf 'wrap=%s\n' "$WRAP"
    printf 'exit_code=%s\n' "$rc"
    printf 'artifact_dir=%s\n' "$artifact_dir"
    printf 'tmpdir=%s\n' "$tmpdir"
} >"$meta_file"

if [ -d "$tmpdir/output_sets" ]; then
    cp -R "$tmpdir/output_sets" "$artifact_dir/output_sets"
fi

if [ -f "$tmpdir/output_sets/stdio/stdout" ]; then
    cp "$tmpdir/output_sets/stdio/stdout" "$dandelion_stdout"
fi

if [ -f "$tmpdir/output_sets/stdio/stderr" ]; then
    cp "$tmpdir/output_sets/stdio/stderr" "$dandelion_stderr"
fi

cat "$host_stdout"
cat "$host_stderr" >&2

rm -rf "$tmpdir"

if [ $rc -eq 124 ]; then
    echo "FAIL $test_bin [timed out]"
elif [ $rc -ne 0 ]; then
    echo "FAIL $test_bin [status $rc]"
fi

exit 0
