int main()
{
	int i = 5;
	while (i > 0) {
		if (i != 2 && i != 4) 
			write(i);
		i = i-1;
	}
	return 0;
}
