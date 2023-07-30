#include <iostream>
#include <future>
#include <random>

int find_min(int* arr, int size)
{
	int min = arr[0];
	int min_j = 0;
	for (int j = 0; j < size; ++j)
	{
		if (arr[j] < min)
		{
			min_j = j;
			min = arr[j];
		}
	}
	return min_j;
}

void selectionSort(int* arr, int size, std::promise<int*> sorted)
{
	if (size <= 1) { return; }
	
	for (int i = 0; i < size; ++i)
	{
		//Поиск минимального элемента реализовать в виде асинхронной задачи.
		auto ft = std::async(std::launch::async, find_min, &arr[i], size - i);
		
		std::swap(arr[i + ft.get()], arr[i]);
	}
	sorted.set_value(arr);
}

void print_arr(int* arr, int size)
{
	for (int i = 0; i < size; ++i)
	{
		std::cout << arr[i] << " ";
	}
	std::cout << std::endl;
}

int main()
{
	setlocale(LC_ALL, "RU");
	const int size = 100;
	int arr[size];
	
	int gens = size;
	std::generate(arr, arr + size, [arr, gens]() mutable {
		return --gens;
		});

	std::cout << "До сортировки:\n";
	print_arr(arr, size);

	//Результат должен возвращаться в основной поток через связку std::promise - std::future.
	std::promise<int*> prom;
	std::future<int*> ft = prom.get_future();
	std::thread t1 (selectionSort, arr, size, std::move(prom));
	t1.detach();

	std::cout << "После сортировки:\n";
	print_arr(ft.get(), size);
}