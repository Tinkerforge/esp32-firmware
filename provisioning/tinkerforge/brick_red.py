# -*- coding: utf-8 -*-
#############################################################
# This file was automatically generated on 2024-02-27.      #
#                                                           #
# Python Bindings Version 2.1.31                            #
#                                                           #
# If you have a bugfix for this file and want to commit it, #
# please fix the bug in the generator. You can find a link  #
# to the generators git repository on tinkerforge.com       #
#############################################################

from collections import namedtuple

try:
    from .ip_connection import Device, IPConnection, Error, create_char, create_char_list, create_string, create_chunk_data
except (ValueError, ImportError):
    try:
        from ip_connection import Device, IPConnection, Error, create_char, create_char_list, create_string, create_chunk_data
    except (ValueError, ImportError):
        from tinkerforge.ip_connection import Device, IPConnection, Error, create_char, create_char_list, create_string, create_chunk_data

CreateSession = namedtuple('CreateSession', ['error_code', 'session_id'])
AllocateString = namedtuple('AllocateString', ['error_code', 'string_id'])
GetStringLength = namedtuple('StringLength', ['error_code', 'length'])
GetStringChunk = namedtuple('StringChunk', ['error_code', 'buffer'])
AllocateList = namedtuple('AllocateList', ['error_code', 'list_id'])
GetListLength = namedtuple('ListLength', ['error_code', 'length'])
GetListItem = namedtuple('ListItem', ['error_code', 'item_object_id', 'type'])
OpenFile = namedtuple('OpenFile', ['error_code', 'file_id'])
CreatePipe = namedtuple('CreatePipe', ['error_code', 'file_id'])
GetFileInfo = namedtuple('FileInfo', ['error_code', 'type', 'name_string_id', 'flags', 'permissions', 'uid', 'gid', 'length', 'access_timestamp', 'modification_timestamp', 'status_change_timestamp'])
ReadFile = namedtuple('ReadFile', ['error_code', 'buffer', 'length_read'])
WriteFile = namedtuple('WriteFile', ['error_code', 'length_written'])
SetFilePosition = namedtuple('SetFilePosition', ['error_code', 'position'])
GetFilePosition = namedtuple('FilePosition', ['error_code', 'position'])
GetFileEvents = namedtuple('FileEvents', ['error_code', 'events'])
OpenDirectory = namedtuple('OpenDirectory', ['error_code', 'directory_id'])
GetDirectoryName = namedtuple('DirectoryName', ['error_code', 'name_string_id'])
GetNextDirectoryEntry = namedtuple('NextDirectoryEntry', ['error_code', 'name_string_id', 'type'])
GetProcesses = namedtuple('Processes', ['error_code', 'processes_list_id'])
SpawnProcess = namedtuple('SpawnProcess', ['error_code', 'process_id'])
GetProcessCommand = namedtuple('ProcessCommand', ['error_code', 'executable_string_id', 'arguments_list_id', 'environment_list_id', 'working_directory_string_id'])
GetProcessIdentity = namedtuple('ProcessIdentity', ['error_code', 'pid', 'uid', 'gid'])
GetProcessStdio = namedtuple('ProcessStdio', ['error_code', 'stdin_file_id', 'stdout_file_id', 'stderr_file_id'])
GetProcessState = namedtuple('ProcessState', ['error_code', 'state', 'timestamp', 'exit_code'])
GetPrograms = namedtuple('Programs', ['error_code', 'programs_list_id'])
DefineProgram = namedtuple('DefineProgram', ['error_code', 'program_id'])
GetProgramIdentifier = namedtuple('ProgramIdentifier', ['error_code', 'identifier_string_id'])
GetProgramRootDirectory = namedtuple('ProgramRootDirectory', ['error_code', 'root_directory_string_id'])
GetProgramCommand = namedtuple('ProgramCommand', ['error_code', 'executable_string_id', 'arguments_list_id', 'environment_list_id', 'working_directory_string_id'])
GetProgramStdioRedirection = namedtuple('ProgramStdioRedirection', ['error_code', 'stdin_redirection', 'stdin_file_name_string_id', 'stdout_redirection', 'stdout_file_name_string_id', 'stderr_redirection', 'stderr_file_name_string_id'])
GetProgramSchedule = namedtuple('ProgramSchedule', ['error_code', 'start_mode', 'continue_after_error', 'start_interval', 'start_fields_string_id'])
GetProgramSchedulerState = namedtuple('ProgramSchedulerState', ['error_code', 'state', 'timestamp', 'message_string_id'])
GetLastSpawnedProgramProcess = namedtuple('LastSpawnedProgramProcess', ['error_code', 'process_id', 'timestamp'])
GetCustomProgramOptionNames = namedtuple('CustomProgramOptionNames', ['error_code', 'names_list_id'])
GetCustomProgramOptionValue = namedtuple('CustomProgramOptionValue', ['error_code', 'value_string_id'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])

