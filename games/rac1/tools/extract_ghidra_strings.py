#!/usr/bin/env python3
"""
Extract defined strings and their cross-references from Ghidra via Ghidra MCP REST API.
Saves the extracted dataset to config/strings.json and config/strings.tsv.

Usage:
  python3 tools/extract_ghidra_strings.py [--url http://127.0.0.1:8089] [--token rac1-local]
"""
import argparse
import bisect
import json
import os
import sys
import urllib.parse
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def get_json(url: str, token: str, params: dict = None, post_data: dict = None):
    full_url = url
    if params:
        full_url += "?" + urllib.parse.urlencode(params)
    data_bytes = json.dumps(post_data).encode("utf-8") if post_data is not None else None
    headers = {
        "Authorization": f"Bearer {token}",
        "Content-Type": "application/json",
    }
    req = urllib.request.Request(full_url, data=data_bytes, headers=headers)
    with urllib.request.urlopen(req, timeout=30) as resp:
        return json.loads(resp.read().decode("utf-8"))


def normalize_addr(addr_str: str) -> str:
    """Normalize address to 8-character uppercase hex without 0x prefix."""
    addr_str = addr_str.strip().lower()
    if addr_str.startswith("0x"):
        addr_str = addr_str[2:]
    return f"{int(addr_str, 16):08X}"


def fetch_functions(base_url: str, token: str) -> list[tuple[int, str]]:
    """Return sorted list of (start_address, func_name)."""
    print("Fetching functions from Ghidra...")
    data = get_json(f"{base_url}/list_functions", token)
    funcs = []
    for item in data.get("functions", []):
        raw_addr = item.get("address", "")
        if not raw_addr:
            continue
        addr = int(raw_addr, 16)
        name = item.get("name", "")
        # Normalize FUN_XXXXXXXX to func_XXXXXXXX
        if name.startswith("FUN_"):
            name = f"func_{name[4:].upper()}"
        funcs.append((addr, name))
    funcs.sort(key=lambda x: x[0])
    print(f"Loaded {len(funcs)} functions.")
    return funcs


def find_containing_function(funcs: list[tuple[int, str]], addr: int) -> str | None:
    """Find the function containing or matching the given address."""
    idx = bisect.bisect_right(funcs, (addr, "zzzzz")) - 1
    if 0 <= idx < len(funcs):
        func_addr, func_name = funcs[idx]
        # In this game most functions are under 64KB
        if addr - func_addr < 0x10000:
            return func_name
    return None


def fetch_strings(base_url: str, token: str) -> list[dict]:
    """Fetch all defined strings from Ghidra."""
    print("Fetching defined strings from Ghidra...")
    all_strings = []
    offset = 0
    batch_size = 200
    while True:
        data = get_json(f"{base_url}/list_strings", token, params={"offset": offset, "limit": batch_size})
        batch = data.get("strings", [])
        if not batch:
            break
        all_strings.extend(batch)
        offset += len(batch)
        if offset >= data.get("total", 0):
            break
    print(f"Fetched {len(all_strings)} defined strings.")
    return all_strings


def fetch_xrefs(base_url: str, token: str, addresses: list[str]) -> dict[str, list[dict]]:
    """Batch fetch xrefs for addresses using /get_bulk_xrefs."""
    print("Fetching cross-references in batches...")
    results = {}
    batch_size = 150
    for i in range(0, len(addresses), batch_size):
        chunk = addresses[i : i + batch_size]
        payload = {"addresses": chunk}
        res = get_json(f"{base_url}/get_bulk_xrefs", token, post_data=payload)
        results.update(res)
    print("Finished fetching cross-references.")
    return results


