"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.mapDataUsingRowHeights = void 0;
const lodash_flatten_1 = __importDefault(require("lodash.flatten"));
const wrapCell_1 = require("./wrapCell");
const mapDataUsingRowHeights = (unmappedRows, rowHeights, config) => {
    const tableWidth = unmappedRows[0].length;
    const mappedRows = unmappedRows.map((unmappedRow, unmappedRowIndex) => {
        const outputRowHeight = rowHeights[unmappedRowIndex];
        const outputRow = Array.from({ length: outputRowHeight }, () => {
            return new Array(tableWidth).fill('');
        });
        // rowHeight
        //     [{row index within rowSaw; index2}]
        //     [{cell index within a virtual row; index1}]
        unmappedRow.forEach((cell, cellIndex) => {
            const cellLines = wrapCell_1.wrapCell(cell, config.columns[cellIndex].width, config.columns[cellIndex].wrapWord);
            cellLines.forEach((cellLine, cellLineIndex) => {
                outputRow[cellLineIndex][cellIndex] = cellLine;
            });
        });
        return outputRow;
    });
    return lodash_flatten_1.default(mappedRows);
};
exports.mapDataUsingRowHeights = mapDataUsingRowHeights;
