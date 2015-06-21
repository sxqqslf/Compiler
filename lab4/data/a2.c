int main()
{
	int a = 1, b = -1;
	int c = read();
	if (c < a && c > b)
		write(a);
	else 
		write(b);

	return 0;
}

