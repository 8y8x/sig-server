const { clampBits } = require("../primitives/Misc");

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
			rightmost: clampBits(Math.floor(rightmostBit)),
			topmost: clampBits(Math.floor(topmostBit)),
			bottommost: clampBits(Math.floor(bottommostBit))
		};
	}

	/** @param {T} item */
	insert(item) {
		const bitRange = this.bitRange(item.range);
		item.bitRange = bitRange;
		for (let x = bitRange.leftmost; x <= bitRange.rightmost; x++) {
			for (let y = bitRange.topmost; y <= bitRange.bottommost; y++) {
				this.tiles[y * 32 + x].add(item);
			}
		}
	}

	/** @param {T} item */
	update(item) {
		const bitRange = this.bitRange(item.range);
		const oldBitRange = item.bitRange;
		item.bitRange = bitRange;

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
			for (let x = oldBitRange.leftmost; x <= oldBitRange.rightmost; ++x) {
				this.tiles[y * 32 + x].delete(item);
			}
		}
		for (let y = oldBitRange.bottommost; y > bitRange.bottommost; y--) {
			for (let x = oldBitRange.leftmost; x <= oldBitRange.rightmost; ++x) {
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
		const { leftmost, rightmost, topmost, bottommost } = item.bitRange;
		for (let x = leftmost; x <= rightmost; x++) {
			for (let y = topmost; y <= bottommost; y++) {
				this.tiles[y * 32 + x].delete(item);
			}
		}
	}

	/**
	 * @param {BitRange} bitRange
	 * @param {(item: T) => void} callback
	 */
	search(bitRange, callback) {
		const { leftmost, rightmost, topmost, bottommost } = bitRange;
		for (let x = leftmost; x <= rightmost; x++) {
			for (let y = topmost; y <= bottommost; y++) {
				this.tiles[y * 32 + x].forEach(item => {
					// don't process items more than once
					if ((leftmost <= item.bitRange.leftmost && item.bitRange.leftmost < x)
							|| (topmost <= item.bitRange.topmost && item.bitRange.topmost < y)) return;
					callback(item);
				});
			}
		}
	}

	/**
	 * @param {BitRange} bitRange
	 * @param {(item: T) => boolean} selector
	 */
	containsAny(bitRange, selector) {
		const { leftmost, rightmost, topmost, bottommost } = bitRange;
		for (let x = leftmost; x <= rightmost; x++) {
			for (let y = topmost; y <= bottommost; y++) {
				for (const item of this.tiles[y * 32 + x].values()) {
					if (selector(item)) return true;
				}
			}
		}

		return false;
	}
}

module.exports = BitGrid;
