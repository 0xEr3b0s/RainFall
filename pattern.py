#!/usr/bin/env python3
import sys
import string
import argparse

UPPER = string.ascii_uppercase
LOWER = string.ascii_lowercase
DIGITS = string.digits

STRIDE_U = len(LOWER) * len(DIGITS)          # 260
STRIDE_L = len(DIGITS)                         # 10
TOTAL_PATTERNS = len(UPPER) * len(LOWER) * len(DIGITS)   # 6760
TOTAL_BYTES = TOTAL_PATTERNS * 3                          # 20280

COMMANDS = {"create", "offset", "pattern", "pack", "show"}


def full_pattern():
    return "".join(
        f"{u}{l}{d}"
        for u in UPPER
        for l in LOWER
        for d in DIGITS
    )


def offset_to_pattern(offset):
    if not 0 <= offset < TOTAL_PATTERNS:
        raise ValueError(f"Index out of range: 0 to {TOTAL_PATTERNS - 1}")
    u, rest = divmod(offset, STRIDE_U)
    l, d = divmod(rest, STRIDE_L)
    return f"{UPPER[u]}{LOWER[l]}{DIGITS[d]}"


def cmd_create(length):
    if length < 0:
        raise ValueError("Length must be positive.")
    if length > TOTAL_BYTES:
        raise ValueError(f"Beyond the period: {length} > {TOTAL_BYTES} bytes.")
    print(full_pattern()[:length])


def cmd_pattern(offset):
    print(offset_to_pattern(offset))


def candidates(value):
    """Return the byte sequences to search for.

    A 0x-prefixed or purely-hexadecimal value is decoded first and
    tried in both byte orders; otherwise it is taken as a literal
    pattern.
    """
    looks_hex = value.lower().startswith("0x")
    raw = value[2:] if looks_hex else value

    is_hex = bool(raw) and all(c in string.hexdigits for c in raw)
    if looks_hex or (is_hex and len(raw) % 2 == 0):
        if is_hex:
            if len(raw) % 2:
                raw = "0" + raw
            data = bytes.fromhex(raw)
            try:
                return [
                    ("big-endian", data.decode("ascii")),
                    ("little-endian", data[::-1].decode("ascii")),
                ]
            except UnicodeDecodeError:
                return []

    if value and all(c in UPPER + LOWER + DIGITS for c in value):
        return [("ascii", value)]

    return []


def all_positions(pattern, needle):
    """Every occurrence of the needle inside the pattern."""
    positions = []
    start = pattern.find(needle)
    while start != -1:
        positions.append(start)
        start = pattern.find(needle, start + 1)
    return positions


def cmd_offset(value):
    probes = candidates(value)
    if not probes:
        print("Invalid input: give a pattern (e.g. Aa3b) or a hex (e.g. 0x41346341).")
        return

    pattern = full_pattern()
    found = False

    for byte_order, needle in probes:
        positions = all_positions(pattern, needle)
        if not positions:
            continue
        found = True

        print(f"Sequence   : {needle}")
        print(f"Byte order : {byte_order}")
        print(f"Length     : {len(needle)} bytes")
        if len(positions) == 1:
            print(f"Offset     : {positions[0]} bytes")
        else:
            joined = ", ".join(str(p) for p in positions)
            print(f"Offsets    : {joined} bytes  (ambiguous, read more bytes)")
        print()

    if not found:
        print("No match: value outside the pattern, or too short.")


def cmd_pack(value, width, order, raw_output):
    """Pack an address into its byte representation."""
    raw = value[2:] if value.lower().startswith("0x") else value
    if not raw or any(c not in string.hexdigits for c in raw):
        raise ValueError("Give an address, e.g. 0x08049464 or 08049464.")

    number = int(raw, 16)
    if number < 0 or number >= (1 << (width * 8)):
        raise ValueError(f"Address does not fit in {width} bytes.")

    data = number.to_bytes(width, order)

    if raw_output:
        sys.stdout.buffer.write(data)
    else:
        print("".join(f"\\x{b:02x}" for b in data))


def parse_value(value):
    """Interpret the input as a number, whatever its notation.

    Prefix wins (0x, 0b, 0o). A plain decimal is decimal. An
    unprefixed all-hex string is read as hexadecimal. Anything else
    is read as ASCII bytes (big-endian).
    """
    low = value.lower()
    if low.startswith("0x"):
        return int(value, 16)
    if low.startswith("0b"):
        return int(value, 2)
    if low.startswith("0o"):
        return int(value, 8)
    if value.isdigit():
        return int(value, 10)
    if value and all(c in string.hexdigits for c in value):
        return int(value, 16)
    try:
        return int.from_bytes(value.encode("ascii"), "big")
    except (UnicodeEncodeError, ValueError):
        raise ValueError("Cannot interpret this value.")


def _escaped(data):
    return "".join(f"\\x{b:02x}" for b in data)


def _printable(data):
    return "".join(chr(b) if 32 <= b < 127 else "." for b in data)


def cmd_show(value):
    """Display a value under every common representation."""
    number = parse_value(value)
    if number < 0:
        raise ValueError("Negative values are not supported.")

    nbytes = max(1, (number.bit_length() + 7) // 8)
    big = number.to_bytes(nbytes, "big")
    little = big[::-1]

    print(f"Decimal       : {number}")
    print(f"Hex           : 0x{number:x}")
    print(f"Binary        : 0b{number:b}")
    print(f"Octal         : 0o{number:o}")
    print(f"Width         : {nbytes} bytes")
    print()
    print(f"Big-endian    : {_escaped(big)}")
    print(f"Little-endian : {_escaped(little)}")
    print()
    print(f"ASCII (BE)    : {_printable(big)}")
    print(f"ASCII (LE)    : {_printable(little)}")


def main():
    # Raccourci : "pattern.py <valeur>" sans verbe est traite comme "show <valeur>".
    argv = sys.argv[1:]
    if argv and argv[0] not in COMMANDS and argv[0] not in ("-h", "--help"):
        argv = ["show"] + argv

    parser = argparse.ArgumentParser(description="Cyclic pattern generation and lookup.")
    sub = parser.add_subparsers(dest="command", required=True)

    p_create = sub.add_parser("create", help="output N bytes of the pattern")
    p_create.add_argument("length", type=int)

    p_offset = sub.add_parser("offset", help="find the offset of a pattern or hex value")
    p_offset.add_argument("value")

    p_pattern = sub.add_parser("pattern", help="return the pattern at a given index")
    p_pattern.add_argument("index", type=int)

    p_pack = sub.add_parser("pack", help="turn an address into \\xXX bytes")
    p_pack.add_argument("value", help="address, e.g. 0x08049464")
    p_pack.add_argument("-w", "--width", type=int, default=4,
                        help="byte width: 4 (32-bit) or 8 (64-bit), default 4")
    p_pack.add_argument("-b", "--big-endian", action="store_true",
                        help="use big-endian instead of little-endian")
    p_pack.add_argument("-r", "--raw", action="store_true",
                        help="emit raw bytes on stdout (for piping)")

    p_show = sub.add_parser("show", help="show a value in every representation")
    p_show.add_argument("value", help="hex, decimal, binary, or ASCII")

    args = parser.parse_args(argv)
    try:
        if args.command == "create":
            cmd_create(args.length)
        elif args.command == "offset":
            cmd_offset(args.value)
        elif args.command == "pattern":
            cmd_pattern(args.index)
        elif args.command == "pack":
            order = "big" if args.big_endian else "little"
            cmd_pack(args.value, args.width, order, args.raw)
        elif args.command == "show":
            cmd_show(args.value)
    except ValueError as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
