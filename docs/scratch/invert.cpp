#include <iostream>
#include <vector>

int n = 100;

int getLine(int index)
{
	index = index % (2*n);

	if (index >= n)
		return 2*n - 1 - 2*(index - n);
	else
		return 2*index;
}

int getIndex(int line)
{
	if (line % 2 == 0)
		return line / 2;
	else
		return 2*n - 1 - (line/2);
}

int main()
{
	n = 5;

	int store;
	std::vector<int> lines;
	std::vector<bool> done;

	lines.resize(2*n);
	done.resize(2*n);

	for (int t = 0; t <= n; t++) {
		for (int i = 0; i < 2*n; i++) {
			lines[i] = i;
			done[i] = false;
		}

		for (int i = 0; i < 2*n; i++) {
			if (done[i]) continue;

			int j = i;
			store = lines[j];
			while (!done[j]) {
				done[j] = true;
				int newj = getLine(getIndex(j) + t);
				if (done[newj])
					lines[j] = store;
				else
					lines[j] = lines[newj];
				j = newj;
			}
		}

		for (int i = 0; i < 2*n; i++) 
			std::cout << lines[i] << " ";
		std::cout << std::endl;
	}
	return 0;
}
