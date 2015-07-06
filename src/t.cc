#include <iostream.h>

main()
{

	char* a="Hallo Jojo!";
	int* &c= &a;
	char* &b= &c;

	cout<<a<<endl;
	cout<<b<<endl<<endl;

	a="Nochmal hallo!";

	cout<<a<<endl;
	cout<<b<<endl<<endl;
}
