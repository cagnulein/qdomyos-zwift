"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.truncateTableData = void 0;
const lodash_truncate_1 = __importDefault(require("lodash.truncate"));
/**
 * @todo Make it work with ASCII content.
 */
const truncateTableData = (rows, config) => {
    return rows.map((cells) => {
        return cells.map((cell, cellIndex) => {
            return lodash_truncate_1.default(cell, {
                length: config.columns[cellIndex].truncate,
                omission: '…',
            });
        });
    });
};
exports.truncateTableData = truncateTableData;
