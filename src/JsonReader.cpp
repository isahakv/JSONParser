#include "JsonReader.h"
#include <sstream>
#include <iostream>

namespace Json
{
	bool JsonReader::Parse(const string& json, JsonObject& root)
	{
		const char* begin = json.c_str();
		const char*  end = begin + json.length();
		return Parse(begin, end, root);
	}

	bool JsonReader::Parse(istream& is, JsonObject& root)
	{
		string json;
		std::getline(is, json, static_cast<char>(EOF));
		return Parse(json, root);
	}

	bool JsonReader::Parse(const char* beginText, const char* endText, JsonObject& root)
	{
		current = begin = beginText;
		end = endText;

		if (*current != '{' && *current != '[')
		{
			SetError("A valid JSON document must be either an array or an object value.");
			return false;
		}

		bool succeed = ReadValue(root);
		return succeed;
	}

	bool JsonReader::ReadValue(JsonObject& object)
	{
		Token token;
		bool succeed = ReadToken(token);
		if (!succeed)
		{
			SetError("Syntax error: value, object or array expected.");
			return false;
		}

		switch (token.type)
		{
		case tokenObjectBegin:
			succeed = ReadObject(object);
			break;
		case tokenArrayBegin:
			succeed = ReadArray(object);
			break;
		case tokenString:
			succeed = DecodeString(token, object);
			break;
		case tokenNumber:
			succeed = DecodeNumber(token, object);
			break;
		case tokenTrue:
			object = true;
			break;
		case tokenFalse:
			object = false;
			break;
		case tokenNull:
			object = JsonObject();
			break;
		case tokenComma:
		case tokenColon:
		case tokenObjectEnd:
		case tokenArrayEnd:
		default:
			// Error.
			succeed = false;
			break;
		}

		return succeed;
	}

	bool JsonReader::ReadToken(Token& token)
	{
		SkipSpaces();
		token.start = current;
		char c = GetNextChar();

		bool succeed = true;
		switch (c)
		{
			break;
		case '{':
			token.type = tokenObjectBegin;
			break;
		case '}':
			token.type = tokenObjectEnd;
			break;
		case '[':
			token.type = tokenArrayBegin;
			break;
		case ']':
			token.type = tokenArrayEnd;
			break;
		case '"':
			token.type = tokenString;
			succeed = ReadString();
			break;
		case '/': // Not Supported Currently.
			token.type = tokenComment;
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case '-':
				token.type = tokenNumber;
				succeed = ReadNumber();
				break;
		case 't':
			token.type = tokenTrue;
			succeed = Match("rue", 3);
			break;
		case 'f':
			token.type = tokenFalse;
			succeed = Match("alse", 4);
			break;
		case 'n':
			token.type = tokenNull;
			succeed = Match("ull", 3);
			break;
		case ',':
			token.type = tokenComma;
			break;
		case ':':
			token.type = tokenColon;
			break;
		case '\0':
			token.type = tokenEndOfStream;
			break;
		default:
			succeed = false;
			break;
		}

		token.end = current;
		return succeed;
	}

	bool JsonReader::ReadObject(JsonObject& object)
	{
		Token tokenName;
		string name;
		while (ReadToken(tokenName))
		{
			// If empty object.
			if (tokenName.type == tokenObjectEnd)
				return true;
			// Clear name.
			name.clear();
			// Check and Decode name.
			if (tokenName.type != tokenString || !DecodeString(tokenName, name))
			{
				// Handle Error.
				return false;
			}
			// Read and Check colon.
			Token colon;
			if (!ReadToken(colon) || colon.type != tokenColon)
			{
				// Handle Error.
				return false;
			}
			// Read and Check value.
			JsonObject value;
			if (!ReadValue(value))
			{
				// Handle Error.
				return false;
			}
			object[name] = std::move(value);
			// Check for comma.
			Token comma;
			if (!ReadToken(comma) || (comma.type != tokenComma && comma.type != tokenObjectEnd))
			{
				// Handle Error.
				return false;
			}
			// If object ended.
			if (comma.type == tokenObjectEnd)
				return true;
		}

		return false;
	}

	bool JsonReader::ReadArray(JsonObject& object)
	{
		return false;
	}

	bool JsonReader::ReadString()
	{
		char c;
		while (current != end)
		{
			c = GetNextChar();
			if (c == '"')
				return true;
		}
		return false;
	}

	bool JsonReader::ReadNumber()
	{
		char c;
		// Integral part.
		do
		{
			if (current == end)
				return true;
			c = GetNextChar();
		} while (c >= '0' && c <= '9');
		// Fractional part.
		if (c == '.')
		{
			do
			{
				if (current == end)
					return true;
				c = GetNextChar();
			} while (c >= '0' && c <= '9');
		}
		// Exponential part.
		if (c == 'e' || c == 'E')
		{
			c = GetNextChar();
			if (c == '+' || c == '-')
			{
				do
				{
					if (current == end)
						return true;
					c = GetNextChar();
				} while (c >= '0' && c <= '9');
			}
			else
				return false;
		}

		return true;
	}

	bool JsonReader::DecodeString(const Token& token, JsonObject& object)
	{
		string decoded;
		if (!DecodeString(token, decoded))
			return false;

		object = decoded;
		return true;
	}

	bool JsonReader::DecodeString(const Token& token, string& decodedText)
	{
		int length = token.end - token.start - 2; // Ship '"'s
		decodedText.reserve(length);
		const char* current = token.start + 1; // Ship '"'
		const char* end = token.end - 1;
		while (current != end)
		{
			// TODO: Handle Escape Sequences.
			char c = *current++;
			decodedText += c;
		}
		return true;
	}

	bool JsonReader::DecodeNumber(const Token& token, JsonObject& object)
	{
		return false;
	}

	void JsonReader::SetError(const char* message)
	{
		errorMessage = message;
	}

	void JsonReader::SkipSpaces()
	{
		char c;
		while (current != end)
		{
			c = *current;
			if (c == ' ' || c == '\n' || c == '\t' || c == '\r')
				++current;
			else
				break;
		}
	}

	char JsonReader::GetNextChar()
	{
		if (current == end)
			return '\0';
		return *current++;
	}
	
	bool JsonReader::Match(const char* pattern, int patternLength)
	{
		if (patternLength > end - current)
			return false;
		int index = 0;
		while (index < patternLength)
		{
			if (current[index] != pattern[index])
				return false;
			index++;
		}
		current += patternLength;
		return true;
	}
}
