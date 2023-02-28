//APIPath:energy_manager/
export interface sdcard_state {
    sd_status: number,
    lfs_status: number,
    card_type: number,
    sector_count: number,
    sector_size: number,
    manufacturer_id: number,
    product_rev: number,
    product_name: string,
}
