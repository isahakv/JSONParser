#include "..\include\JsonReader.h"
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



		return false;
	}

	bool JsonReader::ReadToken(Token& token)
	{
		token.start = current;

		return false;
	}

	void JsonReader::SetError(const char* message)
	{
		errorMessage = message;
	}

	void JsonReader::SkipSpaces()
	{

	}

	void JsonReader::RemoveWhiteSpaces(string& str)
	{
		string::iterator itr = str.begin();
		char symbol;
		while (itr != str.end())
		{
			symbol = *itr;
			if (symbol == ' ' || symbol == '\n' || symbol == '\t' || symbol == '\r')
				itr = str.erase(itr);
			else
				++itr;
		}
	}
}
