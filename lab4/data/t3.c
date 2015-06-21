int sum(int a, int b, int c, int d, int e, int f)
{
	return a+b+c+d+e+f;
}

int main()
{
	int a;
	a = read();
	a = sum(a, 1, 2, 3, 4, 5);
	write(a);
	return 0;
}
