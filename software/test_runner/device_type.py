class DeviceType:
    def __init__(self, type_string: str):
        # Eltako's hostname prefix is wallbox. Translate to something sane.
        self._type = type_string if type_string is not 'wallbox' else 'eltako'

    def is_warp(self, version: int | list[int] | None = None) -> bool:
        if version is None:
            return self._type in ('warp', 'warp2', 'warp3', 'warp4')
        if isinstance(version, list):
            return any(self.is_warp(v) for v in version)
        if version == 1:
            return self._type == 'warp'
        return self._type == f'warp{version}'

    def is_eltako(self) -> bool:
        return self._type == 'eltako'

    def is_wem(self, version: int | list[int] | None = None) -> bool:
        if version is None:
            return self._type in ('wem', 'wem2')
        if isinstance(version, list):
            return any(self.is_wem(v) for v in version)
        if version == 1:
            return self._type == 'wem'
        return self._type == f'wem{version}'

    def __str__(self):  return self._type
    def __repr__(self): return f'DeviceType({self._type!r})'
    def __eq__(self, other):
        if isinstance(other, DeviceType): return self._type == other._type
        if isinstance(other, str):        return self._type == other
        return NotImplemented
    def __hash__(self): return hash(self._type)
