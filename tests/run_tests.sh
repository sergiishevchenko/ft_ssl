#!/usr/bin/env bash
#
# ft_ssl test suite: runs every check from docs/ru/verification.md.
#
# Usage: tests/run_tests.sh [-v] [--no-build] [--no-leaks]
#   -v, --verbose  show command and output of passing tests too
#   --no-build     skip Makefile checks and use the existing ./ft_ssl
#   --no-leaks     skip valgrind / leaks checks
#

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN="$ROOT/ft_ssl"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/ft_ssl_tests.XXXXXX")"
RUN="$TMP/run"
trap 'rm -rf "$TMP"' EXIT

VERBOSE=0
DO_BUILD=1
DO_LEAKS=1
for arg in "$@"; do
	case "$arg" in
		-v|--verbose) VERBOSE=1 ;;
		--no-build) DO_BUILD=0 ;;
		--no-leaks) DO_LEAKS=0 ;;
		-h|--help) sed -n '5,8p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
		*) printf 'unknown option: %s\n' "$arg" >&2; exit 2 ;;
	esac
done

if [ -t 1 ]; then
	R=$'\033[0m'; B=$'\033[1m'; D=$'\033[2m'
	RED=$'\033[31m'; GRN=$'\033[32m'; YLW=$'\033[33m'; CYN=$'\033[36m'
else
	R=; B=; D=; RED=; GRN=; YLW=; CYN=
fi

PASS=0
FAIL=0
SKIP=0
FAILED=()
SECTION=""

section() {
	SECTION="$1"
	printf '\n%s%s▌ %s%s\n' "$B" "$CYN" "$1" "$R"
}

ok() {
	PASS=$((PASS + 1))
	printf '  %s✓%s %s\n' "$GRN" "$R" "$1"
}

ko() {
	FAIL=$((FAIL + 1))
	FAILED+=("$SECTION — $1")
	printf '  %s✗ %s%s\n' "$RED" "$1" "$R"
}

skipped() {
	SKIP=$((SKIP + 1))
	printf '  %s○%s %s %s(%s)%s\n' "$YLW" "$R" "$1" "$D" "$2" "$R"
}

block() {
	printf '    %s%s%s\n' "$D" "$1" "$R"
	if [ -s "$3" ]; then
		awk -v c="$2" -v r="$R" '{ print "      " c "│" r " " $0 }' "$3"
	else
		printf '      %s│%s %s(empty)%s\n' "$2" "$R" "$D" "$R"
	fi
}

show_cmd() {
	printf '    %s$ %s%s\n' "$D" "$1" "$R"
}

to_file() {
	if [ -n "$1" ]; then
		printf '%s\n' "$1" >"$2"
	else
		: >"$2"
	fi
}

run() {
	( cd "$RUN" && eval "$1" ) >"$TMP/out" 2>"$TMP/err"
	CODE=$?
}

ref_md5() {
	if command -v md5sum >/dev/null 2>&1; then
		md5sum | awk '{ print $1 }'
	else
		md5 -q
	fi
}

ref_sha256() {
	if command -v sha256sum >/dev/null 2>&1; then
		sha256sum | awk '{ print $1 }'
	else
		shasum -a 256 | awk '{ print $1 }'
	fi
}

ref_whirlpool() {
	$WP_REF 2>/dev/null | awk '{ print $NF }'
}

#
# find_whirlpool_ref: picks the first openssl that hashes "abc" correctly.
# OpenSSL 3 needs the legacy provider, LibreSSL has whirlpool built in.
#
find_whirlpool_ref() {
	local bin args
	for bin in openssl /opt/homebrew/bin/openssl /usr/bin/openssl; do
		for args in "-provider legacy -provider default" ""; do
			WP_REF="$bin dgst -whirlpool $args"
			[ "$(printf 'abc' | ref_whirlpool)" = "$WP_ABC" ] && return 0
		done
	done
	WP_REF=""
	return 1
}

