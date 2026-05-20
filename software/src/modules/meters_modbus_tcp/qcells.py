from copy import deepcopy
import solax

display_names = [
    ('QCells Hybrid Inverter', {
        'en': 'Q-Cells hybrid inverter (Q.HOME series)',
        'de': 'Q-Cells Hybrid-Wechselrichter (Q.HOME-Serie)',
    }),
    ('QCells String Inverter', {
        'en': 'Q-Cells string inverter (Q.VOLT series)',
        'de': 'Q-Cells String-Wechselrichter (Q.VOLT-Serie)',
    }),
]

table_prototypes = [
    ('QCells Hybrid Inverter', deepcopy(solax.table_prototypes[0][1])),
    ('QCells String Inverter', deepcopy(solax.table_prototypes[1][1])),
]

default_device_addresses = [
    ('QCells Hybrid Inverter', solax.default_device_addresses[0][1]),
    ('QCells String Inverter', solax.default_device_addresses[1][1]),
]

specs = deepcopy(solax.specs)

for spec in specs:
    spec['name'] = spec['name'].replace('Solax', 'QCells')
    spec['virtual_meter'] = (spec['virtual_meter'][0].replace('Solax', 'QCells'), spec['virtual_meter'][1])
