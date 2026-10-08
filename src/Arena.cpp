#include "Arena.hpp"

static constexpr std::optional<std::size_t> safe_add(std::size_t a, std::size_t b)
{
    if (a > std::numeric_limits<std::size_t>::max() - b)
        return std::nullopt;
    return a + b;
}

void Arena::free_arena()
{
    this->size = {};
    this->alignment = {};
    this->state_table.clear();

    if (this->arena)
    {
        free(this->arena);
        this->arena = nullptr;
    }
}

void Arena::free_region(std::uintptr_t region)
{
    std::optional<std::reference_wrapper<Arena::Event>> evt = this->find_contained_region(region);

    if (!evt) return;

    evt->get().state = Arena::EventState::FREE;
    evt->get().size = {};
}

void *Arena::alloc(std::size_t bytes)
{
    if (bytes == 0) throw std::bad_alloc{};

    std::size_t aligned_bytes = this->align(bytes);

    for (std::size_t addr = 0; addr < this->size; addr += this->alignment)
    {
        std::optional<std::size_t> end = safe_add(addr, aligned_bytes);

        if (!end || *end > this->size)
            break;

        if (!this->available(addr, aligned_bytes))
            continue;

        auto region = this->find_contained_region(addr);

        if (region)
        {
            region->get().region = addr;
            region->get().size = aligned_bytes;
            region->get().state = Arena::EventState::ALLOC;

            return static_cast<void *>(static_cast<std::byte *>(this->arena) + addr);
        }

        Event evt;
        evt.size = aligned_bytes;
        evt.state = Arena::EventState::ALLOC;
        evt.region = addr;

        this->state_table.push_back(evt);

        return static_cast<void *>(static_cast<std::byte *>(this->arena) + addr);
    }

    throw std::bad_alloc{};
}

std::optional<std::reference_wrapper<Arena::Event>> Arena::find_contained_region(std::uintptr_t addr)
{
    for (Arena::Event &entry : this->state_table)
    {
        auto entry_end = safe_add(entry.region, entry.size);

        if (!entry_end)
            continue;

        if (entry.region <= addr && addr < *entry_end)
        {
            return entry;
        }
    }

    return std::nullopt;
}

bool Arena::available(std::size_t addr, std::size_t length) const
{
    const auto end = safe_add(addr, length);

    if (!end)
        throw std::overflow_error("Arena allocation range overflow");

    for (const Arena::Event &event : this->state_table)
    {
        if (event.state != Arena::EventState::ALLOC)
            continue;

        const auto event_end = safe_add(event.region, event.size);

        if (!event_end)
            throw std::overflow_error("Arena event range overflow");

        if (addr < *event_end && event.region < *end)
            return false;
    }

    return true;
}
