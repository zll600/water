#ifndef __WATER_BASE_NONCOPYABLE_H__
#define __WATER_BASE_NONCOPYABLE_H__

namespace water {

class Noncopyable {
 protected:
    Noncopyable() =default;
    ~Noncopyable() =default;
 private:
    Noncopyable(const Noncopyable&) =delete;
    Noncopyable& operator=(const Noncopyable&) =delete;
};

}   // namespace water

#endif // __WATER_BASE_NONCOPYABLE_H__
