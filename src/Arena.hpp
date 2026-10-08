#ifndef ARENA_HPP
#define ARENA_HPP

#include <cstdint>
#include <cstddef>
#include <vector>
#include <cstdlib>
#include <optional>
#include <limits>
#include <stdexcept>
    
class Arena
{
public:
    Arena(std::size_t size, std::size_t alignment = alignof(std::max_align_t))
    {
        this->alignment = alignment;

        if (this->alignment == 0)
            throw std::bad_alloc{};

        std::size_t rounded_size = this->align(size);
        this->size = rounded_size;

        this->arena = std::aligned_alloc(this->alignment, this->size);

        if (this->arena == nullptr)
            throw std::bad_alloc{};
    }

    ~Arena()
    {
        free_arena();
    }

    void free_arena();
    void *alloc(std::size_t bytes);
    void free_region(std::uintptr_t region);

public:
    enum class EventState : uint8_t
    {
        ALLOC,
        FREE,
    };

    struct Event
    {
        Arena::EventState state;
        std::uintptr_t region;
        std::size_t size;
    };

private:
    inline std::size_t align(std::size_t sz)
    {
        std::size_t remainder = sz % this->alignment;
        return remainder == 0 ? sz : sz + this->alignment - remainder;
    }

    bool available(std::size_t addr, std::size_t length) const;
    std::optional<std::reference_wrapper<Arena::Event>> find_contained_region(std::uintptr_t addr);

private:
    void *arena;
    size_t size;
    size_t alignment;
    std::vector<Arena::Event> state_table;
};

#endif