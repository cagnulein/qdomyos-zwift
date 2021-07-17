"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.drawRow = void 0;
const drawContent_1 = require("./drawContent");
const drawRow = (row, config) => {
    const { border, drawVerticalLine } = config;
    return drawContent_1.drawContent(row, {
        drawSeparator: drawVerticalLine,
        endSeparator: border.bodyRight,
        middleSeparator: border.bodyJoin,
        startSeparator: border.bodyLeft,
    }) + '\n';
};
exports.drawRow = drawRow;
