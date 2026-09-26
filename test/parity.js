"use strict";
/* eslint-disable no-console */
// Usage: node test/parity.js <reference build dir> <candidate build dir> [ops]
// CI runs it against the last NAN release (1.4.0) to prove the N-API port
// returns exactly the same results.
// Differential test: run the same seeded sequence of operations against two
// builds of boost-geospatial-index and compare every return value.
const [oldPath, newPath, n = "20000"] = process.argv.slice(2);
const OPS = Number(n);

function run(modPath) {
  const B = require(require("path").resolve(modPath));
  let seed = 42;
  const rnd = () =>
    (seed = (seed * 1103515245 + 12345) % 2147483648) / 2147483648;
  const lat = () => rnd() * 170 - 85,
    lon = () => rnd() * 358 - 179;
  const poly = () => {
    const cl = lat(),
      cn = lon(),
      k = 3 + Math.floor(rnd() * 6),
      r = rnd() * 20,
      pts = [];
    for (let i = 0; i < k; i++) {
      const a = (2 * Math.PI * i) / k;
      pts.push([cl + r * Math.sin(a) * 0.5, cn + r * Math.cos(a)]);
    }
    return pts;
  };
  const b = new B(),
    raw = b.spatialIndex,
    out = [],
    ids = [];
  const safe = (f) => {
    try {
      return f();
    } catch (e) {
      return "THROW:" + e.message;
    }
  };
  for (let i = 0; i < OPS; i++) {
    const op = Math.floor(rnd() * 8),
      id =
        rnd() < 0.05 && ids.length
          ? ids[Math.floor(rnd() * ids.length)]
          : "id" + i;
    let r;
    switch (op) {
      case 0: {
        const a = lat(),
          c = lon();
        r = safe(() =>
          b.addBoundingBox(
            id,
            a,
            c,
            a + rnd() * 10 + 0.001,
            c + rnd() * 10 + 0.001,
          ),
        );
        ids.push(id);
        break;
      }
      case 1:
        r = safe(() => b.addCircle(id, lat(), lon(), rnd() * 2e6));
        ids.push(id);
        break;
      case 2: {
        const o = rnd() * 2e6 + 1;
        r = safe(() => b.addAnnulus(id, lat(), lon(), o, o * rnd()));
        ids.push(id);
        break;
      }
      case 3:
        r = safe(() => b.addPolygon(id, poly()));
        ids.push(id);
        break;
      case 4:
      case 5:
        r = safe(() => b.queryPoint(lat(), lon()));
        break;
      case 6:
        r = safe(() => b.queryIntersect(poly()));
        break;
      case 7:
        r = safe(() =>
          b.remove(ids.length ? ids[Math.floor(rnd() * ids.length)] : "none"),
        );
        break;
    }
    out.push(r);
  }
  // Native layer edge cases the JS wrapper lets through or bypasses
  out.push(
    safe(() => raw.addBBox(1, 0, 0, 1, 1)),
    safe(() => raw.addBBox("x", 0, "0", 1, 1)),
    safe(() => raw.addCircle("y")),
    safe(() => raw.addPolygon("z", "nope")),
    safe(() =>
      raw.addPolygon("w", [
        [0, 0],
        [null, ""],
        [1, 1],
      ]),
    ),
    safe(() => raw.queryPoint("a", 1)),
    safe(() => raw.queryIntersect({})),
    safe(() => raw.queryIntersect([])),
    safe(() => raw.remove(3)),
    safe(() => raw.queryPoint(0.5, 0.5)),
    safe(() => b.queryPoint(0, 0)),
  );
  return JSON.stringify(out);
}
const a = run(oldPath),
  b = run(newPath);
const matches = JSON.parse(a).reduce(
  (s, r) => s + (Array.isArray(r) ? r.length : 0),
  0,
);
if (a !== b) {
  const A = JSON.parse(a),
    Bv = JSON.parse(b),
    i = A.findIndex((x, k) => JSON.stringify(x) !== JSON.stringify(Bv[k]));
  console.log(
    `MISMATCH at op ${i}:\n old=${JSON.stringify(A[i])}\n new=${JSON.stringify(Bv[i])}`,
  );
  process.exit(1);
}
console.log(
  `PARITY OK: ${JSON.parse(a).length} results identical (${matches} ids returned by queries), node ${process.version} ${process.platform}-${process.arch}`,
);
