#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <ctime>

std::string dictionaryOf(const std::string& input) //Получаем алфавит исходного текста
{
	std::string dictionary;
	dictionary.push_back(char(255));
	dictionary.push_back('\0');
	const unsigned long long input_size = input.size();
	for (unsigned long long i = 0ull; i < input_size; ++i)
	{
		unsigned long long j = 0u;
		while (j < dictionary.size())
		{
			if (dictionary[j] == input[i])
				break;
			++j;
		}
		if (j >= dictionary.size())
			dictionary += input[i];
	}
	return dictionary;
}

unsigned short int* getFreq(const std::string& input, const std::string& dictionary) //Получаем массив накопленных частот
{
	const unsigned long long
		dictionary_size = dictionary.size(),
		input_size = input.size();
	unsigned short int *frequency = new unsigned short int[dictionary_size];
	frequency[0] = 0u;
	for (unsigned long long i = 1ull; i < dictionary_size; ++i)
		frequency[i] = 1u;
	for (unsigned long long i = 0ull; i < input_size; ++i)
	{
		unsigned long long j = 0ull;
		while (input[i] != dictionary[j])
			++j;
		while (j < dictionary_size)
			++frequency[j++];
	}
	return frequency;
}

unsigned int getCharIdx(const char symbol, const std::string& dictionary) //Получаем индекс символа в алфавите
{
	const unsigned long long dictionary_size = dictionary.size();
	for (unsigned int i = 0u; i < dictionary_size; ++i)
		if (dictionary[i] == symbol)
			return i;
	return 0u;
}

std::string encode(std::string input, const std::string& dictionary, const unsigned short int* freq)
{
	input.push_back('\0');
	const unsigned long long
		input_length = input.length(),
		dictionary_length = dictionary.length();
	unsigned short int
		*h = new unsigned short int[input_length + 1ull],
		*l = new unsigned short int[input_length + 1ull];
	l[0] = 0u;
	h[0] = UINT16_MAX;
	unsigned int bits_to_follow = 0u;
	const unsigned short int
		del = freq[dictionary_length - 1ull],
		FIRST_QTR = (UINT16_MAX >> 2u) + 1u,
		HALF = FIRST_QTR << 1u,
		THIRD_QTR = HALF + FIRST_QTR;
	unsigned long long i = 0ull;
	std::string output;
	while (i < input_length)
	{
		unsigned int
			range = h[i] - l[i] + 1u,
			index = getCharIdx(input[i++], dictionary);
		l[i] = l[i - 1ull] + (range * freq[index - 1u] / del);
		h[i] = l[i - 1ull] + (range * freq[index] / del) - 1u;
		while (true)
		{
			if (h[i] < HALF)
			{
				output += '0';
				while (bits_to_follow)
				{
					output += '1';
					--bits_to_follow;
				}
			}
			else if (l[i] >= HALF)
			{
				output += '1';
				while (bits_to_follow)
				{
					output += '0';
					--bits_to_follow;
				}
				l[i] -= HALF;
				h[i] -= HALF;
			}
			else if (l[i] >= FIRST_QTR && h[i] < THIRD_QTR)
			{
				++bits_to_follow;
				l[i] -= FIRST_QTR;
				h[i] -= FIRST_QTR;
			}
			else
				break;
			l[i] <<= 1u;
			++(h[i] <<= 1u);
		}
	}
	delete[]h;
	delete[]l;
	return output;
}

unsigned short int bits16from(const std::string& bits)
{
	unsigned short int value = 0u;
	unsigned char i = 0u;
	while (i < 16u)
	{
		if (i == bits.size())
			break;
		if (bits[i] == '1')
			value |= 1u << (15u - i);
		++i;
	}
	if (i == 16)
		return value;
	else
		return value >> (16u - i);
}

std::string decode(const std::string& input, const std::string& dictionary, const unsigned short int* freq)
{
	const unsigned long long
		input_length = input.length(),
		dictionary_length = dictionary.length();
	unsigned short int
		*h = new unsigned short int[input_length + 1ull],
		*l = new unsigned short int[input_length + 1ull],
		value = bits16from(input);
	l[0] = 0u;
	h[0] = UINT16_MAX;
	const unsigned short int
		del = freq[dictionary_length - 1ull],
		FIRST_QTR = (UINT16_MAX >> 2u) + 1u,
		HALF = FIRST_QTR << 1u,
		THIRD_QTR = HALF + FIRST_QTR;
	std::string output;
	bool stopflag = false;
	unsigned long long
		i = 1ull,
		queueIdx = 16ull;
	while (true)
	{
		unsigned int
			range = h[i - 1ull] - l[i - 1ull] + 1ull,
			cum = ((value - l[i - 1ull] + 1ull) * del - 1ull) / range,
			index = 1u;
		while (freq[index] <= cum)
			++index;
		l[i] = l[i - 1ull] + (range * freq[index - 1u]) / del;
		h[i] = l[i - 1ull] + (range * freq[index]) / del - 1u;
		if (!dictionary[index])
			return output;
		output += dictionary[index];
		while (true)
		{
			if (h[i] >= HALF)
				if (l[i] >= HALF)
				{
					value -= HALF;
					l[i] -= HALF;
					h[i] -= HALF;
				}
				else if (l[i] >= FIRST_QTR && h[i] < THIRD_QTR)
				{
					value -= FIRST_QTR;
					l[i] -= FIRST_QTR;
					h[i] -= FIRST_QTR;
				}
				else
					break;
			unsigned short int newBit;
			if (queueIdx < input_length)
				newBit = (input[queueIdx++] == '1') ? 1 : 0;
			else if (stopflag)
				newBit = 0;
			else
			{
				newBit = 1;
				stopflag = true;
			}
			l[i] <<= 1u;
			++(h[i] <<= 1u);
			(value <<= 1u) |= newBit;
		}
		++i;
	}
	delete[]h;
	delete[]l;
}

void genStrings(std::string& input)
{
	std::srand(std::time(0));
	for (unsigned char i = 1u; i < 6u; ++i)
	{
		std::string newString;
		for (unsigned int j = 0u; j < (1u << i); ++j)
		{
			newString += (std::rand() % 60) + 32;
		}
		input += '\n' + newString;
	}
}

int main()
{
	std::ifstream input_file("input", std::ifstream::binary);
	input_file.seekg(0, input_file.end);
	const long long input_file_size = input_file.tellg();
	input_file.seekg(0, input_file.beg);
	std::string input_string;
	for (long long i = 0ll; i < input_file_size; ++i)
	{
		char ch;
		input_file.read(&ch, 1);
		input_string += ch;
	}
	input_file.close();
	//input_string = "aaab";
	genStrings(input_string);
	std::string dictionary = dictionaryOf(input_string);
	unsigned short int *freq = getFreq(input_string, dictionary);
	std::string encoded = encode(input_string, dictionary, freq);
	std::cout << "Input: " << input_string << std::endl << "Encoded: " << encoded << std::endl << encoded.size() << " bits" << std::endl;
	std::string decoded = decode(encoded, dictionary, freq);
	std::cout << "Decoded: " << decoded << std::endl;
	if (input_string == decoded)
		std::cout << "====\nInput and decoded are similar\n====\nInput:   " << (input_string.size() << 3) << " bits\nEncoded: " << encoded.size() << " bits\n";
	delete[]freq;
	return 0;
}