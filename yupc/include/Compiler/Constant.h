#ifndef YUPC_COMPILER_CONSTANT_H_
#define YUPC_COMPILER_CONSTANT_H_

#include <string>
#include <cstdint>

namespace yupc
{
    void IntegerCodegen(int64_t value);

    void FloatCodegen(float value);

    void BoolCodegen(bool value);

    void CharCodegen(std::string text);

    void StringCodegen(std::string text);

    void NullCodegen();
} // namespace yupc

#endif
