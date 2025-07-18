interface spi_state {
    clk: number;
    dummy_cycles: number;
    spi_mode: string;
}

export interface state_static {
    heap_dram: number;
    heap_iram: number;
    heap_psram: number;
    psram_size: number;
    ipsock_max: number;
    cpu_clk: number;
    apb_clk: number;
    spi_buses: spi_state[];
    dram_benchmark: number;
    iram_benchmark: number;
    psram_benchmark: number;
    rodata_benchmark: number;
    text_benchmark: number;
}

export interface state_fast {
    uptime: number;
    free_dram: number;
    free_iram: number;
    free_psram: number;
    heap_check_time_avg: number;
    heap_check_time_max: number;
    cpu_usage: number;
}

export interface state_slow {
    largest_free_dram_block: number;
    largest_free_iram_block: number;
    largest_free_psram_block: number;
    heap_integrity_ok: boolean;
    loop_max_us: number;
    loop_max_fn_file: string;
    loop_max_fn_line: number;
    min_free_dram: number;
    min_free_iram: number;
    min_free_psram: number;
    conf_uint_buf_size: number;
    conf_int_buf_size: number;
    conf_float_buf_size: number;
    conf_string_buf_size: number;
    conf_array_buf_size: number;
    conf_object_buf_size: number;
    conf_union_buf_size: number;
    conf_uint53_buf_size: number;
    conf_int52_buf_size: number;
    ipsock_cur: number;
    ipsock_hwm: number;
}

type slot_info = number[];

export type state_slots = slot_info[];

interface task_hwm {
    task_name: string;
    hwm: number;
    stack_size: number;
}

export type state_hwm = task_hwm[];
