typedef union u64 {
	unsigned long long ull;
	unsigned long ul[2];
} UINT64;

unsigned long long do_shl64(unsigned long long a, unsigned int b){
	UINT64 a_cast;
	unsigned long tmp;	

	a_cast.ull = a;

	tmp = a_cast.ul[0] >> (32 - b);
	a_cast.ul[0] = a_cast.ul[0] << b; 
	a_cast.ul[1] = (a_cast.ul[1] << b) | tmp;
	
	return (unsigned long long)a_cast.ull;
}

unsigned long long do_shr64(unsigned long long a, unsigned int b){
	UINT64 a_cast;
	unsigned long tmp;
	
	a_cast.ull = a;
	
	tmp = a_cast.ul[1] << (32 - b);
	a_cast.ul[1] = a_cast.ul[1] >> b;
	a_cast.ul[0] = (a_cast.ul[0] >> b) | tmp;

	return (unsigned long long)a_cast.ull;
}


