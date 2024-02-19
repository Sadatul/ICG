int factorial(int a)
{
	if (a == 0)
		return 1;
	return a * factorial(a - 1);
}

void main()
{
	int n;
	n = factorial(5);
	println(n);
}