def main():
    parser = argparse.ArgumentParser(description="Extract defined strings from Ghidra via MCP REST")
    parser.add_argument("--url", default="http://127.0.0.1:8089", help="Ghidra MCP REST URL")
    parser.add_argument("--token", default=os.environ.get("GHIDRA_MCP_AUTH_TOKEN", "rac1-local"), help="Auth token")
    parser.add_argument("--out-json", default=str(ROOT / "config/strings.json"), help="Output JSON path")
    parser.add_argument("--out-tsv", default=str(ROOT / "config/strings.tsv"), help="Output TSV path")
    args = parser.parse_args()

    # 1. Test connection
    try:
        req = urllib.request.Request(f"{args.url}/check_connection", headers={"Authorization": f"Bearer {args.token}"})
        with urllib.request.urlopen(req, timeout=5) as resp:
            resp.read()
    except Exception as e:
        print(f"Error connecting to Ghidra REST server at {args.url}: {e}", file=sys.stderr)
        print("Ensure the Ghidra MCP container is running: bash tools/docker/ghidra_mcp.sh start", file=sys.stderr)
        sys.exit(1)

    # 2. Fetch functions and strings
    funcs = fetch_functions(args.url, args.token)
    raw_strings = fetch_strings(args.url, args.token)

    # 3. Fetch xrefs
    hex_addrs = [f"0x{s['address'].lower()}" for s in raw_strings]
    xrefs_map = fetch_xrefs(args.url, args.token, hex_addrs)

    # 4. Process and structure data
    by_address = {}
    by_function = {}
    tsv_rows = []

    for s in raw_strings:
        addr_hex = normalize_addr(s["address"])
        addr_int = int(addr_hex, 16)
        sym_name = f"D_{addr_hex}"
        val = s.get("value", "")
        length = s.get("length", len(val))

        raw_key = f"0x{s['address'].lower()}"
        refs = xrefs_map.get(raw_key, [])

        processed_refs = []
        for r in refs:
            from_addr_str = r.get("from", "")
            if not from_addr_str:
                continue
            from_addr = int(from_addr_str, 16)
            func_name = find_containing_function(funcs, from_addr) or f"func_{from_addr:08X}"
            ref_type = r.get("type", "UNKNOWN")

            ref_entry = {
                "from_address": f"0x{from_addr:08X}",
                "from_function": func_name,
                "type": ref_type,
            }
            processed_refs.append(ref_entry)

            # Add to by_function
            if func_name not in by_function:
                by_function[func_name] = []
            by_function[func_name].append({
                "string_address": f"0x{addr_hex}",
                "symbol": sym_name,
                "value": val,
                "ref_address": f"0x{from_addr:08X}",
                "ref_type": ref_type,
            })

        string_entry = {
            "address": f"0x{addr_hex}",
            "symbol": sym_name,
            "value": val,
            "length": length,
            "xrefs": processed_refs,
        }
        by_address[addr_hex] = string_entry

        # TSV row: address, symbol, length, xref_count, sample_functions, value
        func_list = sorted(list({r["from_function"] for r in processed_refs}))
        sample_funcs = ",".join(func_list[:3])
        tsv_rows.append(f"0x{addr_hex}\t{sym_name}\t{length}\t{len(processed_refs)}\t{sample_funcs}\t{repr(val)}")

    # 5. Write outputs
    out_data = {
        "count": len(by_address),
        "strings_by_address": by_address,
        "strings_by_function": by_function,
    }

    out_json = Path(args.out_json)
    out_json.parent.mkdir(parents=True, exist_ok=True)
    out_json.write_text(json.dumps(out_data, indent=2, ensure_ascii=False) + "\n")
    print(f"Wrote JSON dataset to {out_json} ({len(by_address)} strings, {len(by_function)} functions).")

    out_tsv = Path(args.out_tsv)
    tsv_header = "# address\tsymbol\tlength\txref_count\tsample_functions\tvalue\n"
    out_tsv.write_text(tsv_header + "\n".join(tsv_rows) + "\n")
    print(f"Wrote TSV catalogue to {out_tsv}.")


if __name__ == "__main__":
    main()
