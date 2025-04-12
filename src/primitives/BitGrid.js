const { clampBits, intersects } = require("../primitives/Misc");

const bitRangeKey = Symbol();

/** @template {{ range: Rect, bitRange?: BitRange }} T */
class BitGrid {
	/**
	 * @param {Rect} range
	 */
	constructor(range) {
		this.range = range;

		/** @type {Set<T>[]} */
		this.tiles = [];
		for (let i = 0; i < 32 * 32; ++i) this.tiles.push(new Set());
	}

	/** @param {Rect} range */
	bitRange(range) {
		const leftmostBit = ((range.x - range.w) - (this.range.x - this.range.w)) / (this.range.w * 2) * 32;
		const rightmostBit = ((range.x + range.w) - (this.range.x - this.range.w)) / (this.range.w * 2) * 32;
		const topmostBit = ((range.y - range.h) - (this.range.y - this.range.h)) / (this.range.h * 2) * 32;
		const bottommostBit = ((range.y + range.h) - (this.range.y - this.range.h)) / (this.range.h * 2) * 32;

		return {
			leftmost: clampBits(Math.floor(leftmostBit)),
			rightmost: clampBits(Math.ceil(rightmostBit)),
			topmost: clampBits(Math.floor(topmostBit)),
			bottommost: clampBits(Math.ceil(bottommostBit))
		};
	}

	/** @param {T} item */
	insert(item) {
		const bitRange = this.bitRange(item.range);
		item[bitRangeKey] = bitRange;
		for (let x = bitRange.leftmost; x <= bitRange.rightmost; x++) {
			for (let y = bitRange.topmost; y <= bitRange.bottommost; y++) {
				this.tiles[y * 32 + x].add(item);
			}
		}
	}

	/** @param {T} item */
	update(item) {
		const bitRange = this.bitRange(item.range);
		const oldBitRange = item[bitRangeKey];
		item[bitRangeKey] = bitRange;

		// trim item from old tiles
		for (let x = oldBitRange.leftmost; x < bitRange.leftmost; x++) {
			for (let y = oldBitRange.topmost; y <= oldBitRange.bottommost; y++) {
				this.tiles[y * 32 + x].delete(item);
			}
		}
		for (let x = oldBitRange.rightmost; x > bitRange.rightmost; x--) {
			for (let y = oldBitRange.topmost; y <= oldBitRange.bottommost; y++) {
				this.tiles[y * 32 + x].delete(item);
			}
		}
		for (let y = oldBitRange.topmost; y < bitRange.topmost; y++) {
			for (let x = oldBitRange.leftmost; x <= oldBitRange.rightmost; x++) {
				this.tiles[y * 32 + x].delete(item);
			}
		}
		for (let y = oldBitRange.bottommost; y > bitRange.bottommost; y--) {
			for (let x = oldBitRange.leftmost; x <= oldBitRange.rightmost; x++) {
				this.tiles[y * 32 + x].delete(item);
			}
		}

		// add item to new tiles
		for (let x = bitRange.leftmost; x < oldBitRange.leftmost; x++) {
			for (let y = bitRange.topmost; y <= bitRange.bottommost; y++) {
				this.tiles[y * 32 + x].add(item);
			}
		}
		for (let x = bitRange.rightmost; x > oldBitRange.rightmost; x--) {
			for (let y = bitRange.topmost; y <= bitRange.bottommost; y++) {
				this.tiles[y * 32 + x].add(item);
			}
		}
		for (let y = bitRange.topmost; y < oldBitRange.topmost; y++) {
			for (let x = bitRange.leftmost; x <= bitRange.rightmost; x++) {
				this.tiles[y * 32 + x].add(item);
			}
		}
		for (let y = bitRange.bottommost; y > oldBitRange.bottommost; y--) {
			for (let x = bitRange.leftmost; x <= bitRange.rightmost; x++) {
				this.tiles[y * 32 + x].add(item);
			}
		}
	}

	/** @param {T} item */
	remove(item) {
		const { leftmost, rightmost, topmost, bottommost } = item[bitRangeKey];
		for (let x = leftmost; x <= rightmost; x++) {
			for (let y = topmost; y <= bottommost; y++) {
				this.tiles[y * 32 + x].delete(item);
			}
		}
		delete item[bitRangeKey];
	}

	/**
	 * @param {Range} bitRange
	 * @param {(item: T) => void} callback
	 */
	search(range, callback, fast) {
		const { leftmost, rightmost, topmost, bottommost } = this.bitRange(range);
		for (let x = leftmost; x <= rightmost; x++) {
			for (let y = topmost; y <= bottommost; y++) {
				for (const item of this.tiles[y * 32 + x]) {
					// don't process items more than once
					if (!item[bitRangeKey]) {
						// this happens very rarely, and i'm not sure why, but this is a temporary fix
						this.tiles[y * 32 + x].delete(item);
					}
					if ((leftmost <= item[bitRangeKey].leftmost && item[bitRangeKey].leftmost < x)
							|| (topmost <= item[bitRangeKey].topmost && item[bitRangeKey].topmost < y)) continue;
					if (fast || intersects(item.range, range)) callback(item);
				}
			}
		}
	}

	/**
	 * @param {Range} range
	 * @param {(item: T) => boolean} selector
	 */
	containsAny(range, selector) {
		const { leftmost, rightmost, topmost, bottommost } = this.bitRange(range);
		for (let x = leftmost; x <= rightmost; x++) {
			for (let y = topmost; y <= bottommost; y++) {
				for (const item of this.tiles[y * 32 + x]) {
					if (intersects(item.range, range) && (!selector || selector(item))) return true;
				}
			}
		}

		return false;
	}
}

module.exports = BitGrid;
