#pragma once

#include <exception>
#include <string>
#include <cstring>

#ifdef _WIN32
#define __Error_Handling_FileSeparator '\\'
#else
#define __Error_Handling_FileSeparator '/'
#endif

// Substring __FILE__ which is absolute at compile time
// See: https://stackoverflow.com/questions/31050113/how-to-extract-the-source-filename-without-path-and-suffix-at-compile-time

static constexpr const char *Error_Handling_file_name_str_end(const char *str) {
    return *str ? Error_Handling_file_name_str_end(str + 1) : str;
}

static constexpr bool Error_Handling_file_name_str_slant_f(const char *str) {
    return *str == __Error_Handling_FileSeparator || *str != 0 && Error_Handling_file_name_str_slant_f(str + 1);
}

static constexpr const char *Error_Handling_file_name_r_slant_f(const char *str) {
    return *str == __Error_Handling_FileSeparator ? (str + 1) : Error_Handling_file_name_r_slant_f(str - 1);
}

#define Error_Handling_FILENAME Error_Handling_file_name_r_slant_f(__FILE__) ? Error_Handling_file_name_r_slant_f(Error_Handling_file_name_str_end(__FILE__)) : __FILE__

#ifdef _MSC_VER
#define Error_Handling_Function __FUNCTION__
#else
#define Error_Handling_Function __PRETTY_FUNCTION__
#endif

std::string ErrorHandling_str_indent(const char *cstr);

/**
 * Throws an exception of the specified type (specify class name as #ExceptionType), with a specific #message.
 * #exceptionType must be a subclass of errorhandling::Exception.
 *
 * The macro throws the exception while incorporating information about the calling context into the exception.
 */
#define RAISE_EXCEPTION(ExceptionType, message) throw ExceptionType(Error_Handling_Function, Error_Handling_FILENAME, __LINE__, std::string(message).c_str())


/**
 * Throws an exception of the specified type (specify class name as #ExceptionType), with a specific #message and with
 * information about another exception that caused this exception to be raised.
 * #exceptionType must be a subclass of errorhandling::Exception.
 *
 * The macro throws the exception while incorporating information about the calling context into the exception,
 * as well as information about the exception's cause.
 */
#define RAISE_EXCEPTION_CAUSED_BY(ExceptionType, message, cause) throw ExceptionType(Error_Handling_Function, Error_Handling_FILENAME, __LINE__, (std::string(message) + "\n\n\tCaused By: " + ErrorHandling_str_indent((cause).what())).c_str())

namespace errorhandling {

    class Exception : public std::exception {

    public:
        explicit Exception(const char *exceptionTypeName, const char *throwSiteMethodName,
                           const char *throwSiteSourceFile, int throwSiteLineNumber,
                           const char *message);
    };

}

/**
 * Declares a new errorhandling::Exception sub-type interface.
 * This should be used in the header file where the exception type is declared.
 * To create a default implementation for this type, use #EXCEPTION_TYPE_DEFAULT_IMPL(ExceptionType) in the cpp file.
 */
#define NEW_EXCEPTION_TYPE(NewExceptionType) class NewExceptionType : public errorhandling::Exception { \
    public: \
        explicit NewExceptionType(const char *throwSiteMethodName, const char *throwSiteSourceFile, int throwSiteLineNumber, const char *message);\
}

/**
 * Declares a default implementation for the specified errorhandling::Exception type.
 * This should be used in a cpp file to implement the interface
 * declared in a header by #NEW_EXCEPTION_TYPE(NewExceptionType).
 */
#define EXCEPTION_TYPE_DEFAULT_IMPL(ExceptionType) ExceptionType::ExceptionType(const char* throwSiteMethodName, const char *throwSiteSourceFile, int throwSiteLineNumber, const char *message) : Exception(#ExceptionType, throwSiteMethodName, throwSiteSourceFile, throwSiteLineNumber, message) {}