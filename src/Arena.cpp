#include "Arena.hpp"

void *Arena::alloc(std::size_t bytes)
{
    std::size_t aligned_bytes = this->align(bytes);

    for (std::size_t addr = 0; addr < this->size; addr += this->alignment)
    {
        if (!this->available(addr))
            continue;
        if (this->size < (addr + aligned_bytes))
            continue;

        auto region = this->find_region_in_history(addr);

        if (region)
        {
            region->get().size = aligned_bytes;
            region->get().state = Arena::EventState::ALLOC;
            region->get().region = addr;

            return static_cast<void *>(static_cast<std::byte *>(this->arena) + addr);
        }

        Event evt;
        evt.size = aligned_bytes;
        evt.state = Arena::EventState::ALLOC;
        evt.region = addr;

        this->history.push_back(evt);

        return static_cast<void *>(static_cast<std::byte *>(this->arena) + addr);

    }

    throw std::bad_alloc{};
}

std::optional<std::reference_wrapper<Arena::Event>> Arena::find_region_in_history(std::uintptr_t memory)
{
    for (Arena::Event &entry : this->history)
    {
        if (entry.region <= memory && memory < (entry.region + entry.size))
        {
            return entry;
        }
    }

    return std::nullopt;
}

bool Arena::available(std::uintptr_t memory)
{
    std::optional<std::reference_wrapper<Arena::Event>> event = this->find_region_in_history(memory);

    if (event)
    {
        return event->get().state == Arena::EventState::FREE;
    }

    return true;
}