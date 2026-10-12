#!/usr/bin/env python3
"""Small reproducible DWARF/symbol inspector for the recovered df_channel_r.

The 0828 ELF retains full DWARF and a static symbol table.  This tool exposes
the evidence used by the source recovery without requiring a proprietary IDA
database.  It intentionally performs no writes to the input binary.
"""

from __future__ import annotations

import argparse
import re
import struct
import sys
from pathlib import Path

from capstone import CS_ARCH_X86, CS_MODE_32, Cs
from capstone.x86 import X86_OP_IMM
from elftools.dwarf.descriptions import describe_form_class
from elftools.elf.elffile import ELFFile


def text_attr(die, key: str) -> str:
    attr = die.attributes.get(key)
    if not attr:
        return ""
    value = attr.value
    return value.decode(errors="replace") if isinstance(value, bytes) else str(value)


def type_name(die, cu) -> str:
    attr = die.attributes.get("DW_AT_type")
    if not attr:
        return "void"
    target = cu.get_DIE_from_refaddr(attr.value + cu.cu_offset)
    wrappers = {
        "DW_TAG_const_type": "const ",
        "DW_TAG_volatile_type": "volatile ",
        "DW_TAG_pointer_type": "*",
        "DW_TAG_reference_type": "&",
        "DW_TAG_typedef": "",
    }
    prefix = ""
    suffix = ""
    seen = set()
    while target and target.offset not in seen:
        seen.add(target.offset)
        name = text_attr(target, "DW_AT_name")
        if target.tag in ("DW_TAG_structure_type", "DW_TAG_class_type", "DW_TAG_union_type", "DW_TAG_enumeration_type"):
            return prefix + (name or target.tag.removeprefix("DW_TAG_").removesuffix("_type")) + suffix
        if target.tag == "DW_TAG_base_type":
            return prefix + name + suffix
        if target.tag == "DW_TAG_array_type":
            count = "?"
            for child in target.iter_children():
                upper = child.attributes.get("DW_AT_upper_bound")
                if upper:
                    count = str(int(upper.value) + 1)
                    break
            suffix += f"[{count}]"
        elif target.tag == "DW_TAG_pointer_type":
            suffix = "*" + suffix
        elif target.tag == "DW_TAG_reference_type":
            suffix = "&" + suffix
        elif target.tag in ("DW_TAG_const_type", "DW_TAG_volatile_type"):
            prefix += wrappers[target.tag]
        elif target.tag == "DW_TAG_typedef" and name:
            return prefix + name + suffix
        next_attr = target.attributes.get("DW_AT_type")
        if not next_attr:
            return prefix + (name or "void") + suffix
        target = cu.get_DIE_from_refaddr(next_attr.value + cu.cu_offset)
    return prefix + "<?>" + suffix


def member_offset(die) -> str:
    attr = die.attributes.get("DW_AT_data_member_location")
    if not attr:
        return "?"
    if describe_form_class(attr.form) == "constant":
        return str(attr.value)
    # GCC 4.x emits DW_FORM_block1 with DW_OP_plus_uconst (0x23).
    data = bytes(attr.value) if isinstance(attr.value, (bytes, bytearray, list)) else b""
    if data[:1] == b"\x23":
        value = 0
        shift = 0
        for byte in data[1:]:
            value |= (byte & 0x7F) << shift
            if not byte & 0x80:
                return str(value)
            shift += 7
    return repr(attr.value)


def iter_dies(dwarf):
    for cu in dwarf.iter_CUs():
        for die in cu.iter_DIEs():
            yield cu, die


def print_types(dwarf, pattern: re.Pattern[str]) -> None:
    tags = {"DW_TAG_structure_type", "DW_TAG_class_type", "DW_TAG_union_type", "DW_TAG_enumeration_type"}
    for cu, die in iter_dies(dwarf):
        name = text_attr(die, "DW_AT_name")
        if die.tag not in tags or not name or not pattern.search(name):
            continue
        size_attr = die.attributes.get("DW_AT_byte_size")
        size = size_attr.value if size_attr else "?"
        print(f"TYPE {name} size={size} tag={die.tag}")
        for child in die.iter_children():
            if child.tag == "DW_TAG_member":
                print(f"  +{member_offset(child):>4} {type_name(child, cu):<36} {text_attr(child, 'DW_AT_name')}")
            elif child.tag == "DW_TAG_enumerator":
                value = child.attributes.get("DW_AT_const_value")
                print(f"  {text_attr(child, 'DW_AT_name')}={value.value if value else '?'}")


def symbol_table(elf):
    table = elf.get_section_by_name(".symtab")
    return [s for s in table.iter_symbols()] if table else []


def addr_to_offset(elf, address: int) -> int | None:
    for segment in elf.iter_segments():
        start = segment["p_vaddr"]
        end = start + segment["p_filesz"]
        if start <= address < end:
            return int(segment["p_offset"] + address - start)
    return None


