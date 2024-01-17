#include "DataType.h"
#include "glad.h"

unsigned int GetDataTypeElemCount(DataType type)
{
#define CASE(count) case DataType::Float##count:\
                    case DataType::Int##count:\
                    case DataType::Uint##count:\
                    case DataType::Norm##count:\
                    case DataType::UNorm##count:

    switch (type) {
        CASE()
            return 1;
        CASE(2)
            return 2;
        CASE(3)
            return 3;
        CASE(4)
            return 4;
    case DataType::Byte:
    case DataType::UByte:
        return 1;
    }

    return -1;

#undef CASE

}

unsigned int GetDataTypeNativeType(DataType type)
{
#define CASE(type)  case DataType::type:\
                    case DataType::type##2:\
                    case DataType::type##3:\
                    case DataType::type##4:

    switch (type) {
        CASE(Float)
            return GL_FLOAT;
        CASE(Int)
            CASE(Norm)
            return GL_INT;
        CASE(Uint)
            CASE(UNorm)
            return GL_UNSIGNED_INT;
    case DataType::Byte:
        return GL_BYTE;
    case DataType::UByte:
        return GL_UNSIGNED_BYTE;
    }

    return -1;

#undef CASE
}

bool IsDataTypeNormalized(DataType type)
{
    switch (type) {
    case DataType::Norm:
    case DataType::Norm2:
    case DataType::Norm3:
    case DataType::Norm4:
    case DataType::UNorm:
    case DataType::UNorm2:
    case DataType::UNorm3:
    case DataType::UNorm4:
        return true;
    }

    return false;
}

bool IsDataTypeUNorm(DataType type) {
    switch (type) {
    case DataType::UNorm:
    case DataType::UNorm2:
    case DataType::UNorm3:
    case DataType::UNorm4:
        return true;
    }
    return false;
}

bool IsDataTypeSNorm(DataType type) {
    switch (type) {
    case DataType::Norm:
    case DataType::Norm2:
    case DataType::Norm3:
    case DataType::Norm4:
        return true;
    }
    return false;
}

size_t GetDataTypeByteSize(DataType type)
{
    // At the moment, all types are either floats or 32bit integers
    return GetDataTypeElemCount(type) * sizeof(float);
}