#
# expect NAME CMD STDOUT [STDERR] [EXIT]
#   STDERR "<any>" accepts any non-empty stderr
#   EXIT   0 (default), N, "nonzero" or "any"
#
expect() {
	local name="$1" cmd="$2" want_out="$3" want_err="${4-}" want_code="${5:-0}"
	local bad_out=0 bad_err=0 bad_code=0

	run "$cmd"
	to_file "$want_out" "$TMP/want_out"
	cmp -s "$TMP/want_out" "$TMP/out" || bad_out=1

	if [ "$want_err" = "<any>" ]; then
		[ -s "$TMP/err" ] || bad_err=1
		printf '<any non-empty message>\n' >"$TMP/want_err"
	else
		to_file "$want_err" "$TMP/want_err"
		cmp -s "$TMP/want_err" "$TMP/err" || bad_err=1
	fi

	case "$want_code" in
		any) ;;
		nonzero) [ "$CODE" -ne 0 ] || bad_code=1 ;;
		*) [ "$CODE" -eq "$want_code" ] || bad_code=1 ;;
	esac

	if [ $((bad_out + bad_err + bad_code)) -eq 0 ]; then
		ok "$name"
		if [ "$VERBOSE" -eq 1 ]; then
			show_cmd "$cmd"
			[ -s "$TMP/out" ] && block "stdout" "$GRN" "$TMP/out"
			[ -s "$TMP/err" ] && block "stderr" "$GRN" "$TMP/err"
		fi
		return
	fi

	ko "$name"
	show_cmd "$cmd"
	if [ "$bad_out" -eq 1 ]; then
		block "stdout expected" "$GRN" "$TMP/want_out"
		block "stdout got" "$RED" "$TMP/out"
	fi
	if [ "$bad_err" -eq 1 ]; then
		block "stderr expected" "$GRN" "$TMP/want_err"
		block "stderr got" "$RED" "$TMP/err"
	fi
	if [ "$bad_code" -eq 1 ]; then
		printf '    %sexit code expected %s, got %s%s\n' "$RED" "$want_code" "$CODE" "$R"
	fi
}

#
# same_as NAME OUR_CMD REF_CMD
#   stdout of both commands must be identical
#
same_as() {
	local name="$1" ours="$2" ref="$3"

	run "$ref"
	cp "$TMP/out" "$TMP/ref"
	run "$ours"

	if [ "$CODE" -eq 0 ] && [ -s "$TMP/ref" ] && cmp -s "$TMP/ref" "$TMP/out"; then
		ok "$name"
		if [ "$VERBOSE" -eq 1 ]; then
			show_cmd "$ours"
			block "ft_ssl = reference" "$GRN" "$TMP/out"
		fi
		return
	fi

	ko "$name"
	show_cmd "$ours"
	block "reference ($ref)" "$GRN" "$TMP/ref"
	block "ft_ssl" "$RED" "$TMP/out"
	[ -s "$TMP/err" ] && block "stderr" "$RED" "$TMP/err"
}

make_quiet() {
	( cd "$ROOT" && make "$@" ) >"$TMP/make" 2>&1
}

printf '\n%s  ft_ssl · test suite%s\n' "$B" "$R"
printf '  %sproject%s  %s\n' "$D" "$R" "$ROOT"
printf '  %ssystem%s   %s %s\n' "$D" "$R" "$(uname -s)" "$(uname -m)"

if [ "$DO_BUILD" -eq 1 ]; then
	section "5. Build"

	if make_quiet fclean && [ ! -e "$BIN" ]; then
		ok "make fclean removes ft_ssl"
	else
		ko "make fclean removes ft_ssl"
	fi

	if make_quiet && [ -x "$BIN" ]; then
		if grep -qi "warning" "$TMP/make"; then
			ko "make builds without warnings"
			block "make output" "$RED" "$TMP/make"
		else
			ok "make builds without warnings"
		fi
	else
		ko "make builds ft_ssl"
		block "make output" "$RED" "$TMP/make"
		printf '\n  %sBuild failed, nothing else to test.%s\n\n' "$RED" "$R"
		exit 1
	fi

	make_quiet
	if grep -qE "compile|ready" "$TMP/make"; then
		ko "second make does not relink"
		block "make output" "$RED" "$TMP/make"
	else
		ok "second make does not relink"
	fi

	make_quiet clean
	if [ -x "$BIN" ] && [ -z "$(find "$ROOT/srcs" -name '*.o')" ]; then
		ok "make clean removes .o and keeps ft_ssl"
	else
		ko "make clean removes .o and keeps ft_ssl"
	fi

	if make_quiet re && [ -x "$BIN" ]; then
		ok "make re rebuilds from scratch"
	else
		ko "make re rebuilds from scratch"
		block "make output" "$RED" "$TMP/make"
		exit 1
	fi
elif [ ! -x "$BIN" ]; then
	printf '\n  %s%s not found, run make first.%s\n\n' "$RED" "$BIN" "$R"
	exit 1
fi

