#include "rsa.h"
#include "utils.h"

/*
 * Sieve of Eratosthenes Algorithm
 * https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes
 *
 * arg0: A limit
 * arg1: The size of the generated primes list. Empty argument used as ret val
 *
 * ret:  The prime numbers that are less or equal to the limit
 */
size_t *
sieve_of_eratosthenes(int limit, int *primes_sz)
{
	//declare boolean array the size of limit and set all to true
	bool prime[limit+1];
	for (int i=0; i<=limit; i++){
    prime[i] = true;
	}
	int count = 0;
	//set all non primes within limit to false
	for (size_t i = 2; i <= limit; i++) {
		if (prime[i]) {
			count++;
			for (size_t j = i*i; j <= limit; j+=i) {
				prime[j]=false;
			}
		}
	}
	//allocate memmory for primess array
	size_t *primes = (size_t*)malloc(sizeof(size_t)*count);
	count =0;
	//pass prime values to primes array
	for (size_t i = 2; i <= limit; i++) {
		if (prime[i]){
			primes[count]=i;
			count++;
		}
	}
	//pass return values
	*primes_sz = count;
	return primes;
}


/*
 * Greatest Common Denominator
 *
 * arg0: first number
 * arg1: second number
 *
 * ret: the GCD
 */
int
gcd(int a, int b)
{
	int gcd;
  for(int i=1; i <= a && i <= b; ++i){
    // Checks if i is factor of both integers
    if(a%i==0 && b%i==0)
        gcd = i;
  }
	return gcd;
}


/*
 * Chooses 'e' where
 *     1 < e < fi(n) AND gcd(e, fi(n)) == 1
 *
 * arg0: fi(n)
 *
 * ret: 'e'
 */
size_t
choose_e(size_t fi_n)
{
	size_t e;

	/* TODO */

	return e;
}


/*
 * Calculates the modular inverse
 *
 * arg0: first number
 * arg1: second number
 *
 * ret: modular inverse
 */
size_t
mod_inverse(size_t a, size_t b)
{

a = a%b;
    for (int x=1; x<b; x++)
       if ((a*x) % b == 1)
          return x;
}


/*
 * Generates an RSA key pair and saves
 * each key in a different file
 */
void
rsa_keygen(void)
{
	size_t p;
	size_t q;
	size_t n;
	size_t fi_n;
	size_t e;
	size_t d;
	//1.Generate a pool of primes using the Sieve Of Eratosthenes.
	int *primes_sz = (int*)malloc(sizeof(int*));
	size_t *primes;
	primes = sieve_of_eratosthenes(RSA_SIEVE_LIMIT,primes_sz);
	//2.Pick two random primes from the pool, p and q.
	srand(time(NULL));
	p = primes[rand()%*primes_sz];
	q = primes[rand()%*primes_sz];
	//3.Compute n.
	n = p * q;
	//4.Calculate fi(n).
	fi_n = (p-1)*(q-1);
	//5.Choose a prime e where (e % fi(n) != 0) AND (gcd(e, fi(n)) == 1)
	while(1){
		e = primes[rand() % *primes_sz];
		if (e % fi_n != 0 && gcd(e, fi_n) == 1) {
			break;
		}
	}
	//6.Choose d where d is the modular inverse of (e,fi(n)).
	d = mod_inverse(e,fi_n);
	//7.Public to file
	FILE *pub = fopen("public.key","w");
	fwrite(&n, sizeof(size_t), 1, pub);
	fwrite(&d, sizeof(size_t), 1, pub);
	printf("Public\n");
	printf("n:%ld     d:%ld\n",n,d );
	//8.Private to file
	FILE *priv = fopen("private.key","w");
	fwrite(&n, sizeof(size_t), 1, priv);
	fwrite(&e, sizeof(size_t), 1, priv);
	printf("Private\n");
	printf("n:%ld     e:%ld\n",n,e );

	free(primes);
	fclose(pub);
	fclose(priv);
	return;
}


