from copy import deepcopy
import alpha_ess

display_names = [
    ('Hailei Hybrid Inverter', {
        'en': 'Hailei hybrid inverter (TPH series)',
        'de': 'Hailei Hybrid-Wechselrichter (TPH-Serie)',
    }),
]

table_prototypes = [
    ('Hailei Hybrid Inverter', deepcopy(alpha_ess.table_prototypes[0][1])),
]

default_device_addresses = [
    ('Hailei Hybrid Inverter', alpha_ess.default_device_addresses[0][1]),
]

repeat_intervals = [
    ('Hailei Hybrid Inverter', alpha_ess.repeat_intervals[0][1]),
]

specs = deepcopy(alpha_ess.specs)

for spec in specs:
    spec['group'] = spec['group'].replace('Alpha ESS', 'Hailei')
