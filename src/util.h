#pragma once

uint32_t str_length(const char* string);

bool str_in_str(char* hay, char* needle);

static char* format_text(char* text, ...);
static void assert_(b8 cond, char* file, int line, char* function, char* in_text = NULL, ...);

#define assert(mcond, ...) assert_(mcond, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)


template <int N>
struct s_str_sbuilder
{
	static_assert(N > 0);
	int len = 0;
	int current_pad = 0;
	int max_chars = N;
	char data[N + 1];

	char* cstr()
	{
		assert(len > 0, "");
		return data;
	}

	void pad(int p, b8 ignore_color_code = true)
	{
		current_pad += p;
		p = current_pad;

		int color_code_characters = 0;
		if(ignore_color_code)
		{
			b8 inside_color = false;
			for(int i = 0; i < len; i++)
			{
				char c = data[i];
				char c1 = data[i + 1];
				if(c == '$' && c1 == '$')
				{
					if(inside_color)
					{
						inside_color = false;
						color_code_characters += 2;
						i += 1;
					}
					else
					{
						inside_color = true;
						color_code_characters += 8;
						i += 7;
					}
				}
			}
		}

		int spaces_to_add = p - len + color_code_characters;
		for(int i = 0; i < spaces_to_add; i++)
		{
			assert(len < max_chars, "");
			data[len] = ' ';
			len += 1;
		}
		data[len] = 0;
	}

	char* get_end_ptr()
	{
		return &data[len];
	}

	void append(char* format, ...)
	{
		char* where_to_write = &data[len];
		va_list args;
		va_start(args, format);
		int written = vsnprintf(where_to_write, max_chars + 1 - len, format, args);
		assert(written > 0 && written < max_chars, "");
		va_end(args);
		len += written;
		data[len] = 0;
	}

	void remove_last_char()
	{
		assert(len > 0, "");
		len -= 1;
		data[len] = 0;
	}
};



static void assert_(b8 cond, char* file, int line, char* function, char* in_text, ...)
{
	if(!cond)
	{
		s_str_sbuilder<1024> builder;
		if(in_text)
		{
			char buffer[512] = {};
			va_list args;
			va_start(args, in_text);
			vsnprintf(buffer, 512, in_text, args);
			va_end(args);

			builder.append("%s\n", buffer);
		}
		builder.append("File: %s\nLine: %i\nFunction: %s", file, line, function);

		MessageBox(
			NULL,
			builder.cstr(),
			"Assertion failed",
			MB_OK | MB_ICONERROR
		);

		exit(1);
	}
}


static char* format_text(char* text, ...)
{
	constexpr int max_format_text_buffers = 4;
	constexpr int max_text_buffer_length = 256;

	static char buffers[max_format_text_buffers][max_text_buffer_length] = {};
	static int index = 0;

	char* current_buffer = buffers[index];
	memset(current_buffer, 0, max_text_buffer_length);

	va_list args;
	va_start(args, text);
#ifdef DEBUG
	int written = vsnprintf(current_buffer, max_text_buffer_length, text, args);
	assert(written > 0 && written < max_text_buffer_length, "");
#else // DEBUG
	vsnprintf(current_buffer, max_text_buffer_length, text, args);
#endif // NOT DEBUG
	va_end(args);

	index += 1;
	if (index >= max_format_text_buffers) { index = 0; }

	return current_buffer;
}