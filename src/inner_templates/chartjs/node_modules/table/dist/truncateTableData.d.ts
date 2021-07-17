import type { BaseConfig, Row } from './types/internal';
/**
 * @todo Make it work with ASCII content.
 */
export declare const truncateTableData: (rows: Row[], config: BaseConfig) => Row[];
