#ifdef __WALK_BASE_NONCOPYABLE_H__
#define __WALK_BASE_NONCOPYABLE_H__

namespace water {

class Noncopyable {
 protected:
    Noncopyable() =default;
    ~Noncopyable() =default;
 private:
    Noncopyable(const&) =delete;
    Noncopyable& operator=(const Noncopyable&) =delete;
};

}   // namespace water

#endif // __WALK_BASE_NONCOPYABLE_H__
