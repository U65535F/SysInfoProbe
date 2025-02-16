#pragma once
#include <Windows.h>
#include <vector>
#include <string>
#include <format>

/*
 * The `Error` class is used to handle error reporting by maintaining a stack trace and additional error information.
 * It allows for adding function calls with associated error codes, formatting the error trace into a human-readable string,
 * and including descriptions and last error codes for better debugging.
 */
class Error {
public:
    // A structure representing each entry in the error stack trace
    struct StackEntry {
        std::string FunctionName; // The name of the function where the error occurred, in ASCII format
        int ErrorCode;            // The error code associated with the function
    };

    // The stack trace of function calls that led to the error
    std::vector<StackEntry> StackTrace;
    // // A description of the error stored in form of unicode so as to include wide characters.
    std::wstring Description; 
    // The main error code representing the error type
    int ErrorCode;
    // The last error code (optional, can be set to 0 if not needed)
    DWORD LastErrorCode;

    /*
     * Static method to create a new error object with the given parameters.
     * It initializes the error with the function name, error code, description, and last error code.
     */
    static Error New(std::string FunctionName, int ErrorCode, std::wstring Description = L"", DWORD LastErrorCode = 0) {
        return Error(FunctionName, Description, ErrorCode, LastErrorCode);
    }

    /*
     * Static method to dynamically allocate an error object with the provided parameters.
     * This method create an error object on the heap.
     */
    static Error* Allocate(std::string& FunctionName, int ErrorCode, std::wstring Description = L"", DWORD LastErrorCode = 0) {
        return new Error(FunctionName, Description, ErrorCode, LastErrorCode);
    }

    /*
     * Adds a new function name and error code to the stack trace.
     * Used to build a detailed stack trace as the error propagates through different functions.
     */
    void AddNewFunctionToStack(std::string FunctionName, int ErrorCode) {
        StackTrace.push_back({ FunctionName, ErrorCode });
    }

    /*
     * Formats the error information, including the stack trace, description, and error codes,
     * into a human-readable wide string (std::wstring).
     * This method uses std::format which was introduced in C++20.
     * This makes the class only compatible for C++20.
     * TODO: remove std::format to make this compatible for older C++ versions.
     */
    std::wstring Format() {
        if (!_bInitialized) return L"";

        std::wstring FormattedString;

        // Loop through the stack trace in reverse order (most recent function call first)
        for (auto Element = StackTrace.rbegin(); Element != StackTrace.rend(); ++Element) {
            // Convert the ASCII function name to a wide string because for some reason std::format doesn't do it
            // And leads to some unknown constexpr errors.
            std::wstring wFunctionName(Element->FunctionName.begin(), Element->FunctionName.end());
            // Build the stack trace in form of "FuncName[ErrorCode] -> ".
            FormattedString += std::format(L"{0}[{1}] -> ", wFunctionName, Element->ErrorCode);
        }

        // Formats the string to add description.
        // It alsoRemove the last " -> " part which is the stack trace separator from the formatted string
        FormattedString = std::format(L"{0}: {1}", FormattedString.substr(0, FormattedString.size() - 4), Description, ErrorCode);

        // If there is a LastErrorCode, append it to the formatted string as `LEC`
        if (LastErrorCode != 0) {
            FormattedString += std::format(L";LEC={0}", LastErrorCode);
        }

        return FormattedString;
    }

private:
    // A flag indicating initialization of the object
    BOOL _bInitialized = FALSE;

    /*
     * Private constructor to create an `Error` object with the provided parameters.
     * This constructor is used by the static methods `New` and `Allocate` to initialize the error.
     */
    Error(std::string FunctionName, std::wstring Description, int ErrorCode, DWORD LastErrorCode)
        : Description(Description), ErrorCode(ErrorCode), LastErrorCode(LastErrorCode) {
        StackTrace.push_back({ FunctionName, ErrorCode });
        _bInitialized = TRUE;
    }
};
