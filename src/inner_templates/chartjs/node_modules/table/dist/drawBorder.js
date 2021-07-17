"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.drawBorderTop = exports.drawBorderJoin = exports.drawBorderBottom = exports.drawBorder = void 0;
const drawContent_1 = require("./drawContent");
const drawBorder = (columnWidths, config) => {
    const { separator, drawVerticalLine } = config;
    const columns = columnWidths.map((size) => {
        return config.separator.body.repeat(size);
    });
    return drawContent_1.drawContent(columns, {
        drawSeparator: drawVerticalLine,
        endSeparator: separator.right,
        middleSeparator: separator.join,
        startSeparator: separator.left,
    }) + '\n';
};
exports.drawBorder = drawBorder;
const drawBorderTop = (columnWidths, config) => {
    const result = drawBorder(columnWidths, {
        ...config,
        separator: {
            body: config.border.topBody,
            join: config.border.topJoin,
            left: config.border.topLeft,
            right: config.border.topRight,
        },
    });
    if (result === '\n') {
        return '';
    }
    return result;
};
exports.drawBorderTop = drawBorderTop;
const drawBorderJoin = (columnWidths, config) => {
    return drawBorder(columnWidths, {
        ...config,
        separator: {
            body: config.border.joinBody,
            join: config.border.joinJoin,
            left: config.border.joinLeft,
            right: config.border.joinRight,
        },
    });
};
exports.drawBorderJoin = drawBorderJoin;
const drawBorderBottom = (columnWidths, config) => {
    return drawBorder(columnWidths, {
        ...config,
        separator: {
            body: config.border.bottomBody,
            join: config.border.bottomJoin,
            left: config.border.bottomLeft,
            right: config.border.bottomRight,
        },
    });
};
exports.drawBorderBottom = drawBorderBottom;
