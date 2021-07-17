import type { DrawVerticalLine } from './types/api';
import type { BottomBorderConfig, JoinBorderConfig, TopBorderConfig } from './types/internal';
declare type Separator = {
    readonly left: string;
    readonly right: string;
    readonly body: string;
    readonly join: string;
};
declare const drawBorder: (columnWidths: number[], config: {
    separator: Separator;
    drawVerticalLine: DrawVerticalLine;
}) => string;
declare const drawBorderTop: (columnWidths: number[], config: {
    border: TopBorderConfig;
    drawVerticalLine: DrawVerticalLine;
}) => string;
declare const drawBorderJoin: (columnWidths: number[], config: {
    border: JoinBorderConfig;
    drawVerticalLine: DrawVerticalLine;
}) => string;
declare const drawBorderBottom: (columnWidths: number[], config: {
    border: BottomBorderConfig;
    drawVerticalLine: DrawVerticalLine;
}) => string;
export { drawBorder, drawBorderBottom, drawBorderJoin, drawBorderTop, };