def c_string_at(elf, raw: bytes, address: int) -> str | None:
    off = addr_to_offset(elf, address)
    if off is None or off >= len(raw):
        return None
    end = raw.find(b"\0", off, min(len(raw), off + 256))
    if end <= off:
        return None
    data = raw[off:end]
    if not data or any(c < 9 or (13 < c < 32) for c in data):
        return None
    try:
        return data.decode("utf-8")
    except UnicodeDecodeError:
        return data.decode("latin1", errors="replace")


def function_dies(dwarf):
    result = {}
    for cu, die in iter_dies(dwarf):
        if die.tag != "DW_TAG_subprogram":
            continue
        low = die.attributes.get("DW_AT_low_pc")
        if low:
            result[int(low.value)] = (cu, die)
    return result


def source_location(dwarf, cu, die) -> str:
    index = die.attributes.get("DW_AT_decl_file")
    line = die.attributes.get("DW_AT_decl_line")
    if not index:
        return "?"
    program = dwarf.line_program_for_CU(cu)
    files = program["file_entry"] if program else []
    file_name = "?"
    if 0 < int(index.value) <= len(files):
        file_name = files[int(index.value) - 1].name.decode(errors="replace")
    return f"{file_name}:{line.value if line else '?'}"


def descendants(die):
    for child in die.iter_children():
        yield child
        yield from descendants(child)


def print_functions(
    elf,
    dwarf,
    raw: bytes,
    pattern: re.Pattern[str],
    disassemble: bool,
    near_calls: re.Pattern[str] | None,
    address_from: int | None,
    address_to: int | None,
) -> None:
    symbols = symbol_table(elf)
    funcs = [s for s in symbols if s["st_info"]["type"] == "STT_FUNC" and s.name and pattern.search(s.name)]
    by_addr = {int(s["st_value"]): s.name for s in symbols if s.name and s["st_value"]}
    dies = function_dies(dwarf)
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = True
    for symbol in sorted(funcs, key=lambda s: int(s["st_value"])):
        address, size = int(symbol["st_value"]), int(symbol["st_size"])
        details = ""
        if address in dies:
            cu, die = dies[address]
            details = " " + source_location(dwarf, cu, die)
        print(f"FUNC {address:#010x} size={size} {symbol.name}{details}")
        if address in dies:
            cu, die = dies[address]
            for child in descendants(die):
                if child.tag in ("DW_TAG_formal_parameter", "DW_TAG_variable"):
                    kind = "PARAM" if child.tag == "DW_TAG_formal_parameter" else "LOCAL"
                    print(f"  {kind:<5} {type_name(child, cu):<40} {text_attr(child, 'DW_AT_name')}")
        if not disassemble or not size:
            continue
        off = addr_to_offset(elf, address)
        if off is None:
            continue
        instructions = list(md.disasm(raw[off : off + size], address))
        selected = [
            index
            for index, insn in enumerate(instructions)
            if (address_from is None or insn.address >= address_from)
            and (address_to is None or insn.address < address_to)
        ]
        if near_calls is not None:
            indexes = set()
            for index, insn in enumerate(instructions):
                if insn.mnemonic != "call" or not insn.operands or insn.operands[0].type != X86_OP_IMM:
                    continue
                target = int(insn.operands[0].imm) & 0xFFFFFFFF
                if near_calls.search(by_addr.get(target, "")):
                    indexes.update(range(max(0, index - 24), min(len(instructions), index + 25)))
            selected = sorted(indexes)
        previous = -2
        for index in selected:
            insn = instructions[index]
            if index != previous + 1:
                print("  ...")
            previous = index
            note = ""
            for operand in insn.operands:
                if operand.type != X86_OP_IMM:
                    continue
                imm = int(operand.imm) & 0xFFFFFFFF
                if imm in by_addr:
                    note = f" ; {by_addr[imm]}"
                else:
                    string = c_string_at(elf, raw, imm)
                    if string:
                        note = f" ; {string!r}"
            print(f"  {insn.address:08x}: {insn.mnemonic:<8} {insn.op_str}{note}")


def main() -> None:
    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(encoding="utf-8", errors="backslashreplace")
    parser = argparse.ArgumentParser()
    parser.add_argument("binary", type=Path)
    parser.add_argument("--types", default="^$", help="regular expression for DWARF type names")
    parser.add_argument("--functions", default="^$", help="regular expression for symbol names")
    parser.add_argument("--disasm", action="store_true")
    parser.add_argument("--near-calls", help="only show windows around calls whose symbol matches this regex")
    parser.add_argument("--from-address", type=lambda value: int(value, 0))
    parser.add_argument("--to-address", type=lambda value: int(value, 0))
    args = parser.parse_args()
    raw = args.binary.read_bytes()
    with args.binary.open("rb") as stream:
        elf = ELFFile(stream)
        dwarf = elf.get_dwarf_info()
        print_types(dwarf, re.compile(args.types, re.I))
        near_calls = re.compile(args.near_calls, re.I) if args.near_calls else None
        print_functions(
            elf,
            dwarf,
            raw,
            re.compile(args.functions, re.I),
            args.disasm,
            near_calls,
            args.from_address,
            args.to_address,
        )


if __name__ == "__main__":
    main()
