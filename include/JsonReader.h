#ifndef JSON_READER_H
#define JSON_READER_H
#include <string>
#include <istream>
#include "JsonObject.h"

using std::string;
using std::istream;

namespace Json
{
	/** Class for Reading/Parsing JSON data from file/string to
	*	'JsonObject' object. */
	class JsonReader
	{
	public:
		/** Parse JSON text from 'std::string' class. */
		bool Parse(const string& json, JsonObject& root);
		/** Parse JSON text from input stream. */
		bool Parse(istream& is, JsonObject& root);
		/** Parse JSON text from given begin to end. */
		bool Parse(const char* beginText, const char* endText, JsonObject& root);

		/** Get Error message. */
		string GetErrorMessage() const;

	private:
		enum class TokenType : uint8_t
		{
			tokenEndOfStream,
			tokenObjectBegin,
			tokenObjectEnd,
			tokenArrayBegin,
			tokenArrayEnd,
			tokenString,
			tokenInteger,
			tokenReal,
			tokenTrue,
			tokenFalse,
			tokenNull,
			tokenComma,
			tokenColon,
			tokenComment, // Not Supported Currently.
			tokenError
		};

		struct Token
		{
			TokenType type;
			const char* start;
			const char* end;
		};

		struct ErrorInfo
		{
			Token token;
			string message;
		};

	private:
		bool ReadToken(Token& token);
		bool ReadValue(JsonObject& object);
		bool ReadValue(Token& token, JsonObject& object);
		bool ReadObject(JsonObject& object);
		bool ReadArray(JsonObject& object);
		bool ReadString();
		bool ReadNumber(TokenType& outType);

		bool DecodeString(const Token& token, JsonObject& object);
		bool DecodeString(const Token& token, string& decodedText);
		bool DecodeNumber(const Token& token, JsonObject& object);

		/** Set Error message. */
		bool SetError(const char* message, const Token& token);

		void GetLocationLineAndColumn(const char* location, int& line, int& column) const;

		void SkipSpaces();
		char GetNextChar();
		bool Match(const char* pattern, int patternLength);

	private:
		const char* begin = 0;
		const char* current = 0;
		const char* end = 0;

		ErrorInfo errorInfo;
	};
}

#endif // !JSON_READER_H
