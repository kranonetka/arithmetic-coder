#include <iostream>
#include <string>

std::string dictionaryOf(const std::string& input) //Получаем алфавит исходного текста
{
	std::string dictionary = "-";
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
		if (symbol == dictionary[i])
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
	h[0] = UINT16_MAX;
	l[0] = 0u;
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
		for (;;)
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
			else if (FIRST_QTR <= l[i] && h[i] < THIRD_QTR)
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
	const unsigned long long bits_size = bits.size();
	unsigned short int value = 0u;
	for (unsigned char i = 0u; i < 16u; ++i)
	{
		if (i == bits_size)
			return value;
		if (bits[i] == '1')
			value |= 1u << (15u - i);
	}
	return value;
}

std::string decode(const std::string& input, const std::string& dictionary, const unsigned short int* freq)
 {
	const unsigned long long
		input_length = input.length(),
		dictionary_length = dictionary.length();
	unsigned short int
		*h = new unsigned short int[input_length + 1ull],
		*l = new unsigned short int[input_length + 1ull],
		newBit,
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
	while (1)
	{
		unsigned int
			range = h[i - 1ull] - l[i - 1ull] + 1ull,
			cum = ((value - l[i - 1ull] + 1ull) * del - 1ull) / range,
			index = 1u;
		while (freq[index] <= cum)
			++index;
		l[i] = l[i - 1ull] + (range * freq[index - 1u]) / del;
		h[i] = l[i - 1ull] + (range * freq[index]) / del - 1u;
		if (dictionary[index] == '\0')
			return output;
		output += dictionary[index];
		while (1)
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
			(value <<= 1u) += newBit;
		}
		++i;
	}
	delete[]h;
	delete[]l;
}


int main()
{
	std::string input = "Teststring007";
	std::string dictionary = dictionaryOf(input);
	unsigned short int *freq = getFreq(input, dictionary);
	std::string output = encode(input, dictionary, freq);
	std::cout << "Input: " << input << std::endl << "Encoded: " << output << std::endl << output.size() << " bits" << std::endl;
	std::cout << "Decoded: " << decode(output, dictionary, freq) << std::endl;
	delete[]freq;
	return 0;
}