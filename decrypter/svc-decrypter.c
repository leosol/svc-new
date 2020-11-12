#include <mbedtls/aes.h>
#include <mbedtls/md.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <stdio.h>
#include <stdlib.h>

void print_buff(char* buff);
void just_test();
void encrypt_file(char* key, char* input, char* output,int buff_len);
void str_to_char_array(char *hexstring, char *dest);
void decrypt_file(char* key, char* input, char* output, int buff_len);

unsigned char key_random[32];
unsigned char key_array[32];
int main(int argc, char *argv[]){
	printf("arg1: HEXKEY  arg2: INPUT  arg3: OUTPUT  arg4: BUFFER_LEN arg5: 0 or 1  (ENC or DEC)\n");
	printf("progr 0123456789abcdef0123456789abcdef /tmp/f1 /tmp/f2 10240 0 \n");
	if(argc<6){
		return;
	}
	for(int i=0;i<argc;i++){
		printf("Using argument: %s\n", argv[i]);
	}
	char* key_as_str = argv[1];
	char* file1 = argv[2];
	char* file2 = argv[3];
	int buff_len = atoi(argv[4]);
	int mode  = atoi(argv[5]);
	printf("Key: %s\n", key_as_str);
	printf("INPUT: %s\n", file1);
	printf("OUTPUT: %s\n", file2);
	printf("Buffer: %d\n", buff_len);
	printf("Mode (0 enc, 1 dec): %d\n",mode); 

	if(mode==0){
		encrypt_file(key_as_str, file1, file2, buff_len);
	}
	if(mode==1){
		decrypt_file(key_as_str, file1, file2, buff_len);
	}
}

void print_buff(char* buff){
	for (int i = 0; i < 16; i++) {
		printf("%x", buff[i] & 0xff);
	}
	printf("%s","\n");
}

void str_to_char_array(char *hexstring, char *dest){
	for(int i=0; i<32; i++){
		sscanf(hexstring+(2*i), "%02x", dest+i);
	}
	printf("%s:\n", "Using key from hex string");
	print_buff(dest);
	
}

void decrypt_file(char* key, char* input, char* output, int buff_len){
	FILE *finput;
	FILE *foutput;

	printf("Decripting file %s to %s with key %s \n", input, output, key);
	
	str_to_char_array(key, key_array);

	finput = fopen(input,"rb");
	foutput = fopen(output, "wb");

	fseek(finput, 0L, SEEK_END);
	long fsize = ftell(finput);
	fseek(finput, 0L, SEEK_SET);
	
	mbedtls_aes_context aes;
	mbedtls_aes_init(&aes);
	mbedtls_aes_setkey_dec(&aes, key_array, 256);

	printf("Input size in bytes: %d\n", fsize);
	long read_turns = fsize/buff_len;
	//forcing one turn
	read_turns = 1;
	printf("Reading file %s. Expected read count:%d\n",input,read_turns);

	unsigned char* input_buffer = malloc(buff_len);
	unsigned char* output_buffer = malloc(buff_len);	
	for(long i=0;i<read_turns;i++){
		fread(input_buffer,buff_len, 1,finput);
		int sized_block_count = buff_len/16;
		int current_block = 0;
		printf("Small blocks to decrypt: %d\n", sized_block_count);
		for(;current_block<sized_block_count;current_block++){
			int address_shift = 16*current_block;
			mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_DECRYPT, input_buffer+address_shift, output_buffer+address_shift);
		}
		printf("Blocks at end %d\n", current_block);
		fwrite(output_buffer,buff_len, 1,foutput);
	}
	fclose(finput);
	fclose(foutput);
	
}

void encrypt_file(char* key, char* input, char* output, int buff_len){
	FILE *finput;
	FILE *foutput;

	printf("Decripting file %s to %s with key %s \n", input, output, key);
	
	str_to_char_array(key, key_array);

	finput = fopen(input,"rb");
	foutput = fopen(output, "wb");

	fseek(finput, 0L, SEEK_END);
	long fsize = ftell(finput);
	fseek(finput, 0L, SEEK_SET);
	
	mbedtls_aes_context aes;
	mbedtls_ctr_drbg_context drbg;
	mbedtls_entropy_context entropy;
	mbedtls_aes_init(&aes);
	mbedtls_ctr_drbg_init(&drbg);
	mbedtls_entropy_init(&entropy);
	mbedtls_ctr_drbg_seed(&drbg,mbedtls_entropy_func,&entropy,key_array,32);
	mbedtls_ctr_drbg_random(&drbg,key_random,32);
	mbedtls_aes_setkey_enc(&aes, key_random, 256);

	printf("Input size in bytes: %d\n", fsize);
	long read_turns = fsize/buff_len;
	//forcing one turn
	read_turns = 1;
	printf("Reading file %s. Expected read count:%d\n",input,read_turns);

	unsigned char* input_buffer = malloc(buff_len);
	unsigned char* output_buffer = malloc(buff_len);	
	for(long i=0;i<read_turns;i++){
		fread(input_buffer,buff_len, 1,finput);
		mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, input_buffer, output_buffer);
		fwrite(output_buffer,16, 1,foutput);
	}
	fclose(finput);
	fclose(foutput);
	
}

void just_test(){
	const unsigned char RANDOM_KEY32[] = {0xE2,0x82,0xAC,0xE2,0xE2,0xE2,0xE2,0xE2,
				              0xE2,0xE2,0xE2,0xE2,0xE2,0xE2,0xE2,0xE2,
					      0xE2,0xE2,0xE2,0xE2,0xE2,0xE2,0xE2,0xE2, 
					      0xE2,0xE2,0xE2,0xE2,0xE2,0xE2,0xE3,0xE2}; 


	char plain[16] = "0123456789abcdef";
	char encrypted[16] = "0000000000000000";
	char encrypted_decrypted[16] = "0000000000000000";

	printf("%s","Plain:");
	print_buff(plain);
	printf("%s", "Encrypted (should be empty):");
	print_buff(encrypted);

	mbedtls_aes_context aes1;
	mbedtls_aes_context aes2;
	mbedtls_aes_init(&aes1);
	mbedtls_aes_init(&aes2);
	mbedtls_aes_setkey_enc(&aes1, RANDOM_KEY32, 256);
	mbedtls_aes_setkey_dec(&aes2, RANDOM_KEY32, 256);
	mbedtls_aes_crypt_ecb(&aes1, MBEDTLS_AES_ENCRYPT, plain, encrypted);
	mbedtls_aes_crypt_ecb(&aes2, MBEDTLS_AES_DECRYPT, encrypted, encrypted_decrypted);
	
	printf("%s","Plain:");
	print_buff(plain);
	printf("%s", "Encrypted (should NOT be empty):");
	print_buff(encrypted);
	printf("%s", "Encrypted decrypted (should NOT be empty):");
	print_buff(encrypted_decrypted);
}
