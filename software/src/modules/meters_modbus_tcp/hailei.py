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

specs = deepcopy(alpha_ess.specs)

for spec in specs:
    spec['name'] = spec['name'].replace('Alpha ESS', 'Hailei')
    spec['virtual_meter'] = (spec['virtual_meter'][0].replace('Alpha ESS', 'Hailei'), spec['virtual_meter'][1])