mkdir -p "$RUN"
ln -s "$BIN" "$RUN/ft_ssl"
echo "And above all," >"$RUN/file"
echo "https://www.42.fr/" >"$RUN/website"
mkdir "$RUN/dir"

USAGE="ft_ssl: Error: 'foobar' is an invalid command.
Commands:
md5
sha256
whirlpool
Flags:
-p -q -r -s"

section "6. Output format (subject examples)"

expect "md5 stdin" \
	'echo "42 is nice" | ./ft_ssl md5' \
	'(stdin)= 35f1d6de0302e2086a4e472266efb3a9'

expect "md5 -p" \
	'echo "42 is nice" | ./ft_ssl md5 -p' \
	'("42 is nice")= 35f1d6de0302e2086a4e472266efb3a9'

expect "md5 -q -r" \
	'echo "Pity the living." | ./ft_ssl md5 -q -r' \
	'e20c3b973f63482a778f3fd1869b7f25'

expect "md5 file" \
	'./ft_ssl md5 file' \
	'MD5 (file) = 53d53ea94217b259c11a5a2d104ec58a'

expect "md5 -r file" \
	'./ft_ssl md5 -r file' \
	'53d53ea94217b259c11a5a2d104ec58a file'

expect "md5 -s" \
	"./ft_ssl md5 -s \"pity those that aren't following baerista on spotify.\"" \
	"MD5 (\"pity those that aren't following baerista on spotify.\") = a3c990a1964705d9bf0e602f44572f5f"

expect "md5 -p file" \
	'echo "be sure to handle edge cases carefully" | ./ft_ssl md5 -p file' \
	'("be sure to handle edge cases carefully")= 3553dc7dc5963b583c056d1b9fa3349c
MD5 (file) = 53d53ea94217b259c11a5a2d104ec58a'

expect "pipe without -p is ignored when a file is given" \
	'echo "some of this will not make sense at first" | ./ft_ssl md5 file' \
	'MD5 (file) = 53d53ea94217b259c11a5a2d104ec58a'

expect "md5 -p -r file" \
	'echo "but eventually you will understand" | ./ft_ssl md5 -p -r file' \
	'("but eventually you will understand")= dcdd84e0f635694d2a943fa8d3905281
53d53ea94217b259c11a5a2d104ec58a file'

expect "md5 -p -s file" \
	"echo \"GL HF let's go\" | ./ft_ssl md5 -p -s \"foo\" file" \
	"(\"GL HF let's go\")= d1e3cc342b6da09480b27ec57ff243e2
