export class Cell {
	readonly id: number;
	readonly x: number;
	readonly y: number;
	readonly size: number;
	readonly color: number;
	readonly onRemoved(): void;
	readonly onSpawned(): void;
	readonly onTick(): void;
};

export class Virus extends Cell {
	readonly fedTimes: number;
	readonly splitAngle: number;
}
