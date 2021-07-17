"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.padTableData = void 0;
const padTableData = (rows, config) => {
    return rows.map((cells) => {
        return cells.map((cell, cellIndex) => {
            const column = config.columns[cellIndex];
            return ' '.repeat(column.paddingLeft) + cell + ' '.repeat(column.paddingRight);
        });
    });
};
exports.padTableData = padTableData;
