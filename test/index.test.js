"use strict";

const { test } = require("node:test");
const assert = require("node:assert");
const BoostSpatialIndex = require("..");

function index() {
  const bsi = new BoostSpatialIndex();

  bsi.addBoundingBox("box", 0, 0, 10, 10);
  bsi.addCircle("circle", 5, 5, 100000);
  bsi.addAnnulus("annulus", 5, 5, 500000, 1000);
  bsi.addPolygon("polygon", [
    [0, 0],
    [0, 20],
    [20, 20],
    [20, 0],
  ]);

  return bsi;
}

test("add* methods return undefined on success", () => {
  const bsi = new BoostSpatialIndex();

  assert.strictEqual(bsi.addBoundingBox("a", 0, 0, 1, 1), undefined);
  assert.strictEqual(bsi.addCircle("b", 0, 0, 10), undefined);
  assert.strictEqual(bsi.addAnnulus("c", 0, 0, 10, 1), undefined);
  assert.strictEqual(
    bsi.addPolygon("d", [
      [0, 0],
      [0, 1],
      [1, 1],
    ]),
    undefined,
  );
});

test("queryPoint returns the shapes covering a point", () => {
  const bsi = index();

  assert.deepStrictEqual(bsi.queryPoint(5, 5).sort(), [
    "box",
    "circle",
    "polygon",
  ]);
  assert.deepStrictEqual(bsi.queryPoint(15, 15), ["polygon"]);
  assert.deepStrictEqual(bsi.queryPoint(50, 50), []);
});

test("queryPoint excludes the inner disc of an annulus", () => {
  const bsi = new BoostSpatialIndex();

  bsi.addAnnulus("annulus", 0, 0, 500000, 100000);

  assert.deepStrictEqual(bsi.queryPoint(0, 0), []);
  assert.deepStrictEqual(bsi.queryPoint(0, 2), ["annulus"]);
});

test("queryIntersect returns the shapes intersecting a polygon", () => {
  const bsi = index();

  assert.deepStrictEqual(
    bsi
      .queryIntersect([
        [1, 1],
        [1, 2],
        [2, 2],
      ])
      .sort(),
    ["annulus", "box", "polygon"],
  );
  assert.deepStrictEqual(
    bsi.queryIntersect([
      [50, 50],
      [50, 51],
      [51, 51],
    ]),
    [],
  );
});

test("remove deletes a shape once", () => {
  const bsi = index();

  assert.strictEqual(bsi.remove("box"), true);
  assert.strictEqual(bsi.remove("box"), false);
  assert.deepStrictEqual(bsi.queryPoint(5, 5).sort(), ["circle", "polygon"]);
});

test("the native layer rejects invalid arguments with false or []", () => {
  const raw = new BoostSpatialIndex().spatialIndex;

  assert.strictEqual(raw.addBBox(1, 0, 0, 1, 1), false);
  assert.strictEqual(raw.addBBox("x", 0, "0", 1, 1), false);
  assert.strictEqual(raw.addPolygon("x", "nope"), false);
  assert.strictEqual(raw.remove(3), false);
  assert.deepStrictEqual(raw.queryPoint("a", 1), []);
  assert.deepStrictEqual(raw.queryIntersect({}), []);
});
