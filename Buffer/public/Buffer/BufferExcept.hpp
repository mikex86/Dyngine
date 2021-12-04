#pragma once

#include <ErrorHandling/ErrorHandling.hpp>

namespace buffer {

    NEW_EXCEPTION_TYPE(BufferSeekException);

    NEW_EXCEPTION_TYPE(BufferWriteException);

    NEW_EXCEPTION_TYPE(BufferReadException);

}