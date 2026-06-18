ZBASE32 = "ybndrfg8ejkmcpqxot1uwisza345h769"

def zbase32encode(value):
    encoded = ''

    while value >= 32:
        div, mod = divmod(value, 32)
        encoded = ZBASE32[mod] + encoded
        value = div

    return ZBASE32[value] + encoded

def zbase32decode(encoded):
    value = 0
    column_multiplier = 1

    for c in encoded[::-1]:
        try:
            column = ZBASE32.index(c)
        except ValueError:
            raise Exception(f'Invalid character {repr(c)} in {repr(encoded)}')

        value += column * column_multiplier
        column_multiplier *= 32

    return value
