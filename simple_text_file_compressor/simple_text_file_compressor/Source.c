#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* String object */
char* string_ctor(const char* str, int len)
{
	char* s = (char*)malloc(sizeof(char) * (len + 1)); // + '\0'
	if (!s)
	{
		printf("Unable to allocate memory for a string...");
		return NULL;
	}

	memcpy(s, str, len);
	s[len] = '\0';

	return s;
}

void string_dtor(char* s)
{
	free(s);
}
/* */

/* Vector of strings */

typedef struct vector_of_strings
{
	int size;
	int capacity;
	char** buff; // Array of pointers to strings
} str_vec;

str_vec* str_vec_ctor(int size);
void str_vec_push(str_vec* v, const char* s);
int str_vec_push_unique(str_vec* v, const char* s);
int str_vec_push_unique_move(str_vec* v, char* s);
void str_vec_resize(str_vec* v, int new_capacity);
void str_vec_dtor(str_vec* v);
void str_vec_print(str_vec* v);
int str_vec_find(str_vec* v, const char* s);

str_vec* str_vec_ctor(int size)
{
	str_vec* v = (str_vec*)malloc(sizeof(str_vec));
	if (!v)
	{
		printf("Unable to allocate memory for a vector of strings...");
		return NULL;
	}
	v->size = 0;
	v->capacity = 0;
	v->buff = NULL;

	str_vec_resize(v, size);

	return v;
}

int str_vec_find(str_vec* v, const char* s)
{
	for (int i = 0; i < v->size; ++i)
	{
		if (strcmp(s, v->buff[i]) == 0)
		{
			return i;
		}
	}
	return -1;
}

void str_vec_push(str_vec* v, const char* s)
{
	if (v->size == v->capacity)
	{
		assert(v->capacity > 0);
		str_vec_resize(v, v->capacity * 2);
	}

	v->buff[v->size] = string_ctor(s, strlen(s));
	v->size += 1;
}

// Returns the index of the pushed element. Note that it could be already inserted!
int str_vec_push_unique(str_vec* v, const char* s)
{
	char* copy_s = string_ctor(s, strlen(s));

	return str_vec_push_unique_move(v, copy_s);
}

int str_vec_push_unique_move(str_vec* v, char* s)
{
	int i = str_vec_find(v, s);
	if (i != -1)
	{
		free(s);
		return i;
	}

	if (v->size == v->capacity)
	{
		assert(v->capacity > 0);
		str_vec_resize(v, v->capacity * 2);
	}

	int new_index = v->size;
	v->buff[v->size] = s;
	v->size += 1;

	return new_index;
}

void str_vec_resize(str_vec* v, int new_capacity)
{
	if (v->capacity >= new_capacity) // If the current size is big enough - then it's OK, do nothing.
	{
		return;
	}

	char** buff = (char**)malloc(sizeof(char*) * new_capacity); // Creates array of 'size' number of pointers to strings
	if (!buff)
	{
		printf("Unable to resize vector of strings...");
		return;
	}

	memcpy(buff, v->buff, v->size * sizeof(char*)); // Copy the current pointers in the buff into the new buffer (with bigger capacity)

	free(v->buff); // Frees only the array which holds the pointers, not the strings pointed by them!
	v->buff = buff;
	v->capacity = new_capacity;
}

void str_vec_dtor(str_vec* v)
{
	int i = 0;
	while (i < v->size)
	{
		string_dtor(v->buff[i]);
		++i;
	}

	free(v->buff);
	free(v);
}

void str_vec_print(str_vec* v)
{
	int i;
	for (i = 0; i < v->size; ++i)
	{
		printf("%s\n", v->buff[i]);
	}
}
/* */

/*
	I am going to make a dictionary(vector of strings) with the unique words from the file.
	Then, the word's index is going to be the unique number representing (compressing) the word.
	It would be much better if the dictionary is a hash map instead of vector of strings.
*/

char* get_word(FILE* in, char* last_read_symbol)
{
	char * word = malloc(100), *wordp = word;
	size_t lenmax = 100, len = lenmax;
	int c;

	if (word == NULL)
	{
		return NULL;
	}

	for (;;)
	{
		c = fgetc(in);
		if (c == EOF || c == '\t' || c == '\n' || c == ' ')
			break;

		if (--len == 0) // the word is longer than 100 symbols
		{
			len = lenmax;
			char * word_bigger = realloc(wordp, lenmax *= 2);

			if (word_bigger == NULL)
			{
				free(wordp);
				return NULL;
			}
			word = word_bigger + (word - wordp);
			wordp = word_bigger;
		}

		*word++ = c;
	}
	*word = '\0';
	*last_read_symbol = c;
	return wordp;
}

