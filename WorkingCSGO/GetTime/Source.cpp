#include <ctime>
#include <iostream>

using namespace std;
int main()
{
	time_t t; 
	time_t prev;
	time(&t);
	time(&prev);
	while (true)
	{
		time(&t);
		if (t != prev)
		{
			cout << t << endl;
			time(&prev);
		}
		
	}
	system("pause");
}