/*
 * Encrypts an input file and dumps the ciphertext into an output file
 *
 * arg0: path to input file
 * arg1: path to output file
 * arg2: path to key file
 */
void
rsa_encrypt(char *input_file, char *output_file, char *key_file)
{
	FILE *inFile = fopen(input_file,"r");
	FILE *outFile = fopen(output_file,"w");
	FILE *keyFile = fopen(key_file,"r");

	//read key in memmory
	size_t key[2];
	if (!fread(key,sizeof(size_t),2,keyFile)) {
    printf("-------Error passing key in memmory\n");
  }
		printf("Key derived :\n n:%ld    e/d:%ld\n",key[0], key[1]);

	//read input file in memmory
	fseek(inFile, 0, SEEK_END);
  int inSize = ftell(inFile);
  fseek(inFile, 0, SEEK_SET);
	char *messageBuffer = (char*)malloc(sizeof(char*)*inSize);
	if (!fread(messageBuffer,1,inSize,inFile)) {
    printf("-------Error passing message in memmory\n");
  }

	//encrypt input message
	size_t *encryptedBuffer = (size_t*)malloc(sizeof(size_t*)*inSize);
	size_t val;

	for (int i = 0; i < inSize; i++) {
		encryptedBuffer[i] = (size_t)exponentMod((size_t)messageBuffer[i], key[1], key[0]);
	}

	//write encrypted message to output file
	printf("\nFile %s with size %d decrypted and stored in:\n", input_file, inSize);
	printf("File %s with size %d\n", output_file, inSize*8);
	fwrite(encryptedBuffer, sizeof(size_t), inSize, outFile);

	fclose(inFile);
	fclose(outFile);
	fclose(keyFile);
	free(messageBuffer);
	free(encryptedBuffer);
	return;
}

/*
 * Decrypts an input file and dumps the plaintext into an output file
 *
 * arg0: path to input file
 * arg1: path to output file
 * arg2: path to key file
 */
void
rsa_decrypt(char *input_file, char *output_file, char *key_file)
{
	FILE *inFile = fopen(input_file,"r");
	FILE *outFile = fopen(output_file,"w");
	FILE *keyFile = fopen(key_file,"r");

	//read key in memmory
	size_t *key = (size_t*)malloc(sizeof(size_t)*2);
	if (!fread(key,sizeof(size_t),2,keyFile)) {
    printf("-------Error passing key in memmory\n");
  }
	printf("Key derived :\n n:%ld    e/d:%ld\n",key[0], key[1]);

	//read input file in memmory
	fseek(inFile, 0, SEEK_END);
  int inSize = ftell(inFile);
  fseek(inFile, 0, SEEK_SET);
	size_t *messageBuffer = (size_t*)malloc(sizeof(size_t*)*inSize/8);
	if (!fread(messageBuffer,sizeof(size_t),inSize/8,inFile)) {
    printf("-------Error passing message in memmory\n");
  }

	//decrypt input message
	char *decryptedBuffer = (char*)malloc(sizeof(char*)*inSize/8);
	long int ch;
	for (int i = 0; i < inSize/8; i++) {
		decryptedBuffer[i] = (char)exponentMod(messageBuffer[i], key[1], key[0]);
	}

	//write decrypted message to output file
	printf("\nFile %s with size %d decrypted and stored in:\n", input_file, inSize);
	printf("File %s with size %d\n", output_file, inSize/8);
	fwrite(decryptedBuffer, sizeof(char), inSize/8, outFile);

	fclose(inFile);
	fclose(outFile);
	fclose(keyFile);
	free(messageBuffer);
	free(decryptedBuffer);
	return;
}

int exponentMod(size_t A, size_t B, size_t C)
{
    // Base cases
    if (A == 0)
        return 0;
    if (B == 0)
        return 1;

    // If B is even
    size_t y;
    if (B % 2 == 0) {
        y = exponentMod(A, B / 2, C);
        y = (y * y) % C;
    }

    // If B is odd
    else {
        y = A % C;
        y = (y * exponentMod(A, B - 1, C) % C) % C;
    }

    return (int)((y + C) % C);
}
