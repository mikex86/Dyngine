#include <ErrorHandling/ErrorHandling.hpp>
#include <typeinfo>
#include <string>


errorhandling::Exception::Exception(const char *const exceptionTypeName, const char *const throwSiteMethodName,
                                    const char *const throwSiteSourceFile, int throwSiteLineNumber,
                                    const char *const message)
        :
        std::exception(
                (
                        std::string(exceptionTypeName) + "\n\tin " + throwSiteMethodName + "\n\tat " +
                        throwSiteSourceFile + ":" +
                        std::to_string(throwSiteLineNumber) + " " +
                        message
                ).c_str()
        ) {
}

std::string ErrorHandling_str_indent(const char *cstr) {
    std::string str(cstr);
    size_t index = 0;
    do {
        index = str.find('\t', index);
        str.insert(index, "\t");
        index = str.find('\n', index);
    } while (index != std::string::npos);
    return str;
}