#include "src/Visualiser.hpp"
#include <random>

// Testing
int main()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1000, 10000);

    int number = dist(gen);

	std::vector<void*> pointers;

	Arena arena(number);
	
	std::uniform_int_distribution<int> bytes(1, 100);
	for(int i = 0; i < number / 64; i++)
	{
		pointers.push_back(arena.alloc(bytes(gen)));
	}

	srand(time(NULL));
	for (auto & p : pointers)
	{
		if (rand() % 10 < 3)
		{
			arena.free_region(p);
		}
	}

	Visualiser v;
	v.init();
	v.process_arena_memory_state(arena);
	v.draw();
	v.deinit();

}
