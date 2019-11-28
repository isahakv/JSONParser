#include "JsonReader.h"
#include <sstream>
#include <iostream>

namespace Json
{
	bool JsonReader::Parse(const string& json, JsonObject& root)
	{
		const char* begin = json.c_str();
		const char* end = begin + json.length();
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

		SkipSpaces();
		if (*current != '{' && *current != '[')
		{
			Token token;
			token.type = TokenType::tokenError;
			token.start = begin;
			token.end = end;
			return SetError("A valid JSON document must be either an array or an object value.", token);
		}

		bool succeed = ReadValue(root);
		return succeed;
	}

	string JsonReader::GetErrorMessage() const
	{
		if (errorInfo.message.empty())
			return "";

		int line, column;
		GetLocationLineAndColumn(errorInfo.token.start, line, column);
		string message = "Error in Line " + std::to_string(line) + ':'
			+ std::to_string(column) + ": " + errorInfo.message + '\n';

		return message;
	}

	bool JsonReader::ReadValue(JsonObject& object)
	{
		Token token;
		if (!ReadToken(token))
			return SetError("Syntax error: value, object or array expected.", token);

		return ReadValue(token, object);
	}

	bool JsonReader::ReadValue(Token& token, JsonObject& object)
	{
		bool succeed = true;
		switch (token.type)
		{
		case TokenType::tokenObjectBegin:
			succeed = ReadObject(object);
			break;
		case TokenType::tokenArrayBegin:
			succeed = ReadArray(object);
			break;
		case TokenType::tokenString:
			succeed = DecodeString(token, object);
			break;
		case TokenType::tokenInteger:
		case TokenType::tokenReal:
			succeed = DecodeNumber(token, object);
			break;
		case TokenType::tokenTrue:
			object = true;
			break;
		case TokenType::tokenFalse:
			object = false;
			break;
		case TokenType::tokenNull:
			object = JsonObject();
			break;
			// Unexpected Tokens.
		case TokenType::tokenComma:
		case TokenType::tokenColon:
		case TokenType::tokenObjectEnd:
		case TokenType::tokenArrayEnd:
		case TokenType::tokenError:
		default:
			return SetError("Syntax error: Unexpected token.", token);
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
		case '{':
			token.type = TokenType::tokenObjectBegin;
			break;
		case '}':
			token.type = TokenType::tokenObjectEnd;
			break;
		case '[':
			token.type = TokenType::tokenArrayBegin;
			break;
		case ']':
			token.type = TokenType::tokenArrayEnd;
			break;
		case '"':
			token.type = TokenType::tokenString;
			succeed = ReadString();
			break;
		case '/': // Not Supported Currently.
			token.type = TokenType::tokenComment;
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
			succeed = ReadNumber(token.type);
			break;
		case 't':
			token.type = TokenType::tokenTrue;
			succeed = Match("rue", 3);
			break;
		case 'f':
			token.type = TokenType::tokenFalse;
			succeed = Match("alse", 4);
			break;
		case 'n':
			token.type = TokenType::tokenNull;
			succeed = Match("ull", 3);
			break;
		case ',':
			token.type = TokenType::tokenComma;
			break;
		case ':':
			token.type = TokenType::tokenColon;
			break;
		case '\0':
			token.type = TokenType::tokenEndOfStream;
			break;
		default: // If unknown/unspecified symbol.
			succeed = false;
			break;
		}
		// If not succeed, then token type is error.
		if (!succeed)
			token.type = TokenType::tokenError;

		token.end = current;
		return succeed;
	}

	bool JsonReader::ReadObject(JsonObject& object)
	{
		Token nameToken, colon, comma;
		string name;
		while (ReadToken(nameToken))
		{
			// If empty object.
			if (nameToken.type == TokenType::tokenObjectEnd)
				return true;
			// Clear name.
			name.clear();
			// Check and Decode name.
			if (nameToken.type != TokenType::tokenString || !DecodeString(nameToken, name))
			{
				if (nameToken.type != TokenType::tokenString)
					return SetError("Expected object member name.", nameToken);
				else // Error already set.
					return false;
			}
			// Read and Check colon.
			if (!ReadToken(colon) || colon.type != TokenType::tokenColon)
				return SetError("Missing ':' after object member name.", colon);

			// Read and Check value.
			JsonObject value;
			if (!ReadValue(value))
				return false; // Error already set.

			object[name] = std::move(value);
			// Check for comma.
			if (!ReadToken(comma) || (comma.type != TokenType::tokenComma && comma.type != TokenType::tokenObjectEnd))
				return SetError("Missing ',' or '}' after object member value.", colon);

			// If object ended.
			if (comma.type == TokenType::tokenObjectEnd)
				return true;
		}

		return false;
	}

	bool JsonReader::ReadArray(JsonObject& object)
	{
		Token valueToken, comma;
		while (true)
		{
			if (!ReadToken(valueToken))
				return false; // Error already set.

			// If empty array.
			if (valueToken.type == TokenType::tokenArrayEnd)
				return true;

			// Read and Check value.
			JsonObject value;
			if (!ReadValue(valueToken, value))
				return false; // Error already set.

			object.Append(std::move(value));
			// Check for comma.
			if (!ReadToken(comma) || (comma.type != TokenType::tokenComma && comma.type != TokenType::tokenArrayEnd))
				return SetError("Missing ',' or ']' after array value.", comma);

			// If array ended.
			if (comma.type == TokenType::tokenArrayEnd)
				return true;
		}
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

	bool JsonReader::ReadNumber(TokenType& outType)
	{
		char c = *current;
		outType = TokenType::tokenInteger;
		// Integral part.
		while (c >= '0' && c <= '9')
		{
			if (current == end)
				return false;
			++current;
			c = *current;
		}
		// Fractional part.
		if (c == '.')
		{
			++current;
			c = *current;
			outType = TokenType::tokenReal;
			while (c >= '0' && c <= '9')
			{
				if (current == end)
					return false;
				++current;
				c = *current;
			}
		}
		// Exponential part.
		if (c == 'e' || c == 'E')
		{
			++current;
			c = *current;
			if (c == '+' || c == '-')
			{
				++current;
				c = *current;
				outType = TokenType::tokenReal;
				while (c >= '0' && c <= '9')
				{
					if (current == end)
						return false;
					++current;
					c = *current;
				}
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
		if (*(token.end - 1) != '"')
			return SetError("Unexpected end of string.", token);

		int length = token.end - token.start - 2; // Skip '"'s
		decodedText.reserve(length);
		const char* current = token.start + 1; // Skip '"'
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
		string buffer(token.start, token.end);
		std::istringstream is(buffer);
		if (token.type == TokenType::tokenInteger)
		{
			bool isNegative = is.peek() == '-';
			int value;
			if (is >> value)
			{
				object = isNegative ? value : static_cast<unsigned int>(value);
				return true;
			}
		}
		else if (token.type == TokenType::tokenReal)
		{
			float value;
			if (is >> value)
			{
				object = value;
				return true;
			}
		}

		return false;
	}

	bool JsonReader::SetError(const char* message, const Token& token)
	{
		errorInfo.token = token;
		errorInfo.message = message;
		return false;
	}

	void JsonReader::GetLocationLineAndColumn(const char* location, int& line, int& column) const
	{
		const char* current = begin;
		const char* lastLineStart = current;
		line = 0;
		while (current < location && current != end)
		{
			char c = *current++;
			if (c == '\n')
			{
				lastLineStart = current;
				line++;
			}
		}

		column = current - lastLineStart + 1;
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