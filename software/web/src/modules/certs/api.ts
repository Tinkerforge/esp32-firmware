type Cert = {
    id: number;
    name: string;
    size: number;
};

export interface state {
    certs: Cert[];
}

export interface add {
    id: number;
    name: string;
    cert: string;
}

export type modify = add;

export interface remove {
    id: number;
}
