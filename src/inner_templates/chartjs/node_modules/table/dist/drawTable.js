"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.drawTable = void 0;
const drawBorder_1 = require("./drawBorder");
const drawContent_1 = require("./drawContent");
const drawRow_1 = require("./drawRow");
const utils_1 = require("./utils");
const drawTable = (rows, columnWidths, rowHeights, config) => {
    const { drawHorizontalLine, singleLine, } = config;
    const contents = utils_1.groupBySizes(rows, rowHeights).map((group) => {
        return group.map((row) => {
            return drawRow_1.drawRow(row, config);
        }).join('');
    });
    return drawContent_1.drawContent(contents, {
        drawSeparator: (index, size) => {
            // Top/bottom border
            if (index === 0 || index === size) {
                return drawHorizontalLine(index, size);
            }
            return !singleLine && drawHorizontalLine(index, size);
        },
        endSeparator: drawBorder_1.drawBorderBottom(columnWidths, config),
        middleSeparator: drawBorder_1.drawBorderJoin(columnWidths, config),
        startSeparator: drawBorder_1.drawBorderTop(columnWidths, config),
    });
};
exports.drawTable = drawTable;