class BrickRED(Device):
    r"""
    Executes user programs and controls other Bricks/Bricklets standalone
    """

    DEVICE_IDENTIFIER = 17
    DEVICE_DISPLAY_NAME = 'RED Brick'
    DEVICE_URL_PART = 'red' # internal

    CALLBACK_ASYNC_FILE_READ = 30
    CALLBACK_ASYNC_FILE_WRITE = 31
    CALLBACK_FILE_EVENTS_OCCURRED = 32
    CALLBACK_PROCESS_STATE_CHANGED = 45
    CALLBACK_PROGRAM_SCHEDULER_STATE_CHANGED = 65
    CALLBACK_PROGRAM_PROCESS_SPAWNED = 66


    FUNCTION_CREATE_SESSION = 1
    FUNCTION_EXPIRE_SESSION = 2
    FUNCTION_EXPIRE_SESSION_UNCHECKED = 3
    FUNCTION_KEEP_SESSION_ALIVE = 4
    FUNCTION_RELEASE_OBJECT = 5
    FUNCTION_RELEASE_OBJECT_UNCHECKED = 6
    FUNCTION_ALLOCATE_STRING = 7
    FUNCTION_TRUNCATE_STRING = 8
    FUNCTION_GET_STRING_LENGTH = 9
    FUNCTION_SET_STRING_CHUNK = 10
    FUNCTION_GET_STRING_CHUNK = 11
    FUNCTION_ALLOCATE_LIST = 12
    FUNCTION_GET_LIST_LENGTH = 13
    FUNCTION_GET_LIST_ITEM = 14
    FUNCTION_APPEND_TO_LIST = 15
    FUNCTION_REMOVE_FROM_LIST = 16
    FUNCTION_OPEN_FILE = 17
    FUNCTION_CREATE_PIPE = 18
    FUNCTION_GET_FILE_INFO = 19
    FUNCTION_READ_FILE = 20
    FUNCTION_READ_FILE_ASYNC = 21
    FUNCTION_ABORT_ASYNC_FILE_READ = 22
    FUNCTION_WRITE_FILE = 23
    FUNCTION_WRITE_FILE_UNCHECKED = 24
    FUNCTION_WRITE_FILE_ASYNC = 25
    FUNCTION_SET_FILE_POSITION = 26
    FUNCTION_GET_FILE_POSITION = 27
    FUNCTION_SET_FILE_EVENTS = 28
    FUNCTION_GET_FILE_EVENTS = 29
    FUNCTION_OPEN_DIRECTORY = 33
    FUNCTION_GET_DIRECTORY_NAME = 34
    FUNCTION_GET_NEXT_DIRECTORY_ENTRY = 35
    FUNCTION_REWIND_DIRECTORY = 36
    FUNCTION_CREATE_DIRECTORY = 37
    FUNCTION_GET_PROCESSES = 38
    FUNCTION_SPAWN_PROCESS = 39
    FUNCTION_KILL_PROCESS = 40
    FUNCTION_GET_PROCESS_COMMAND = 41
    FUNCTION_GET_PROCESS_IDENTITY = 42
    FUNCTION_GET_PROCESS_STDIO = 43
    FUNCTION_GET_PROCESS_STATE = 44
    FUNCTION_GET_PROGRAMS = 46
    FUNCTION_DEFINE_PROGRAM = 47
    FUNCTION_PURGE_PROGRAM = 48
    FUNCTION_GET_PROGRAM_IDENTIFIER = 49
    FUNCTION_GET_PROGRAM_ROOT_DIRECTORY = 50
    FUNCTION_SET_PROGRAM_COMMAND = 51
    FUNCTION_GET_PROGRAM_COMMAND = 52
    FUNCTION_SET_PROGRAM_STDIO_REDIRECTION = 53
    FUNCTION_GET_PROGRAM_STDIO_REDIRECTION = 54
    FUNCTION_SET_PROGRAM_SCHEDULE = 55
    FUNCTION_GET_PROGRAM_SCHEDULE = 56
    FUNCTION_GET_PROGRAM_SCHEDULER_STATE = 57
    FUNCTION_CONTINUE_PROGRAM_SCHEDULE = 58
    FUNCTION_START_PROGRAM = 59
    FUNCTION_GET_LAST_SPAWNED_PROGRAM_PROCESS = 60
    FUNCTION_GET_CUSTOM_PROGRAM_OPTION_NAMES = 61
    FUNCTION_SET_CUSTOM_PROGRAM_OPTION_VALUE = 62
    FUNCTION_GET_CUSTOM_PROGRAM_OPTION_VALUE = 63
    FUNCTION_REMOVE_CUSTOM_PROGRAM_OPTION = 64
    FUNCTION_GET_IDENTITY = 255

    ERROR_CODE_SUCCESS = 0
    ERROR_CODE_UNKNOWN_ERROR = 1
    ERROR_CODE_INVALID_OPERATION = 2
    ERROR_CODE_OPERATION_ABORTED = 3
    ERROR_CODE_INTERNAL_ERROR = 4
    ERROR_CODE_UNKNOWN_SESSION_ID = 5
    ERROR_CODE_NO_FREE_SESSION_ID = 6
    ERROR_CODE_UNKNOWN_OBJECT_ID = 7
    ERROR_CODE_NO_FREE_OBJECT_ID = 8
    ERROR_CODE_OBJECT_IS_LOCKED = 9
    ERROR_CODE_NO_MORE_DATA = 10
    ERROR_CODE_WRONG_LIST_ITEM_TYPE = 11
    ERROR_CODE_PROGRAM_IS_PURGED = 12
    ERROR_CODE_INVALID_PARAMETER = 128
    ERROR_CODE_NO_FREE_MEMORY = 129
    ERROR_CODE_NO_FREE_SPACE = 130
    ERROR_CODE_ACCESS_DENIED = 121
    ERROR_CODE_ALREADY_EXISTS = 132
    ERROR_CODE_DOES_NOT_EXIST = 133
    ERROR_CODE_INTERRUPTED = 134
    ERROR_CODE_IS_DIRECTORY = 135
    ERROR_CODE_NOT_A_DIRECTORY = 136
    ERROR_CODE_WOULD_BLOCK = 137
    ERROR_CODE_OVERFLOW = 138
    ERROR_CODE_BAD_FILE_DESCRIPTOR = 139
    ERROR_CODE_OUT_OF_RANGE = 140
    ERROR_CODE_NAME_TOO_LONG = 141
    ERROR_CODE_INVALID_SEEK = 142
    ERROR_CODE_NOT_SUPPORTED = 143
    ERROR_CODE_TOO_MANY_OPEN_FILES = 144
    OBJECT_TYPE_STRING = 0
    OBJECT_TYPE_LIST = 1
    OBJECT_TYPE_FILE = 2
    OBJECT_TYPE_DIRECTORY = 3
    OBJECT_TYPE_PROCESS = 4
    OBJECT_TYPE_PROGRAM = 5
    FILE_FLAG_READ_ONLY = 1
    FILE_FLAG_WRITE_ONLY = 2
    FILE_FLAG_READ_WRITE = 4
    FILE_FLAG_APPEND = 8
    FILE_FLAG_CREATE = 16
    FILE_FLAG_EXCLUSIVE = 32
    FILE_FLAG_NON_BLOCKING = 64
    FILE_FLAG_TRUNCATE = 128
    FILE_FLAG_TEMPORARY = 256
    FILE_FLAG_REPLACE = 512
    FILE_PERMISSION_USER_ALL = 448
    FILE_PERMISSION_USER_READ = 256
    FILE_PERMISSION_USER_WRITE = 128
    FILE_PERMISSION_USER_EXECUTE = 64
    FILE_PERMISSION_GROUP_ALL = 56
    FILE_PERMISSION_GROUP_READ = 32
    FILE_PERMISSION_GROUP_WRITE = 16
    FILE_PERMISSION_GROUP_EXECUTE = 8
    FILE_PERMISSION_OTHERS_ALL = 7
    FILE_PERMISSION_OTHERS_READ = 4
    FILE_PERMISSION_OTHERS_WRITE = 2
    FILE_PERMISSION_OTHERS_EXECUTE = 1
    PIPE_FLAG_NON_BLOCKING_READ = 1
    PIPE_FLAG_NON_BLOCKING_WRITE = 2
    FILE_TYPE_UNKNOWN = 0
    FILE_TYPE_REGULAR = 1
    FILE_TYPE_DIRECTORY = 2
    FILE_TYPE_CHARACTER = 3
    FILE_TYPE_BLOCK = 4
    FILE_TYPE_FIFO = 5
    FILE_TYPE_SYMLINK = 6
    FILE_TYPE_SOCKET = 7
    FILE_TYPE_PIPE = 8
    FILE_ORIGIN_BEGINNING = 0
    FILE_ORIGIN_CURRENT = 1
    FILE_ORIGIN_END = 2
    FILE_EVENT_READABLE = 1
    FILE_EVENT_WRITABLE = 2
    DIRECTORY_ENTRY_TYPE_UNKNOWN = 0
    DIRECTORY_ENTRY_TYPE_REGULAR = 1
    DIRECTORY_ENTRY_TYPE_DIRECTORY = 2
    DIRECTORY_ENTRY_TYPE_CHARACTER = 3
    DIRECTORY_ENTRY_TYPE_BLOCK = 4
    DIRECTORY_ENTRY_TYPE_FIFO = 5
    DIRECTORY_ENTRY_TYPE_SYMLINK = 6
    DIRECTORY_ENTRY_TYPE_SOCKET = 7
    DIRECTORY_FLAG_RECURSIVE = 1
    DIRECTORY_FLAG_EXCLUSIVE = 2
    PROCESS_SIGNAL_INTERRUPT = 2
    PROCESS_SIGNAL_QUIT = 3
    PROCESS_SIGNAL_ABORT = 6
    PROCESS_SIGNAL_KILL = 9
    PROCESS_SIGNAL_USER1 = 10
    PROCESS_SIGNAL_USER2 = 12
    PROCESS_SIGNAL_TERMINATE = 15
    PROCESS_SIGNAL_CONTINUE = 18
    PROCESS_SIGNAL_STOP = 19
    PROCESS_STATE_UNKNOWN = 0
    PROCESS_STATE_RUNNING = 1
    PROCESS_STATE_ERROR = 2
    PROCESS_STATE_EXITED = 3
    PROCESS_STATE_KILLED = 4
    PROCESS_STATE_STOPPED = 5
    PROGRAM_STDIO_REDIRECTION_DEV_NULL = 0
    PROGRAM_STDIO_REDIRECTION_PIPE = 1
    PROGRAM_STDIO_REDIRECTION_FILE = 2
    PROGRAM_STDIO_REDIRECTION_INDIVIDUAL_LOG = 3
    PROGRAM_STDIO_REDIRECTION_CONTINUOUS_LOG = 4
    PROGRAM_STDIO_REDIRECTION_STDOUT = 5
    PROGRAM_START_MODE_NEVER = 0
    PROGRAM_START_MODE_ALWAYS = 1
    PROGRAM_START_MODE_INTERVAL = 2
    PROGRAM_START_MODE_CRON = 3
    PROGRAM_SCHEDULER_STATE_STOPPED = 0
    PROGRAM_SCHEDULER_STATE_RUNNING = 1

    def __init__(self, uid, ipcon):
        r"""
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon, BrickRED.DEVICE_IDENTIFIER, BrickRED.DEVICE_DISPLAY_NAME)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickRED.FUNCTION_CREATE_SESSION] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_EXPIRE_SESSION] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_EXPIRE_SESSION_UNCHECKED] = BrickRED.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickRED.FUNCTION_KEEP_SESSION_ALIVE] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_RELEASE_OBJECT] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_RELEASE_OBJECT_UNCHECKED] = BrickRED.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickRED.FUNCTION_ALLOCATE_STRING] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_TRUNCATE_STRING] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_GET_STRING_LENGTH] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_SET_STRING_CHUNK] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_GET_STRING_CHUNK] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_ALLOCATE_LIST] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_GET_LIST_LENGTH] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_GET_LIST_ITEM] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_APPEND_TO_LIST] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_REMOVE_FROM_LIST] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_OPEN_FILE] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_CREATE_PIPE] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_GET_FILE_INFO] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_READ_FILE] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_READ_FILE_ASYNC] = BrickRED.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickRED.FUNCTION_ABORT_ASYNC_FILE_READ] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_WRITE_FILE] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_WRITE_FILE_UNCHECKED] = BrickRED.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickRED.FUNCTION_WRITE_FILE_ASYNC] = BrickRED.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickRED.FUNCTION_SET_FILE_POSITION] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_GET_FILE_POSITION] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_SET_FILE_EVENTS] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_GET_FILE_EVENTS] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_OPEN_DIRECTORY] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_GET_DIRECTORY_NAME] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_GET_NEXT_DIRECTORY_ENTRY] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_REWIND_DIRECTORY] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_CREATE_DIRECTORY] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_GET_PROCESSES] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_SPAWN_PROCESS] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_KILL_PROCESS] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_GET_PROCESS_COMMAND] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_GET_PROCESS_IDENTITY] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_GET_PROCESS_STDIO] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_GET_PROCESS_STATE] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_GET_PROGRAMS] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_DEFINE_PROGRAM] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_PURGE_PROGRAM] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_GET_PROGRAM_IDENTIFIER] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_GET_PROGRAM_ROOT_DIRECTORY] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_SET_PROGRAM_COMMAND] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_GET_PROGRAM_COMMAND] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_SET_PROGRAM_STDIO_REDIRECTION] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_GET_PROGRAM_STDIO_REDIRECTION] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_SET_PROGRAM_SCHEDULE] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_GET_PROGRAM_SCHEDULE] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_GET_PROGRAM_SCHEDULER_STATE] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_CONTINUE_PROGRAM_SCHEDULE] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_START_PROGRAM] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_GET_LAST_SPAWNED_PROGRAM_PROCESS] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_GET_CUSTOM_PROGRAM_OPTION_NAMES] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_SET_CUSTOM_PROGRAM_OPTION_VALUE] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_GET_CUSTOM_PROGRAM_OPTION_VALUE] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_REMOVE_CUSTOM_PROGRAM_OPTION] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickRED.FUNCTION_GET_IDENTITY] = BrickRED.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickRED.CALLBACK_ASYNC_FILE_READ] = (72, 'H B 60B B')
        self.callback_formats[BrickRED.CALLBACK_ASYNC_FILE_WRITE] = (12, 'H B B')
        self.callback_formats[BrickRED.CALLBACK_FILE_EVENTS_OCCURRED] = (12, 'H H')
        self.callback_formats[BrickRED.CALLBACK_PROCESS_STATE_CHANGED] = (20, 'H B Q B')
        self.callback_formats[BrickRED.CALLBACK_PROGRAM_SCHEDULER_STATE_CHANGED] = (10, 'H')
        self.callback_formats[BrickRED.CALLBACK_PROGRAM_PROCESS_SPAWNED] = (10, 'H')

        ipcon.add_device(self)

    def create_session(self, lifetime):
        r"""

        """
        self.check_validity()

        lifetime = int(lifetime)

        return CreateSession(*self.ipcon.send_request(self, BrickRED.FUNCTION_CREATE_SESSION, (lifetime,), 'I', 11, 'B H'))

    def expire_session(self, session_id):
        r"""

        """
        self.check_validity()

        session_id = int(session_id)

        return self.ipcon.send_request(self, BrickRED.FUNCTION_EXPIRE_SESSION, (session_id,), 'H', 9, 'B')

    def expire_session_unchecked(self, session_id):
        r"""

        """
        self.check_validity()

        session_id = int(session_id)

        self.ipcon.send_request(self, BrickRED.FUNCTION_EXPIRE_SESSION_UNCHECKED, (session_id,), 'H', 0, '')

    def keep_session_alive(self, session_id, lifetime):
        r"""

        """
        self.check_validity()

        session_id = int(session_id)
        lifetime = int(lifetime)

        return self.ipcon.send_request(self, BrickRED.FUNCTION_KEEP_SESSION_ALIVE, (session_id, lifetime), 'H I', 9, 'B')

    def release_object(self, object_id, session_id):
        r"""
        Decreases the reference count of an object by one and returns the resulting
        error code. If the reference count reaches zero the object gets destroyed.
        """
        self.check_validity()

        object_id = int(object_id)
        session_id = int(session_id)

        return self.ipcon.send_request(self, BrickRED.FUNCTION_RELEASE_OBJECT, (object_id, session_id), 'H H', 9, 'B')

    def release_object_unchecked(self, object_id, session_id):
        r"""

        """
        self.check_validity()

        object_id = int(object_id)
        session_id = int(session_id)

        self.ipcon.send_request(self, BrickRED.FUNCTION_RELEASE_OBJECT_UNCHECKED, (object_id, session_id), 'H H', 0, '')

    def allocate_string(self, length_to_reserve, buffer, session_id):
        r"""
        Allocates a new string object, reserves ``length_to_reserve`` bytes memory
        for it and sets up to the first 60 bytes. Set ``length_to_reserve`` to the
        length of the string that should be stored in the string object.

        Returns the object ID of the new string object and the resulting error code.
        """
        self.check_validity()

        length_to_reserve = int(length_to_reserve)
        buffer = create_string(buffer)
        session_id = int(session_id)

        return AllocateString(*self.ipcon.send_request(self, BrickRED.FUNCTION_ALLOCATE_STRING, (length_to_reserve, buffer, session_id), 'I 58s H', 11, 'B H'))

    def truncate_string(self, string_id, length):
        r"""
        Truncates a string object to ``length`` bytes and returns the resulting
        error code.
        """
        self.check_validity()

        string_id = int(string_id)
        length = int(length)

        return self.ipcon.send_request(self, BrickRED.FUNCTION_TRUNCATE_STRING, (string_id, length), 'H I', 9, 'B')

    def get_string_length(self, string_id):
        r"""
        Returns the length of a string object and the resulting error code.
        """
        self.check_validity()

        string_id = int(string_id)

        return GetStringLength(*self.ipcon.send_request(self, BrickRED.FUNCTION_GET_STRING_LENGTH, (string_id,), 'H', 13, 'B I'))

    def set_string_chunk(self, string_id, offset, buffer):
        r"""
        Sets a chunk of up to 58 bytes in a string object beginning at ``offset``.

        Returns the resulting error code.
        """
        self.check_validity()

        string_id = int(string_id)
        offset = int(offset)
        buffer = create_string(buffer)

        return self.ipcon.send_request(self, BrickRED.FUNCTION_SET_STRING_CHUNK, (string_id, offset, buffer), 'H I 58s', 9, 'B')

    def get_string_chunk(self, string_id, offset):
        r"""
        Returns a chunk up to 63 bytes from a string object beginning at ``offset`` and
        returns the resulting error code.
        """
        self.check_validity()

        string_id = int(string_id)
        offset = int(offset)

        return GetStringChunk(*self.ipcon.send_request(self, BrickRED.FUNCTION_GET_STRING_CHUNK, (string_id, offset), 'H I', 72, 'B 63s'))

    def allocate_list(self, length_to_reserve, session_id):
        r"""
        Allocates a new list object and reserves memory for ``length_to_reserve``
        items. Set ``length_to_reserve`` to the number of items that should be stored
        in the list object.

        Returns the object ID of the new list object and the resulting error code.

        When a list object gets destroyed then the reference count of each object in
        the list object is decreased by one.
        """
        self.check_validity()

        length_to_reserve = int(length_to_reserve)
        session_id = int(session_id)

        return AllocateList(*self.ipcon.send_request(self, BrickRED.FUNCTION_ALLOCATE_LIST, (length_to_reserve, session_id), 'H H', 11, 'B H'))

    def get_list_length(self, list_id):
        r"""
        Returns the length of a list object in items and the resulting error code.
        """
        self.check_validity()

        list_id = int(list_id)

        return GetListLength(*self.ipcon.send_request(self, BrickRED.FUNCTION_GET_LIST_LENGTH, (list_id,), 'H', 11, 'B H'))

    def get_list_item(self, list_id, index, session_id):
        r"""
        Returns the object ID and type of the object stored at ``index`` in a list
        object and returns the resulting error code.

        Possible object types are:

        * String = 0
        * List = 1
        * File = 2
        * Directory = 3
        * Process = 4
        * Program = 5
        """
        self.check_validity()

        list_id = int(list_id)
        index = int(index)
        session_id = int(session_id)

        return GetListItem(*self.ipcon.send_request(self, BrickRED.FUNCTION_GET_LIST_ITEM, (list_id, index, session_id), 'H H H', 12, 'B H B'))

    def append_to_list(self, list_id, item_object_id):
        r"""
        Appends an object to a list object and increases the reference count of the
        appended object by one.

        Returns the resulting error code.
        """
        self.check_validity()

        list_id = int(list_id)
        item_object_id = int(item_object_id)

        return self.ipcon.send_request(self, BrickRED.FUNCTION_APPEND_TO_LIST, (list_id, item_object_id), 'H H', 9, 'B')

    def remove_from_list(self, list_id, index):
        r"""
        Removes the object stored at ``index`` from a list object and decreases the
        reference count of the removed object by one.

        Returns the resulting error code.
        """
        self.check_validity()

        list_id = int(list_id)
        index = int(index)

        return self.ipcon.send_request(self, BrickRED.FUNCTION_REMOVE_FROM_LIST, (list_id, index), 'H H', 9, 'B')

    def open_file(self, name_string_id, flags, permissions, uid, gid, session_id):
        r"""
        Opens an existing file or creates a new file and allocates a new file object
        for it.

        FIXME: name has to be absolute

        The reference count of the name string object is increased by one. When the
        file object gets destroyed then the reference count of the name string object is
        decreased by one. Also the name string object is locked and cannot be modified
        while the file object holds a reference to it.

        The ``flags`` parameter takes a ORed combination of the following possible file
        flags (in hexadecimal notation):

        * ReadOnly = 0x0001 (O_RDONLY)
        * WriteOnly = 0x0002 (O_WRONLY)
        * ReadWrite = 0x0004 (O_RDWR)
        * Append = 0x0008 (O_APPEND)
        * Create = 0x0010 (O_CREAT)
        * Exclusive = 0x0020 (O_EXCL)
        * NonBlocking = 0x0040 (O_NONBLOCK)
        * Truncate = 0x0080 (O_TRUNC)
        * Temporary = 0x0100
        * Replace = 0x0200

        FIXME: explain *Temporary* and *Replace* flag

        The ``permissions`` parameter takes a ORed combination of the following
        possible file permissions (in octal notation) that match the common UNIX
        permission bits:

        * UserRead = 00400
        * UserWrite = 00200
        * UserExecute = 00100
        * GroupRead = 00040
        * GroupWrite = 00020
        * GroupExecute = 00010
        * OthersRead = 00004
        * OthersWrite = 00002
        * OthersExecute = 00001

        Returns the object ID of the new file object and the resulting error code.
        """
        self.check_validity()

        name_string_id = int(name_string_id)
        flags = int(flags)
        permissions = int(permissions)
        uid = int(uid)
        gid = int(gid)
        session_id = int(session_id)

        return OpenFile(*self.ipcon.send_request(self, BrickRED.FUNCTION_OPEN_FILE, (name_string_id, flags, permissions, uid, gid, session_id), 'H I H I I H', 11, 'B H'))

    def create_pipe(self, flags, length, session_id):
        r"""
        Creates a new pipe and allocates a new file object for it.

        The ``flags`` parameter takes a ORed combination of the following possible
        pipe flags (in hexadecimal notation):

        * NonBlockingRead = 0x0001
        * NonBlockingWrite = 0x0002

        The length of the pipe buffer can be specified with the ``length`` parameter
        in bytes. If length is set to zero, then the default pipe buffer length is used.

        Returns the object ID of the new file object and the resulting error code.
        """
        self.check_validity()

        flags = int(flags)
        length = int(length)
        session_id = int(session_id)

        return CreatePipe(*self.ipcon.send_request(self, BrickRED.FUNCTION_CREATE_PIPE, (flags, length, session_id), 'I Q H', 11, 'B H'))

    def get_file_info(self, file_id, session_id):
        r"""
        Returns various information about a file and the resulting error code.

        Possible file types are:

        * Unknown = 0
        * Regular = 1
        * Directory = 2
        * Character = 3
        * Block = 4
        * FIFO = 5
        * Symlink = 6
        * Socket = 7
        * Pipe = 8

        If the file type is *Pipe* then the returned name string object is invalid,
        because a pipe has no name. Otherwise the returned name string object was used
        to open or create the file object, as passed to :func:`Open File`.

        The returned flags were used to open or create the file object, as passed to
        :func:`Open File` or :func:`Create Pipe`. See the respective function for a list
        of possible file and pipe flags.

        FIXME: everything except flags and length is invalid if file type is *Pipe*
        """
        self.check_validity()

        file_id = int(file_id)
        session_id = int(session_id)

        return GetFileInfo(*self.ipcon.send_request(self, BrickRED.FUNCTION_GET_FILE_INFO, (file_id, session_id), 'H H', 58, 'B B H I H I I Q Q Q Q'))

    def read_file(self, file_id, length_to_read):
        r"""
        Reads up to 62 bytes from a file object.

        Returns the bytes read, the actual number of bytes read and the resulting
        error code.

        If there is not data to be read, either because the file position reached
        end-of-file or because there is not data in the pipe, then zero bytes are
        returned.

        If the file object was created by :func:`Open File` without the *NonBlocking*
        flag or by :func:`Create Pipe` without the *NonBlockingRead* flag then the
        error code *NotSupported* is returned.
        """
        self.check_validity()

        file_id = int(file_id)
        length_to_read = int(length_to_read)

        return ReadFile(*self.ipcon.send_request(self, BrickRED.FUNCTION_READ_FILE, (file_id, length_to_read), 'H B', 72, 'B 62B B'))

    def read_file_async(self, file_id, length_to_read):
        r"""
        Reads up to 2\ :sup:`63`\  - 1 bytes from a file object asynchronously.

        Reports the bytes read (in 60 byte chunks), the actual number of bytes read and
        the resulting error code via the :cb:`Async File Read` callback.

        If there is not data to be read, either because the file position reached
        end-of-file or because there is not data in the pipe, then zero bytes are
        reported.

        If the file object was created by :func:`Open File` without the *NonBlocking*
        flag or by :func:`Create Pipe` without the *NonBlockingRead* flag then the error
        code *NotSupported* is reported via the :cb:`Async File Read` callback.
        """
        self.check_validity()

        file_id = int(file_id)
        length_to_read = int(length_to_read)

        self.ipcon.send_request(self, BrickRED.FUNCTION_READ_FILE_ASYNC, (file_id, length_to_read), 'H Q', 0, '')

    def abort_async_file_read(self, file_id):
        r"""
        Aborts a :func:`Read File Async` operation in progress.

        Returns the resulting error code.

        On success the :cb:`Async File Read` callback will report *OperationAborted*.
        """
        self.check_validity()

        file_id = int(file_id)

        return self.ipcon.send_request(self, BrickRED.FUNCTION_ABORT_ASYNC_FILE_READ, (file_id,), 'H', 9, 'B')

    def write_file(self, file_id, buffer, length_to_write):
        r"""
        Writes up to 61 bytes to a file object.

        Returns the actual number of bytes written and the resulting error code.

        If the file object was created by :func:`Open File` without the *NonBlocking*
        flag or by :func:`Create Pipe` without the *NonBlockingWrite* flag then the
        error code *NotSupported* is returned.
        """
        self.check_validity()

        file_id = int(file_id)
        buffer = list(map(int, buffer))
        length_to_write = int(length_to_write)

        return WriteFile(*self.ipcon.send_request(self, BrickRED.FUNCTION_WRITE_FILE, (file_id, buffer, length_to_write), 'H 61B B', 10, 'B B'))

    def write_file_unchecked(self, file_id, buffer, length_to_write):
        r"""
        Writes up to 61 bytes to a file object.

        Does neither report the actual number of bytes written nor the resulting error
        code.

        If the file object was created by :func:`Open File` without the *NonBlocking*
        flag or by :func:`Create Pipe` without the *NonBlockingWrite* flag then the
        write operation will fail silently.
        """
        self.check_validity()

        file_id = int(file_id)
        buffer = list(map(int, buffer))
        length_to_write = int(length_to_write)

        self.ipcon.send_request(self, BrickRED.FUNCTION_WRITE_FILE_UNCHECKED, (file_id, buffer, length_to_write), 'H 61B B', 0, '')

    def write_file_async(self, file_id, buffer, length_to_write):
        r"""
        Writes up to 61 bytes to a file object.

        Reports the actual number of bytes written and the resulting error code via the
        :cb:`Async File Write` callback.

        If the file object was created by :func:`Open File` without the *NonBlocking*
        flag or by :func:`Create Pipe` without the *NonBlockingWrite* flag then the
        error code *NotSupported* is reported via the :cb:`Async File Write` callback.
        """
        self.check_validity()

        file_id = int(file_id)
        buffer = list(map(int, buffer))
        length_to_write = int(length_to_write)

        self.ipcon.send_request(self, BrickRED.FUNCTION_WRITE_FILE_ASYNC, (file_id, buffer, length_to_write), 'H 61B B', 0, '')

    def set_file_position(self, file_id, offset, origin):
        r"""
        Set the current seek position of a file object relative to ``origin``.

        Possible file origins are:

        * Beginning = 0
        * Current = 1
        * End = 2

        Returns the resulting absolute seek position and error code.

        If the file object was created by :func:`Create Pipe` then it has no seek
        position and the error code *InvalidSeek* is returned.
        """
        self.check_validity()

        file_id = int(file_id)
        offset = int(offset)
        origin = int(origin)

        return SetFilePosition(*self.ipcon.send_request(self, BrickRED.FUNCTION_SET_FILE_POSITION, (file_id, offset, origin), 'H q B', 17, 'B Q'))

    def get_file_position(self, file_id):
        r"""
        Returns the current seek position of a file object and returns the
        resulting error code.

        If the file object was created by :func:`Create Pipe` then it has no seek
        position and the error code *InvalidSeek* is returned.
        """
        self.check_validity()

        file_id = int(file_id)

        return GetFilePosition(*self.ipcon.send_request(self, BrickRED.FUNCTION_GET_FILE_POSITION, (file_id,), 'H', 17, 'B Q'))

    def set_file_events(self, file_id, events):
        r"""

        """
        self.check_validity()

        file_id = int(file_id)
        events = int(events)

        return self.ipcon.send_request(self, BrickRED.FUNCTION_SET_FILE_EVENTS, (file_id, events), 'H H', 9, 'B')

    def get_file_events(self, file_id):
        r"""

        """
        self.check_validity()

        file_id = int(file_id)

        return GetFileEvents(*self.ipcon.send_request(self, BrickRED.FUNCTION_GET_FILE_EVENTS, (file_id,), 'H', 11, 'B H'))

    def open_directory(self, name_string_id, session_id):
        r"""
        Opens an existing directory and allocates a new directory object for it.

        FIXME: name has to be absolute

        The reference count of the name string object is increased by one. When the
        directory object is destroyed then the reference count of the name string
        object is decreased by one. Also the name string object is locked and cannot be
        modified while the directory object holds a reference to it.

        Returns the object ID of the new directory object and the resulting error code.
        """
        self.check_validity()

        name_string_id = int(name_string_id)
        session_id = int(session_id)

        return OpenDirectory(*self.ipcon.send_request(self, BrickRED.FUNCTION_OPEN_DIRECTORY, (name_string_id, session_id), 'H H', 11, 'B H'))

    def get_directory_name(self, directory_id, session_id):
        r"""
        Returns the name of a directory object, as passed to :func:`Open Directory`, and
        the resulting error code.
        """
        self.check_validity()

        directory_id = int(directory_id)
        session_id = int(session_id)

        return GetDirectoryName(*self.ipcon.send_request(self, BrickRED.FUNCTION_GET_DIRECTORY_NAME, (directory_id, session_id), 'H H', 11, 'B H'))

    def get_next_directory_entry(self, directory_id, session_id):
        r"""
        Returns the next entry in a directory object and the resulting error code.

        If there is not next entry then error code *NoMoreData* is returned. To rewind
        a directory object call :func:`Rewind Directory`.

        Possible directory entry types are:

        * Unknown = 0
        * Regular = 1
        * Directory = 2
        * Character = 3
        * Block = 4
        * FIFO = 5
        * Symlink = 6
        * Socket = 7
        """
        self.check_validity()

        directory_id = int(directory_id)
        session_id = int(session_id)

        return GetNextDirectoryEntry(*self.ipcon.send_request(self, BrickRED.FUNCTION_GET_NEXT_DIRECTORY_ENTRY, (directory_id, session_id), 'H H', 12, 'B H B'))

    def rewind_directory(self, directory_id):
        r"""
        Rewinds a directory object and returns the resulting error code.
        """
        self.check_validity()

        directory_id = int(directory_id)

        return self.ipcon.send_request(self, BrickRED.FUNCTION_REWIND_DIRECTORY, (directory_id,), 'H', 9, 'B')

    def create_directory(self, name_string_id, flags, permissions, uid, gid):
        r"""
        FIXME: name has to be absolute
        """
        self.check_validity()

        name_string_id = int(name_string_id)
        flags = int(flags)
        permissions = int(permissions)
        uid = int(uid)
        gid = int(gid)

        return self.ipcon.send_request(self, BrickRED.FUNCTION_CREATE_DIRECTORY, (name_string_id, flags, permissions, uid, gid), 'H I H I I', 9, 'B')

    def get_processes(self, session_id):
        r"""

        """
        self.check_validity()

        session_id = int(session_id)

        return GetProcesses(*self.ipcon.send_request(self, BrickRED.FUNCTION_GET_PROCESSES, (session_id,), 'H', 11, 'B H'))

    def spawn_process(self, executable_string_id, arguments_list_id, environment_list_id, working_directory_string_id, uid, gid, stdin_file_id, stdout_file_id, stderr_file_id, session_id):
        r"""

        """
        self.check_validity()

        executable_string_id = int(executable_string_id)
        arguments_list_id = int(arguments_list_id)
        environment_list_id = int(environment_list_id)
        working_directory_string_id = int(working_directory_string_id)
        uid = int(uid)
        gid = int(gid)
        stdin_file_id = int(stdin_file_id)
        stdout_file_id = int(stdout_file_id)
        stderr_file_id = int(stderr_file_id)
        session_id = int(session_id)

        return SpawnProcess(*self.ipcon.send_request(self, BrickRED.FUNCTION_SPAWN_PROCESS, (executable_string_id, arguments_list_id, environment_list_id, working_directory_string_id, uid, gid, stdin_file_id, stdout_file_id, stderr_file_id, session_id), 'H H H H I I H H H H', 11, 'B H'))

    def kill_process(self, process_id, signal):
        r"""
        Sends a UNIX signal to a process object and returns the resulting error code.

        Possible UNIX signals are:

        * Interrupt = 2
        * Quit = 3
        * Abort = 6
        * Kill = 9
        * User1 = 10
        * User2 = 12
        * Terminate = 15
        * Continue =  18
        * Stop = 19
        """
        self.check_validity()

        process_id = int(process_id)
        signal = int(signal)

        return self.ipcon.send_request(self, BrickRED.FUNCTION_KILL_PROCESS, (process_id, signal), 'H B', 9, 'B')

    def get_process_command(self, process_id, session_id):
        r"""
        Returns the executable, arguments, environment and working directory used to
        spawn a process object, as passed to :func:`Spawn Process`, and the resulting
        error code.
        """
        self.check_validity()

        process_id = int(process_id)
        session_id = int(session_id)

        return GetProcessCommand(*self.ipcon.send_request(self, BrickRED.FUNCTION_GET_PROCESS_COMMAND, (process_id, session_id), 'H H', 17, 'B H H H H'))

    def get_process_identity(self, process_id):
        r"""
        Returns the process ID and the user and group ID used to spawn a process object,
        as passed to :func:`Spawn Process`, and the resulting error code.

        The process ID is only valid if the state is *Running* or *Stopped*, see
        :func:`Get Process State`.
        """
        self.check_validity()

        process_id = int(process_id)

        return GetProcessIdentity(*self.ipcon.send_request(self, BrickRED.FUNCTION_GET_PROCESS_IDENTITY, (process_id,), 'H', 21, 'B I I I'))

    def get_process_stdio(self, process_id, session_id):
        r"""
        Returns the stdin, stdout and stderr files used to spawn a process object, as
        passed to :func:`Spawn Process`, and the resulting error code.
        """
        self.check_validity()

        process_id = int(process_id)
        session_id = int(session_id)

        return GetProcessStdio(*self.ipcon.send_request(self, BrickRED.FUNCTION_GET_PROCESS_STDIO, (process_id, session_id), 'H H', 15, 'B H H H'))

    def get_process_state(self, process_id):
        r"""
        Returns the current state, timestamp and exit code of a process object, and
        the resulting error code.

        Possible process states are:

        * Unknown = 0
        * Running = 1
        * Error = 2
        * Exited = 3
        * Killed = 4
        * Stopped = 5

        The timestamp represents the UNIX time since when the process is in its current
        state.

        The exit code is only valid if the state is *Error*, *Exited*, *Killed* or
        *Stopped* and has different meanings depending on the state:

        * Error: error code for error occurred while spawning the process (see below)
        * Exited: exit status of the process
        * Killed: UNIX signal number used to kill the process
        * Stopped: UNIX signal number used to stop the process

        Possible exit/error codes in *Error* state are:

        * InternalError = 125
        * CannotExecute = 126
        * DoesNotExist = 127

        The *CannotExecute* error can be caused by the executable being opened for
        writing.
        """
        self.check_validity()

        process_id = int(process_id)

        return GetProcessState(*self.ipcon.send_request(self, BrickRED.FUNCTION_GET_PROCESS_STATE, (process_id,), 'H', 19, 'B B Q B'))

    def get_programs(self, session_id):
        r"""

        """
        self.check_validity()

        session_id = int(session_id)

        return GetPrograms(*self.ipcon.send_request(self, BrickRED.FUNCTION_GET_PROGRAMS, (session_id,), 'H', 11, 'B H'))

    def define_program(self, identifier_string_id, session_id):
        r"""

        """
        self.check_validity()

        identifier_string_id = int(identifier_string_id)
        session_id = int(session_id)

        return DefineProgram(*self.ipcon.send_request(self, BrickRED.FUNCTION_DEFINE_PROGRAM, (identifier_string_id, session_id), 'H H', 11, 'B H'))

    def purge_program(self, program_id, cookie):
        r"""

        """
        self.check_validity()

        program_id = int(program_id)
        cookie = int(cookie)

        return self.ipcon.send_request(self, BrickRED.FUNCTION_PURGE_PROGRAM, (program_id, cookie), 'H I', 9, 'B')

    def get_program_identifier(self, program_id, session_id):
        r"""

        """
        self.check_validity()

        program_id = int(program_id)
        session_id = int(session_id)

        return GetProgramIdentifier(*self.ipcon.send_request(self, BrickRED.FUNCTION_GET_PROGRAM_IDENTIFIER, (program_id, session_id), 'H H', 11, 'B H'))

    def get_program_root_directory(self, program_id, session_id):
        r"""
        FIXME: root directory is absolute: <home>/programs/<identifier>
        """
        self.check_validity()

        program_id = int(program_id)
        session_id = int(session_id)

        return GetProgramRootDirectory(*self.ipcon.send_request(self, BrickRED.FUNCTION_GET_PROGRAM_ROOT_DIRECTORY, (program_id, session_id), 'H H', 11, 'B H'))

    def set_program_command(self, program_id, executable_string_id, arguments_list_id, environment_list_id, working_directory_string_id):
        r"""
        FIXME: working directory is relative to <home>/programs/<identifier>/bin
        """
        self.check_validity()

        program_id = int(program_id)
        executable_string_id = int(executable_string_id)
        arguments_list_id = int(arguments_list_id)
        environment_list_id = int(environment_list_id)
        working_directory_string_id = int(working_directory_string_id)

        return self.ipcon.send_request(self, BrickRED.FUNCTION_SET_PROGRAM_COMMAND, (program_id, executable_string_id, arguments_list_id, environment_list_id, working_directory_string_id), 'H H H H H', 9, 'B')

    def get_program_command(self, program_id, session_id):
        r"""
        FIXME: working directory is relative to <home>/programs/<identifier>/bin
        """
        self.check_validity()

        program_id = int(program_id)
        session_id = int(session_id)

        return GetProgramCommand(*self.ipcon.send_request(self, BrickRED.FUNCTION_GET_PROGRAM_COMMAND, (program_id, session_id), 'H H', 17, 'B H H H H'))

    def set_program_stdio_redirection(self, program_id, stdin_redirection, stdin_file_name_string_id, stdout_redirection, stdout_file_name_string_id, stderr_redirection, stderr_file_name_string_id):
        r"""
        FIXME: stdio file names are relative to <home>/programs/<identifier>/bin
        """
        self.check_validity()

        program_id = int(program_id)
        stdin_redirection = int(stdin_redirection)
        stdin_file_name_string_id = int(stdin_file_name_string_id)
        stdout_redirection = int(stdout_redirection)
        stdout_file_name_string_id = int(stdout_file_name_string_id)
        stderr_redirection = int(stderr_redirection)
        stderr_file_name_string_id = int(stderr_file_name_string_id)

        return self.ipcon.send_request(self, BrickRED.FUNCTION_SET_PROGRAM_STDIO_REDIRECTION, (program_id, stdin_redirection, stdin_file_name_string_id, stdout_redirection, stdout_file_name_string_id, stderr_redirection, stderr_file_name_string_id), 'H B H B H B H', 9, 'B')

    def get_program_stdio_redirection(self, program_id, session_id):
        r"""
        FIXME: stdio file names are relative to <home>/programs/<identifier>/bin
        """
        self.check_validity()

        program_id = int(program_id)
        session_id = int(session_id)

        return GetProgramStdioRedirection(*self.ipcon.send_request(self, BrickRED.FUNCTION_GET_PROGRAM_STDIO_REDIRECTION, (program_id, session_id), 'H H', 18, 'B B H B H B H'))

    def set_program_schedule(self, program_id, start_mode, continue_after_error, start_interval, start_fields_string_id):
        r"""

        """
        self.check_validity()

        program_id = int(program_id)
        start_mode = int(start_mode)
        continue_after_error = bool(continue_after_error)
        start_interval = int(start_interval)
        start_fields_string_id = int(start_fields_string_id)

        return self.ipcon.send_request(self, BrickRED.FUNCTION_SET_PROGRAM_SCHEDULE, (program_id, start_mode, continue_after_error, start_interval, start_fields_string_id), 'H B ! I H', 9, 'B')

    def get_program_schedule(self, program_id, session_id):
        r"""

        """
        self.check_validity()

        program_id = int(program_id)
        session_id = int(session_id)

        return GetProgramSchedule(*self.ipcon.send_request(self, BrickRED.FUNCTION_GET_PROGRAM_SCHEDULE, (program_id, session_id), 'H H', 17, 'B B ! I H'))

    def get_program_scheduler_state(self, program_id, session_id):
        r"""
        FIXME: message is currently valid in error-occurred state only
        """
        self.check_validity()

        program_id = int(program_id)
        session_id = int(session_id)

        return GetProgramSchedulerState(*self.ipcon.send_request(self, BrickRED.FUNCTION_GET_PROGRAM_SCHEDULER_STATE, (program_id, session_id), 'H H', 20, 'B B Q H'))

    def continue_program_schedule(self, program_id):
        r"""

        """
        self.check_validity()

        program_id = int(program_id)

        return self.ipcon.send_request(self, BrickRED.FUNCTION_CONTINUE_PROGRAM_SCHEDULE, (program_id,), 'H', 9, 'B')

    def start_program(self, program_id):
        r"""

        """
        self.check_validity()

        program_id = int(program_id)

        return self.ipcon.send_request(self, BrickRED.FUNCTION_START_PROGRAM, (program_id,), 'H', 9, 'B')

    def get_last_spawned_program_process(self, program_id, session_id):
        r"""

        """
        self.check_validity()

        program_id = int(program_id)
        session_id = int(session_id)

        return GetLastSpawnedProgramProcess(*self.ipcon.send_request(self, BrickRED.FUNCTION_GET_LAST_SPAWNED_PROGRAM_PROCESS, (program_id, session_id), 'H H', 19, 'B H Q'))

    def get_custom_program_option_names(self, program_id, session_id):
        r"""

        """
        self.check_validity()

        program_id = int(program_id)
        session_id = int(session_id)

        return GetCustomProgramOptionNames(*self.ipcon.send_request(self, BrickRED.FUNCTION_GET_CUSTOM_PROGRAM_OPTION_NAMES, (program_id, session_id), 'H H', 11, 'B H'))

    def set_custom_program_option_value(self, program_id, name_string_id, value_string_id):
        r"""

        """
        self.check_validity()

        program_id = int(program_id)
        name_string_id = int(name_string_id)
        value_string_id = int(value_string_id)

        return self.ipcon.send_request(self, BrickRED.FUNCTION_SET_CUSTOM_PROGRAM_OPTION_VALUE, (program_id, name_string_id, value_string_id), 'H H H', 9, 'B')

    def get_custom_program_option_value(self, program_id, name_string_id, session_id):
        r"""

        """
        self.check_validity()

        program_id = int(program_id)
        name_string_id = int(name_string_id)
        session_id = int(session_id)

        return GetCustomProgramOptionValue(*self.ipcon.send_request(self, BrickRED.FUNCTION_GET_CUSTOM_PROGRAM_OPTION_VALUE, (program_id, name_string_id, session_id), 'H H H', 11, 'B H'))

    def remove_custom_program_option(self, program_id, name_string_id):
        r"""

        """
        self.check_validity()

        program_id = int(program_id)
        name_string_id = int(name_string_id)

        return self.ipcon.send_request(self, BrickRED.FUNCTION_REMOVE_CUSTOM_PROGRAM_OPTION, (program_id, name_string_id), 'H H', 9, 'B')

    def get_identity(self):
        r"""
        Returns the UID, the UID where the Brick is connected to,
        the position, the hardware and firmware version as well as the
        device identifier.

        The position is the position in the stack from '0' (bottom) to '8' (top).

        The device identifier numbers can be found :ref:`here <device_identifier>`.
        |device_identifier_constant|
        """
        return GetIdentity(*self.ipcon.send_request(self, BrickRED.FUNCTION_GET_IDENTITY, (), '', 33, '8s 8s c 3B 3B H'))

    def register_callback(self, callback_id, function):
        r"""
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

RED = BrickRED # for backward compatibility