void write_str_vec_to_file(FILE* out, str_vec* v)
{
	int i;
	for (i = 0; i < v->size; ++i)
	{
		fprintf(out, "%s\n", v->buff[i]);
	}
}

str_vec* compress(FILE* in, FILE* out)
{
	str_vec* dict = str_vec_ctor(128); // I expect to be at least a few unique words in the file. Just to not resize first few times.
	
	char* last_read_symbol = malloc(sizeof(char));
	char* word = NULL;
	do
	{
		char* word = get_word(in, last_read_symbol);
		if (!word)
		{
			break;
		}

		if (strlen(word) > 0)
		{
			int idx = str_vec_push_unique_move(dict, word);
			fprintf(out, "%d", idx);
		}

		if (*last_read_symbol != EOF)
		{
			fprintf(out, "%c", *last_read_symbol);
		}
	} while (*last_read_symbol != EOF);

	free(last_read_symbol);

	return dict;
}

void compress_file(const char* file, const char* dictionary, const char* compressed)
{
	printf("Compressing the '%s' into '%s' and the dictionary is written to '%s'...\n", file, compressed, dictionary);

	FILE *fp_in;
	fp_in = fopen(file, "r");
	if (!fp_in)
	{
		perror(fp_in);
		return;
	}

	FILE* fp_out;
	fp_out = fopen(compressed, "w");
	if (!fp_out)
	{
		fclose(fp_in);
		perror(fp_out);
		return;
	}

	FILE* fp_out_dict;
	fp_out_dict = fopen(dictionary, "w");
	if (!fp_out_dict)
	{
		fclose(fp_in);
		fclose(fp_out);
		perror(fp_out_dict);
		return;
	}

	str_vec* dict = compress(fp_in, fp_out);

	write_str_vec_to_file(fp_out_dict, dict);

	str_vec_dtor(dict);

	fclose(fp_in);
	fclose(fp_out);
	fclose(fp_out_dict);

	printf("...Done\n");
}

str_vec* read_dict_from_file(FILE* in)
{
	str_vec* dict = str_vec_ctor(128);

	char* last_read_symbol = malloc(sizeof(char));
	char* word = NULL;
	do
	{
		char* word = get_word(in, last_read_symbol);
		if (!word || strlen(word) == 0)
		{
			break;
		}
		
		int current_size = dict->size;
		int idx = str_vec_push_unique_move(dict, word);
		assert(idx == current_size); // The words in the file should be unique and the order of reading is indeed their unique number reprsentation (idx).
		assert(*last_read_symbol == '\n' || *last_read_symbol == EOF);

	} while (*last_read_symbol != EOF);

	free(last_read_symbol);
	return dict;
}

void decompress(FILE* in, FILE* out, const str_vec* dict)
{
	char* last_read_symbol = malloc(sizeof(char));
	char* word = NULL;
	do
	{
		char* word = get_word(in, last_read_symbol);
		if (!word)
		{
			break;
		}
		
		if (strlen(word) > 0)
		{
			int code = atoi(word);
			assert(code >= 0 && code < dict->size);
			fprintf(out, "%s", dict->buff[code]);
		}
		
		if (*last_read_symbol != EOF) // Write the space/tab/new line
		{
			fprintf(out, "%c", *last_read_symbol);
		}
	} while (*last_read_symbol != EOF);

	free(last_read_symbol);
}

void decompress_file(const char* compressed, const char* dictionary, const char* decompressed)
{
	printf("Decompressing the '%s' into '%s' and the dictionary is taken from '%s'...\n", compressed, decompressed, dictionary);

	FILE *fp_in;
	fp_in = fopen(compressed, "r");
	if (!fp_in)
	{
		perror(fp_in);
		return;
	}

	FILE* fp_out;
	fp_out = fopen(decompressed, "w");
	if (!fp_out)
	{
		fclose(fp_in);
		perror(fp_out);
		return;
	}

	FILE* fp_out_dict;
	fp_out_dict = fopen(dictionary, "r");
	if (!fp_out_dict)
	{
		fclose(fp_in);
		fclose(fp_out);
		perror(fp_out_dict);
		return;
	}

	str_vec* dict = read_dict_from_file(fp_out_dict);
	//printf("Read dictionary from file: \n");
	//str_vec_print(dict);
	decompress(fp_in, fp_out, dict);

	str_vec_dtor(dict);

	fclose(fp_in);
	fclose(fp_out);
	fclose(fp_out_dict);

	printf("...Done\n");
}

int main(void)
{
	compress_file("text.txt", "dict.txt", "compressed.txt");

	decompress_file("compressed.txt", "dict.txt", "decompressed.txt");

	return 0;
}