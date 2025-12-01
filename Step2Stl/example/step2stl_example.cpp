#include <Step2Stl.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Progress callback function
 * @param progress Progress percentage (0-100)
 * @param userData User data passed to the callback
 */
void progressCallback(int progress, void* userData)
{
    printf("Progress: %d%%\r", progress);
    fflush(stdout);
}

int main(int argc, char* argv[])
{
    printf("Step2Stl Library Example\n");
    printf("=========================\n\n");
    
    // Check command line arguments
    if (argc < 3) {
        printf("Usage: %s <input.step> <output.stl> [tolerance] [ascii]
", argv[0]);
        printf("  <input.step>  - Input STEP file path (.step or .stp)\n");
        printf("  <output.stl> - Output STL file path\n");
        printf("  [tolerance]  - Optional: Mesh tolerance in millimeters (default: 0.1)\n");
        printf("  [ascii]      - Optional: 1 for ASCII STL format, 0 for binary (default: 0)\n");
        return 1;
    }
    
    const char* inputPath = argv[1];
    const char* outputPath = argv[2];
    double tolerance = 0.1; // Default tolerance: 0.1 mm
    int useAscii = 0;        // Default format: binary
    
    // Parse optional arguments
    if (argc > 3) {
        tolerance = atof(argv[3]);
        if (tolerance <= 0.0) {
            printf("Warning: Invalid tolerance value, using default (0.1 mm)\n");
            tolerance = 0.1;
        }
    }
    
    if (argc > 4) {
        useAscii = atoi(argv[4]);
    }
    
    printf("Input file:    %s\n", inputPath);
    printf("Output file:   %s\n", outputPath);
    printf("Mesh tolerance: %.3f mm\n", tolerance);
    printf("STL format:    %s\n", useAscii ? "ASCII" : "Binary");
    printf("\nConverting...\n");
    
    // Initialize the Step2Stl library
    Step2Stl_ErrorCode initResult = Step2Stl_Initialize();
    if (initResult != STEP2STL_SUCCESS) {
        printf("Error: Failed to initialize Step2Stl library: %s\n", 
               Step2Stl_GetErrorMessage(initResult));
        return 1;
    }
    
    // Configure conversion options
    Step2Stl_Config config;
    config.meshTolerance = tolerance;
    config.useAsciiFormat = useAscii;
    config.progressCallback = progressCallback;
    config.userData = NULL;
    
    // Perform the conversion
    Step2Stl_ErrorCode result = Step2Stl_Convert(inputPath, outputPath, &config);
    
    // Print result
    printf("\n\nConversion result: %s\n", 
           result == STEP2STL_SUCCESS ? "SUCCESS" : "FAILED");
    
    if (result != STEP2STL_SUCCESS) {
        printf("Error: %s\n", Step2Stl_GetErrorMessage(result));
    } else {
        printf("STEP file successfully converted to STL!\n");
    }
    
    // Cleanup the library
    Step2Stl_Cleanup();
    
    printf("\nPress Enter to exit...");
    getchar();
    
    return result == STEP2STL_SUCCESS ? 0 : 1;
}
