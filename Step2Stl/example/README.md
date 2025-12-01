# Step2Stl Library Example

This is an example application that demonstrates how to use the Step2Stl library for converting STEP files to STL format.

## Prerequisites

1. **Step2Stl Library**: The Step2Stl library must be built first. Refer to the main project's build instructions.
2. **CMake**: Version 3.16 or higher.
3. **C++ Compiler**: Supporting C++17 or higher.

## Building the Example

1. **Create a build directory**:
   ```bash
   mkdir build
   cd build
   ```

2. **Run CMake**:
   ```bash
   cmake ..
   ```

3. **Build the project**:
   ```bash
   cmake --build . --config Release
   ```

## Using the Example

The example application is a command-line tool that converts a STEP file to STL format.

### Usage

```bash
step2stl_example.exe <input.step> <output.stl> [tolerance] [ascii]
```

### Arguments

- **`<input.step>`**: Path to the input STEP file (.step or .stp)
- **`<output.stl>`**: Path to the output STL file
- **`[tolerance]`**: Optional. Mesh tolerance in millimeters (default: 0.1)
- **`[ascii]`**: Optional. 1 for ASCII STL format, 0 for binary (default: 0)

### Examples

1. **Basic conversion** (default settings):
   ```bash
   step2stl_example.exe input.step output.stl
   ```

2. **Custom tolerance** (0.05 mm):
   ```bash
   step2stl_example.exe input.step output.stl 0.05
   ```

3. **ASCII format**: 
   ```bash
   step2stl_example.exe input.step output.stl 0.1 1
   ```

## Example Output

```
Step2Stl Library Example
=========================

Input file:    input.step
Output file:   output.stl
Mesh tolerance: 0.100 mm
STL format:    Binary

Converting...
Progress: 100%

Conversion result: SUCCESS
STEP file successfully converted to STL!

Press Enter to exit...
```

## Troubleshooting

1. **Library not found**: Ensure the Step2Stl library has been built and is located in the expected directory.
2. **DLL not found**: On Windows, make sure the Step2Stl.dll file is in the same directory as the executable or in the system PATH.
3. **Invalid STEP file**: Ensure the input file is a valid STEP file (.step or .stp).
4. **Permission denied**: Check if you have write permission for the output directory.

## License

Refer to the main Step2Stl library's license.
