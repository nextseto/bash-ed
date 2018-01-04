# bash-ed

[![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](https://raw.githubusercontent.com/nextseto/bash-ed/master/LICENSE)

**bash-ed** is an alternative shell for UNIX-based systems. This shell implements the following:

1. Uses child process creation which allows the user to enter and execute commands

2. Stores and retrieves previous commands via `history` and `!<number>`

3. Keyboard shortcuts, like the `UP` and `DOWN` arrow keys, can navigate through previous commands in the `history`

4. Custom directory manipulation commands such as: `create`, `cd`, `rename/move`, `copy` and `delete`

5. Supports batch processing so that scripts that are compatible with bash-ed can be run. `#! bashed`

## Installation

### Requirements

- Tested with macOS Sierra/High Sierra (10.12+)
- Linux distros. Mainly tested with Ubuntu 16.04

### How to use

1. Clone the directory: `git clone `
2. Navigate to the source directory: `cd bash-ed/source`
3. Compile with `make`
4. Run with `./bashed`

## License

All **source code** in this repository is released under the MIT license. See LICENSE for details.
