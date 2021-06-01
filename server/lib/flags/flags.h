#pragma once

#include <server/lib/types/types.h>

#include <cassert>

class TFlags {
public:
    void Store(ui8 flag) {
        if (Stored(flag)) {
            return;
        }

        Mask_ |= ((ui64) 1) << (ui64) flag;
        ++Size_;
    }

    bool Stored(ui8 flag) const {
        assert(((ui64) flag) < MAX);
        return Mask_ & (((ui64) 1) << (ui64) flag);
    }

    void Remove(ui8 flag) {
        if (!Stored(flag)) {
            return;
        }

        Mask_ ^= ((ui64) 1) << (ui64) flag;
        --Size_;
    }

    size_t Size() const {
        return Size_;
    }

private:
    static constexpr ui32 MAX = sizeof(ui64) * 8;

    ui64 Mask_{0};
    ui8 Size_{0};
};
