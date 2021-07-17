declare type SeparatorConfig = {
    drawSeparator: (index: number, size: number) => boolean;
    startSeparator: string;
    middleSeparator: string;
    endSeparator: string;
};
/**
 * Shared function to draw horizontal borders, rows or the entire table
 */
export declare const drawContent: (contents: string[], separatorConfig: SeparatorConfig) => string;
export {};
