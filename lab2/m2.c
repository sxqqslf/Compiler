int func(int a, int c[10]) {
	float b = 1.0;
	return b;
}

struct pos {
	int x;
};

int main() {
	int d[10];
	struct pos p;
	float e = func(1, p);
}
