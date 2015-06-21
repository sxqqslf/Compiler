int main()
{
	int i=0, j, n=4; 
	while (i < n) 
	{
		j = 0;
		while (j < i)
		{
			write(j);
			j = j + 1;
		}
		i = i + 1;
	}
	return 0;
}
