#include <iostream>
#include <future>
#include <algorithm>
#include <barrier>
#include <chrono>

using namespace std::chrono_literals;
std::mutex m_cout;
using duration_ms = std::chrono::duration<double, std::milli>;

template <typename It, typename T>
void async_for_each(It begin, It end, void (*func)(T&))
{	
	static int cores = std::thread::hardware_concurrency();
	//static int cores = 16;
	//static std::barrier barrier(cores);
	int size = std::distance(begin, end);
	//определение макисмального размера блока, до которого продолжится рекурсивное деление массива исходя из кол-ва доступных ядер процессора
	static int max_block_size = size / cores;
	if (size <= max_block_size || max_block_size == 0)//max_block_size == 0 на случай малого размера массива контейнера, когда size < cores
	{	
		//для обеспечения честного кол-ва потоков равного кол-ву ядер можно добавить барьер, но это существенно замедляет функцию,
		//т.к. создание новых потоков ресурсозатратный процесс, быстрее выполняется с частичным пересечением id потоков,
		//когда отработавший поток отадется системой на выполнение следующей асинхронной задачи
		//if(max_block_size != 0) 
		//{barrier.arrive_and_wait();}//ожидание создания необходимого кол-ва потоков
		//
		std::for_each(begin, end, func);
		m_cout.lock();
		std::cout << "\nПоток " << std::this_thread::get_id() << "; кол-во обработанных элементов = " << size;
		m_cout.unlock();
	}
	else
	{	
		It mid = begin;
		std::advance(mid, size / 2);
		auto ft1 = std::async(std::launch::async, async_for_each<It, T>, begin, mid, func);//запуск в новом потоке
		
		async_for_each(mid, end, func);//запуск в текущем потоке
	}
}

void multiply(int &val)
{
	val *= 2;	
}

int main()
{
	setlocale(LC_ALL, "RU");
    
	std::vector<int> v1(800, 4);
	std::cout << "Входные данные: ";
	for (const auto& el : v1)
	{
		std::cout << el << " ";
	}
	
	auto start = std::chrono::high_resolution_clock::now();
	async_for_each(v1.begin(), v1.end(), multiply);
	auto finish = std::chrono::high_resolution_clock::now();
	duration_ms delta = std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start);
	std::cout << "\nВремя работы функции = " << delta.count() << " ms";

	std::cout << "\nВыходные данные: ";
	for (const auto& el : v1)
	{
		std::cout << el << " ";
	}
}