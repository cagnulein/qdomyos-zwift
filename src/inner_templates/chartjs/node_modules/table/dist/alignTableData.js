"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.alignTableData = void 0;
const string_width_1 = __importDefault(require("string-width"));
const alignString_1 = require("./alignString");
const alignTableData = (rows, config) => {
    return rows.map((row) => {
        return row.map((cell, cellIndex) => {
            const column = config.columns[cellIndex];
            if (string_width_1.default(cell) === column.width) {
                return cell;
            }
            else {
                return alignString_1.alignString(cell, column.width, column.alignment);
            }
        });
    });
};
exports.alignTableData = alignTableData;
