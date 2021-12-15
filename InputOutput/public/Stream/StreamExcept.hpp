#pragma once

#include <ErrorHandling/ErrorHandling.hpp>

namespace Stream {

    NEW_EXCEPTION_TYPE(StreamSeekException);

    NEW_EXCEPTION_TYPE(StreamWriteException);

    NEW_EXCEPTION_TYPE(StreamReadException);

}