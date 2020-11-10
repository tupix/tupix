unsigned int str_len(const char* str)
{
	unsigned int len = 0;
	while (*(str++)) {
		len++;
	}
	return len;
}