MD5 (\"foo\") = acbd18db4cc2f85cedef654fccc4a4d8
MD5 (file) = 53d53ea94217b259c11a5a2d104ec58a"

expect "flags after the first file are file names" \
	'echo "one more thing" | ./ft_ssl md5 -r -p -s "foo" file -s "bar"' \
	'("one more thing")= a0bd1876c6f011dd50fae52827f445f5
acbd18db4cc2f85cedef654fccc4a4d8 "foo"
53d53ea94217b259c11a5a2d104ec58a file' \
	'ft_ssl: md5: -s: No such file or directory
ft_ssl: md5: bar: No such file or directory' \
	nonzero

expect "md5 -r -q -p -s file" \
	'echo "just to be extra clear" | ./ft_ssl md5 -r -q -p -s "foo" file' \
	'just to be extra clear
3ba35f1ea0d170cb3b9a752e3360286c
acbd18db4cc2f85cedef654fccc4a4d8
53d53ea94217b259c11a5a2d104ec58a'

expect "sha256 -q file" \
	'./ft_ssl sha256 -q website' \
	'1ceb55d2845d9dd98557b50488db12bbf51aaca5aa9c1199eb795607a2457daf'

expect "sha256 -s" \
	'./ft_ssl sha256 -s "42 is nice"' \
	'SHA256 ("42 is nice") = b7e44c7a40c5f80139f0a50f3650fb2bd8d00b0d24667c4c2ca32c88e13b758f'

expect "sha256 file" \
	'./ft_ssl sha256 website' \
	'SHA256 (website) = 1ceb55d2845d9dd98557b50488db12bbf51aaca5aa9c1199eb795607a2457daf'

expect "sha256 -r -s" \
	'./ft_ssl sha256 -r -s "abc"' \
	'ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad "abc"'

expect "sha256 -p" \
	'echo "42 is nice" | ./ft_ssl sha256 -p' \
	'("42 is nice")= a5482539287a4069ebd3eb45a13a47b1968316c442a7e69bc6b9c100b101d65d'

section "7. Digests against $(command -v md5sum >/dev/null 2>&1 && echo md5sum || echo md5) / $(command -v sha256sum >/dev/null 2>&1 && echo sha256sum || echo shasum)"

same_as "md5 stdin with newline" \
	'echo "42 is nice" | ./ft_ssl md5 -q' \
	'echo "42 is nice" | ref_md5'

same_as "md5 stdin without newline" \
	"printf '42 is nice' | ./ft_ssl md5 -q" \
	"printf '42 is nice' | ref_md5"

same_as "sha256 file" \
	'./ft_ssl sha256 -q website' \
	'ref_sha256 < website'

same_as "sha256 -s" \
	'./ft_ssl sha256 -q -s "42 is nice"' \
	"printf '42 is nice' | ref_sha256"

expect "md5 empty string" './ft_ssl md5 -q -s ""' 'd41d8cd98f00b204e9800998ecf8427e'
expect "md5 \"abc\"" './ft_ssl md5 -q -s "abc"' '900150983cd24fb0d6963f7d28e17f72'
expect "sha256 empty string" './ft_ssl sha256 -q -s ""' 'e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855'
expect "sha256 \"abc\"" './ft_ssl sha256 -q -s "abc"' 'ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad'

SIZES="0 1 3 31 32 33 55 56 57 63 64 65 111 112 119 120 127 128 129 1000 4096"
for n in $SIZES; do
	printf '%*s' "$n" '' | tr ' ' 'a' >"$RUN/len_$n"
done
head -c 1048576 /dev/urandom >"$RUN/random.bin"
head -c 5242880 /dev/urandom >"$RUN/big.bin"

ALGOS="md5 sha256"
WP_ABC="4e2448a4c6f486bb16b6562c73b4020bf3043e3a731bce721ae1b303d97e6d4c7181eebdb6c57e277d0e34957114cbd6c797fc9d95d8b582d225292076d4eef5"
if find_whirlpool_ref; then
	ALGOS="$ALGOS whirlpool"
	printf '  %swhirlpool reference: %s%s\n' "$D" "$WP_REF" "$R"
else
	skipped "whirlpool against openssl" "no openssl with whirlpool support found"
fi

for algo in $ALGOS; do
	bad=""
	for n in $SIZES; do
		ours="$("$BIN" "$algo" -q "$RUN/len_$n")"
		ref="$(ref_$algo <"$RUN/len_$n")"
		[ "$ours" = "$ref" ] || bad="$bad $n"
	done
	if [ -z "$bad" ]; then
		ok "$algo padding boundaries ($(echo $SIZES | tr ' ' ','))"
	else
		ko "$algo padding boundaries"
		printf '    %swrong digest for sizes:%s%s\n' "$RED" "$bad" "$R"
	fi

	same_as "$algo binary file with NUL bytes (1 MB)" \
		"./ft_ssl $algo -q random.bin" \
		"ref_$algo < random.bin"

	same_as "$algo large file from stdin (5 MB)" \
		"./ft_ssl $algo -q < big.bin" \
		"ref_$algo < big.bin"
done

for algo in md5 sha256 whirlpool; do
	printf 'same input' >"$RUN/same"
	s="$("$BIN" "$algo" -q -s "same input")"
	f="$("$BIN" "$algo" -q "$RUN/same")"
	i="$(printf 'same input' | "$BIN" "$algo" -q)"
	if [ -n "$s" ] && [ "$s" = "$f" ] && [ "$s" = "$i" ]; then
		ok "$algo gives the same digest via -s, file and stdin"
	else
		ko "$algo gives the same digest via -s, file and stdin"
		printf '    %s-s     %s\n    file   %s\n    stdin  %s%s\n' "$RED" "$s" "$f" "$i" "$R"
	fi
done

section "8. Errors"

expect "unknown command" './ft_ssl foobar' '' "$USAGE" nonzero
expect "directory is not hashed" './ft_ssl md5 .' '' 'ft_ssl: md5: .: Is a directory' nonzero
expect "-s without argument" './ft_ssl md5 -s' '' 'ft_ssl: md5: option requires an argument -- s' nonzero
expect "missing file" './ft_ssl sha256 missing' '' 'ft_ssl: sha256: missing: No such file or directory' nonzero

expect "valid file still hashed next to a missing one" \
	'./ft_ssl md5 file missing' \
	'MD5 (file) = 53d53ea94217b259c11a5a2d104ec58a' \
	'ft_ssl: md5: missing: No such file or directory' \
	nonzero

expect "unknown flag is reported" './ft_ssl md5 -x' '' '<any>' nonzero

if [ -r /etc/shadow ]; then
	skipped "unreadable file" "running as root"
else
	touch "$RUN/noperm" && chmod 000 "$RUN/noperm"
	expect "unreadable file" './ft_ssl md5 noperm' '' '<any>' nonzero
fi

section "9. Commands from stdin"

expect "subject example through a pipe" \
	"printf 'md5 -s \"42 is nice\"\nsha256 -s \"42 is nice\"\n' | ./ft_ssl" \
	'MD5 ("42 is nice") = 0029a98ee90fdb85d70924d44d3c9e75
SHA256 ("42 is nice") = b7e44c7a40c5f80139f0a50f3650fb2bd8d00b0d24667c4c2ca32c88e13b758f'

expect "empty line, error, exit" \
	"printf '\nfoobar\nexit\n' | ./ft_ssl" \
	'' "$USAGE" 0

expect "unknown command does not stop the reader" \
	"printf 'foobar\nmd5 -s \"a\"\n' | ./ft_ssl" \
	'MD5 ("a") = 0cc175b9c0f1b6a831c399e269772661' "$USAGE" any

expect "quit stops reading" \
	"printf 'md5 -s a\nquit\nmd5 -s b\n' | ./ft_ssl" \
	'MD5 ("a") = 0cc175b9c0f1b6a831c399e269772661'

expect "exit stops reading" \
	"printf 'md5 -s a\nexit\nmd5 -s b\n' | ./ft_ssl" \
	'MD5 ("a") = 0cc175b9c0f1b6a831c399e269772661'

expect "last line without newline, then EOF" \
	"printf 'md5 -s a' | ./ft_ssl" \
	'MD5 ("a") = 0cc175b9c0f1b6a831c399e269772661'

expect "whitespace-only lines are ignored" \
	"printf '   \n\t\nmd5 -q -s a\n' | ./ft_ssl" \
	'0cc175b9c0f1b6a831c399e269772661'

expect "quoted argument with spaces" \
	"printf 'md5 -s \"a b\"\n' | ./ft_ssl" \
	'MD5 ("a b") = 0cc9cd4dd26c5137b675a0d819cb9ab0'

expect "file argument" \
	"printf 'sha256 -q website\n' | ./ft_ssl" \
	'1ceb55d2845d9dd98557b50488db12bbf51aaca5aa9c1199eb795607a2457daf'

expect "no prompt and no output on empty input" "./ft_ssl < /dev/null" ''

if ! command -v script >/dev/null 2>&1; then
	skipped "prompt is shown on a terminal" "script not installed"
else
	if script --version 2>/dev/null | grep -q util-linux; then
		TTY_CMD="printf 'exit\n' | script -qec ./ft_ssl /dev/null"
	else
		TTY_CMD="printf 'exit\n' | script -q /dev/null ./ft_ssl"
	fi
	run "$TTY_CMD"
	if grep -q 'ft_ssl> ' "$TMP/out"; then
		ok "prompt is shown on a terminal"
	else
		ko "prompt is shown on a terminal"
		show_cmd "$TTY_CMD"
		block "output" "$RED" "$TMP/out"
	fi
fi

section "10. Whirlpool (ISO/IEC 10118-3 vectors)"

expect "empty string" './ft_ssl whirlpool -q -s ""' \
	'19fa61d75522a4669b44e39c1d2e1726c530232130d407f89afee0964997f7a73e83be698b288febcf88e3e03c4f0757ea8964e59b63d93708b138cc42a66eb3'
expect "\"a\"" './ft_ssl whirlpool -q -s "a"' \
	'8aca2602792aec6f11a67206531fb7d7f0dff59413145e6973c45001d0087b42d11bc645413aeff63a42391a39145a591a92200d560195e53b478584fdae231a'
expect "\"abc\"" './ft_ssl whirlpool -q -s "abc"' "$WP_ABC"
expect "\"message digest\"" './ft_ssl whirlpool -q -s "message digest"' \
	'378c84a4126e2dc6e56dcc7458377aac838d00032230f53ce1f5700c0ffb4d3b8421557659ef55c106b4b52ac5a4aaa692ed920052838f3362e86dbd37a8903e'
expect "a..z" './ft_ssl whirlpool -q -s "abcdefghijklmnopqrstuvwxyz"' \
	'f1d754662636ffe92c82ebb9212a484a8d38631ead4238f5442ee13b8054e41b08bf2a9251c30b6a0b8aae86177ab4a6f68f673e7207865d5d9819a3dba4eb3b'
expect "8 × \"1234567890\" (80 bytes, two blocks)" \
	'./ft_ssl whirlpool -q -s "12345678901234567890123456789012345678901234567890123456789012345678901234567890"' \
	'466ef18babb0154d25b9d38a6414f5c08784372bccb204d6549c4afadb6014294d5bd8df2a6c44e538cd047b2681a51a2c60481e88c5a20b2c2a80cf3a9a083b'
expect "quick brown fox" './ft_ssl whirlpool -q -s "The quick brown fox jumps over the lazy dog"' \
	'b97de512e91e3828b40d2b0fdce9ceb3c4a71f9bea8d88e75c4fa854df36725fd2b52eb6544edcacd6f8beddfea403cb55ae31f03ad62a5ef54e42ee82c3fb35'
expect "output format without -q" './ft_ssl whirlpool -s "abc"' "WHIRLPOOL (\"abc\") = $WP_ABC"
expect "-r -s" './ft_ssl whirlpool -r -s "abc"' "$WP_ABC \"abc\""
expect "-p" "printf 'abc' | ./ft_ssl whirlpool -p" "(\"abc\")= $WP_ABC"

section "11. Memory leaks"

LEAK_CASES=(
	'echo "42 is nice" | ./ft_ssl md5'
	'./ft_ssl sha256 -s "42 is nice"'
	'./ft_ssl whirlpool -s "abc"'
	'echo "one more thing" | ./ft_ssl md5 -r -p -s "foo" file -s "bar"'
	'./ft_ssl md5 -s "hello" missing'
	'./ft_ssl md5 .'
	'./ft_ssl md5 -s'
	'./ft_ssl foobar'
	"printf 'md5 -s \"hello\"\nfoobar\n\nsha256 -q website\nexit\n' | ./ft_ssl"
)

if [ "$DO_LEAKS" -eq 0 ]; then
	skipped "leak checks" "--no-leaks"
elif command -v valgrind >/dev/null 2>&1; then
	LEAK_TOOL="valgrind -q --leak-check=full --show-leak-kinds=all --errors-for-leak-kinds=all --error-exitcode=42"
	for cmd in "${LEAK_CASES[@]}"; do
		run "${cmd/.\/ft_ssl/$LEAK_TOOL ./ft_ssl}"
		if [ "$CODE" -ne 42 ] && ! grep -q "==[0-9][0-9]*==" "$TMP/err"; then
			ok "valgrind: $cmd"
		else
			ko "valgrind: $cmd"
			block "valgrind" "$RED" "$TMP/err"
		fi
	done
elif [ "$(uname -s)" = "Darwin" ] && command -v leaks >/dev/null 2>&1; then
	LEAK_TOOL="leaks -quiet --atExit --"
	for cmd in "${LEAK_CASES[@]}"; do
		run "${cmd/.\/ft_ssl/$LEAK_TOOL ./ft_ssl}"
		if grep -q " 0 leaks for 0 total leaked bytes" "$TMP/out"; then
			ok "leaks: $cmd"
		else
			ko "leaks: $cmd"
			block "leaks" "$RED" "$TMP/out"
		fi
	done
else
	skipped "leak checks" "neither valgrind nor leaks is installed"
fi

TOTAL=$((PASS + FAIL + SKIP))
printf '\n%s%s%s\n' "$D" "────────────────────────────────────────────────────────" "$R"
printf '  %s%s%d passed%s' "$B" "$GRN" "$PASS" "$R"
if [ "$FAIL" -gt 0 ]; then
	printf '   %s%s%d failed%s' "$B" "$RED" "$FAIL" "$R"
else
	printf '   %s0 failed%s' "$D" "$R"
fi
[ "$SKIP" -gt 0 ] && printf '   %s%d skipped%s' "$YLW" "$SKIP" "$R"
printf '   %sof %d in %ds%s\n' "$D" "$TOTAL" "$SECONDS" "$R"

if [ "$FAIL" -gt 0 ]; then
	printf '\n  %sFailed:%s\n' "$RED" "$R"
	for name in "${FAILED[@]}"; do
		printf '    %s•%s %s\n' "$RED" "$R" "$name"
	done
	printf '\n'
	exit 1
fi

printf '\n'
exit 0
