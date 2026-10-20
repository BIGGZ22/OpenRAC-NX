// Numbers as the OpenRAC website writes them (openrac-site
// src/components/Progress.tsx): two decimals with a decimal comma, like
// decomp.dev's figures, and byte counts grouped with thin spaces so the
// groups cannot be mistaken for that comma.

/** 59.3217 -> "59,32 %" */
export function pct(p: number): string {
  return `${p.toFixed(2).replace(".", ",")} %`;
}

/** 2202320 -> "2 202 320" (narrow no-break spaces) */
export function grouped(n: number): string {
  return n.toLocaleString("en-US").replace(/,/g, " ");
}

/** A size in bytes, for people: "4,2 GB". */
export function size(bytes: number): string {
  const units = ["bytes", "KB", "MB", "GB", "TB"];
  let value = bytes;
  let unit = 0;
  while (value >= 1000 && unit < units.length - 1) {
    value /= 1000;
    unit++;
  }
  // Rounded before the unit is chosen: 999 999 bytes is 1000 KB, which is a megabyte.
  value = unit === 0 ? Math.round(value) : Math.round(value * 10) / 10;
  if (value >= 1000 && unit < units.length - 1) {
    value /= 1000;
    unit++;
  }
  const text = unit === 0 ? grouped(value) : value.toFixed(value < 10 ? 1 : 0).replace(".", ",");
  return `${text} ${units[unit] ?? ""}`;
}

/** Milliseconds as "4 s", "2 min 05 s", "1 h 03 min". */
export function elapsed(ms: number): string {
  const s = Math.max(0, Math.round(ms / 1000));
  if (s < 60) return `${s} s`;
  const m = Math.floor(s / 60);
  if (m < 60) return `${m} min ${String(s % 60).padStart(2, "0")} s`;
  return `${Math.floor(m / 60)} h ${String(m % 60).padStart(2, "0")} min`;
}

/** The progress line under a bar, as on the site's cards. */
export function matchedLine(matched: number, total: number): string {
  return `${grouped(matched)} of ${grouped(total)} code bytes matched`;
}
